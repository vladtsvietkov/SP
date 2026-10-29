#include "board.h"
#include "bringup.h"
#include "sensors.h"
#include "dew.h"
#include "sd.h"
#include "gpio_i2c.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
extern UART_HandleTypeDef huart2;
static int force_nack;
static bmp180 baro;
static int32_t air_t,humidity,bmp_t,pressure;
static float surface;
static uint32_t dht_at,ntc_at,bmp_at,last_dht,last_ntc,last_bmp;
static unsigned dht_errors,ntc_errors,bmp_errors;
static int dht_valid,ntc_valid,bmp_valid;
static uint8_t block[512];
static dw_error rd(void*c,uint8_t a,uint8_t reg,uint8_t*b,size_t n){
 (void)c;return gpio_i2c_read(force_nack?0x76:a,reg,b,n);
}
static dw_error wr(void*c,uint8_t a,uint8_t reg,const uint8_t*b,size_t n){
 (void)c;return gpio_i2c_write(force_nack?0x76:a,reg,b,n);
}
static void delay(void*c,uint32_t t){(void)c;HAL_Delay(t);}
static int wait_level(int level,uint16_t limit){
 uint16_t start=board_us();
 while((int)HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)!=level)
  if((uint16_t)(board_us()-start)>limit)return 0;
 return 1;
}
static dw_error dht_read(void){
 uint8_t frame[5]={0};GPIO_InitTypeDef g={0};
 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
 g.Pin=GPIO_PIN_6;g.Mode=GPIO_MODE_OUTPUT_OD;g.Pull=GPIO_PULLUP;HAL_GPIO_Init(GPIOA,&g);
 board_delay_us(1100);HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
 g.Mode=GPIO_MODE_INPUT;HAL_GPIO_Init(GPIOA,&g);
 if(!wait_level(0,120)||!wait_level(1,120)||!wait_level(0,120))return DW_TIMEOUT;
 for(int i=0;i<40;i++){
  if(!wait_level(1,100))return DW_TIMEOUT;
  uint16_t t=board_us();if(!wait_level(0,120))return DW_TIMEOUT;
  frame[i/8]=(uint8_t)((frame[i/8]<<1)|((uint16_t)(board_us()-t)>50));
 }
 return dht_decode(frame,&air_t,&humidity);
}
static uint8_t sdbyte(void*c,uint8_t b){(void)c;uint8_t r=255;
 if(HAL_SPI_TransmitReceive(&hspi1,&b,&r,1,20)!=HAL_OK) return 255;
 return r;
}
static void sdcs(void*c,int high){(void)c;HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,high?GPIO_PIN_SET:GPIO_PIN_RESET);}
static uint32_t ms(void*c){(void)c;return HAL_GetTick();}
static void storage_probe(void){
 dw_sd sd={0,sdbyte,sdcs,ms,0};
 int init=sd_init(&sd),before=0,write=0,read=0;
 if(init){
  read=sd_read(&sd,2048,block);before=read&&!memcmp(block,"DewWindow-storage-v1",20);
  if(!before){memset(block,0xa5,sizeof(block));memcpy(block,"DewWindow-storage-v1",20);
   write=sd_write(&sd,2048,block);memset(block,0,sizeof(block));
   read=sd_read(&sd,2048,block)&&!memcmp(block,"DewWindow-storage-v1",20);
  }else write=1;
 }
 printf("SELF dev=SD init=%d prior=%d write=%d readback=%d hc=%d\r\n",init,before,write,read,sd.high_capacity);
}
void sensors_start(void){
 gpio_i2c_init();
 printf("SELF bus=I2C backend=HAL_GPIO_direction stretch_timeout_us=100\r\n");
 dw_bus bus={0,rd,wr,delay};dw_error e=bmp_init(&baro,bus);
 printf("SELF dev=BMP180 id_expected=0x55 status=%d\r\n",e);
 uint8_t bad;dw_error anomaly=rd(0,0x76,0xd0,&bad,1);
 printf("ANOMALY addr=0x76 result=%d expected=NACK\r\n",anomaly);
 printf("SELF dev=DHT22 id=none checksum_pending=1 warmup_ms=2500\r\n");
 printf("SELF dev=NTC id=none ADC=12bit beta_K=3950 averaging=8\r\n");
 storage_probe();
 last_dht=HAL_GetTick();last_ntc=last_dht-250;last_bmp=last_dht-1000;
}
void sensors_poll(void){
 /* Bring-up diagnostics only. Product CLI belongs to Lab3. */
 static char line[16]; static unsigned used; static int overflow;
 uint8_t byte;
 if(HAL_UART_Receive(&huart2,&byte,1,0)==HAL_OK) {
  if(byte=='\n'||byte=='\r') {
   if(used||overflow) {
    line[used]=0;
    if(overflow) printf("DIAG error=line_overflow\r\n");
    else if(!strcmp(line,"nack")) { force_nack=1; printf("DIAG command=nack applied=1\r\n"); }
    else if(!strcmp(line,"recover")) { force_nack=0;baro.ready=0;printf("DIAG command=recover applied=1\r\n"); }
    else if(!strcmp(line,"probe")) {baro.ready=0;printf("DIAG command=probe applied=1\r\n");}
    else printf("DIAG error=unknown\r\n");
   }
   used=0;overflow=0;
  } else if(used<sizeof(line)-1) line[used++]=(char)byte;
  else overflow=1;
 }
 uint32_t now=HAL_GetTick();
 if((uint32_t)(now-last_ntc)>=250){
  last_ntc=now;uint32_t sum=0;dw_error e=DW_OK;
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
  for(int i=0;i<8;i++){
   if(HAL_ADC_Start(&hadc1)!=HAL_OK||HAL_ADC_PollForConversion(&hadc1,5)!=HAL_OK){e=DW_TIMEOUT;break;}
   sum+=HAL_ADC_GetValue(&hadc1);HAL_ADC_Stop(&hadc1);
  }
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
  uint16_t raw=sum/8;if(e==DW_OK)e=ntc_decode(raw,&surface);
  ntc_valid=e==DW_OK;if(ntc_valid)ntc_at=HAL_GetTick();else ntc_errors++;
  printf("NTC t_ms=%lu raw=%u surface_cC=%ld valid=%d err=%d count=%u\r\n",(unsigned long)now,raw,(long)lroundf(surface*100),ntc_valid,e,ntc_errors);
 }
 if((uint32_t)(now-last_bmp)>=1000){
  last_bmp=now;dw_error e=bmp_read(&baro,&bmp_t,&pressure);bmp_valid=e==DW_OK;
  if(bmp_valid)bmp_at=HAL_GetTick();else bmp_errors++;
  printf("BMP t_ms=%lu T_dC=%ld p_Pa=%ld valid=%d err=%d count=%u\r\n",(unsigned long)now,(long)bmp_t,(long)pressure,bmp_valid,e,bmp_errors);
 }
 if((uint32_t)(now-last_dht)>=2500){
  last_dht=now;dw_error e=dht_read();dht_valid=e==DW_OK;
  if(dht_valid)dht_at=HAL_GetTick();else dht_errors++;
  printf("DHT t_ms=%lu T_dC=%ld RH_dPct=%ld valid=%d err=%d count=%u\r\n",(unsigned long)now,(long)air_t,(long)humidity,dht_valid,e,dht_errors);
  static int first_summary=1;
  if(first_summary) {
   printf("SELF phase=sampled dht_valid=%d ntc_valid=%d bmp_valid=%d\r\n",dht_valid,ntc_valid,bmp_valid);
   first_summary=0;
  }
  now=HAL_GetTick();uint8_t v=dew_fresh(now,dht_at,6000,dht_valid)|
   (dew_fresh(now,ntc_at,1000,ntc_valid)<<1)|(dew_fresh(now,bmp_at,3000,bmp_valid)<<2);
  dew_result r;int ok=dew_compute(air_t/10.0f,humidity/10.0f,surface,pressure,v,&r);
  if(ok)printf("DEW t_ms=%lu dew_cC=%ld margin_cC=%ld w_cgkg=%ld validity=%u\r\n",(unsigned long)now,(long)lroundf(r.dew_C*100),(long)lroundf(r.margin_C*100),(long)lroundf(r.mixing_g_kg*100),v);
  else printf("DEGRADED t_ms=%lu validity=%u formula_valid=0\r\n",(unsigned long)now,v);
 }
}


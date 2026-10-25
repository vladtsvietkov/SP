#include "sensors.h"
#include "dew.h"
#include "sd.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static const int32_t golden[11]={408,-72,-14383,32741,32757,23153,6190,4,-32768,-8711,2868};
static unsigned checks;
#define CHECK(v) do{assert(v);checks++;}while(0)
typedef struct{int index,fail;dw_error error;uint8_t id;int bad_cal;int writes;} fake_bmp;
static dw_error read_bus(void*ctx,uint8_t a,uint8_t reg,uint8_t*b,size_t n){
 fake_bmp*f=ctx;CHECK(a==0x77);if(++f->index==f->fail)return f->error;
 if(reg==0xd0){CHECK(n==1);*b=f->id;}
 else if(reg==0xaa){CHECK(n==22);for(int i=0;i<11;i++){uint16_t v=f->bad_cal?0:(uint16_t)golden[i];b[2*i]=v>>8;b[2*i+1]=v;}}
 else {CHECK(reg==0xf6);if(n==2){b[0]=27898>>8;b[1]=27898&255;}else{CHECK(n==3);b[0]=23843>>8;b[1]=23843&255;b[2]=0;}}
 return DW_OK;
}
static dw_error write_bus(void*ctx,uint8_t a,uint8_t reg,const uint8_t*b,size_t n){
 fake_bmp*f=ctx;CHECK(a==0x77&&reg==0xf4&&n==1);CHECK(*b==(f->writes++%2?0x34:0x2e));
 if(++f->index==f->fail) return f->error;
 return DW_OK;
}
static void delay_bus(void*c,uint32_t t){(void)c;CHECK(t==5);}
static void test_sensors(void){
 int32_t t,h,p;float n;
 uint8_t d[]={2,88,0,240,74};CHECK(dht_decode(d,&t,&h)==DW_OK&&t==240&&h==600);
 d[2]=128;d[3]=55;d[4]=(uint8_t)(d[0]+d[1]+d[2]+d[3]);CHECK(dht_decode(d,&t,&h)==DW_OK&&t==-55);
 d[4]++;CHECK(dht_decode(d,&t,&h)==DW_CHECKSUM);
 uint8_t invalid[]={255,255,0,0,254};CHECK(dht_decode(invalid,&t,&h)==DW_RANGE);
 CHECK(ntc_decode(2048,&n)==DW_OK&&fabsf(n-25)<0.02);
 CHECK(ntc_decode(0,&n)==DW_RANGE);CHECK(ntc_decode(4095,&n)==DW_RANGE);
 CHECK(ntc_decode(1,&n)==DW_RANGE);CHECK(ntc_decode(4094,&n)==DW_RANGE);
 CHECK(ntc_decode(1000,&n)==DW_OK&&n>25);
 CHECK(bmp_compensate(golden,27898,23843,&t,&p)==DW_OK&&t==150&&p==69964);
 int32_t c[11];memcpy(c,golden,sizeof(c));c[4]=0;c[10]=0;CHECK(bmp_compensate(c,27898,23843,&t,&p)==DW_RANGE);
 memcpy(c,golden,sizeof(c));c[3]=0;CHECK(bmp_compensate(c,27898,23843,&t,&p)==DW_RANGE);
 CHECK(bmp_compensate(golden,27898,0,&t,&p)==DW_RANGE);
 CHECK(bmp_compensate(golden,27898,60000,&t,&p)==DW_RANGE);
 for(int failure=0;failure<=6;failure++){
  fake_bmp f={0,failure,DW_NACK,0x55,0,0};dw_bus bus={&f,read_bus,write_bus,delay_bus};bmp180 dev={0};
  dw_error e=bmp_init(&dev,bus);
  if(failure==1||failure==2){CHECK(e==DW_NACK);continue;}CHECK(e==DW_OK);
  e=bmp_read(&dev,&t,&p);
  CHECK(e==(failure?DW_NACK:DW_OK));
  if(!failure)CHECK(p==69964&&t==150);
  else {CHECK(!dev.ready);f.fail=0;f.writes=0;CHECK(bmp_read(&dev,&t,&p)==DW_OK);}
 }
 fake_bmp f={0,1,DW_TIMEOUT,0x55,0,0};dw_bus bus={&f,read_bus,write_bus,delay_bus};bmp180 dev={0};
 CHECK(bmp_init(&dev,bus)==DW_TIMEOUT);f.fail=0;f.id=0;CHECK(bmp_init(&dev,bus)==DW_ID);
 f.id=0x55;f.bad_cal=1;CHECK(bmp_init(&dev,bus)==DW_RANGE);
}
static void test_dew(void){
 dew_result r;CHECK(dew_compute(24,60,24,101325,7,&r));
 CHECK(fabsf(r.dew_C-15.75f)<0.05&&fabsf(r.margin_C-8.25f)<0.05);
 float w=r.mixing_g_kg;CHECK(dew_compute(24,60,24,90000,7,&r)&&r.mixing_g_kg>w);
 CHECK(dew_compute(0,100,0,30000,7,&r)&&fabsf(r.dew_C)<0.001);
 CHECK(dew_compute(50,1,125,110000,7,&r));
 CHECK(!dew_compute(24,60,24,101325,3,&r));CHECK(!dew_compute(NAN,60,24,101325,7,&r));
 CHECK(!dew_compute(-1,60,24,101325,7,&r));CHECK(!dew_compute(51,60,24,101325,7,&r));
 CHECK(!dew_compute(24,0,24,101325,7,&r));CHECK(!dew_compute(24,101,24,101325,7,&r));
 CHECK(!dew_compute(24,60,-41,101325,7,&r));CHECK(!dew_compute(24,60,126,101325,7,&r));
 CHECK(!dew_compute(24,60,24,0,7,&r));CHECK(!dew_compute(24,60,24,110001,7,&r));
 CHECK(dew_fresh(5,0xfffffff0u,21,1));CHECK(!dew_fresh(6,0xfffffff0u,21,1));CHECK(!dew_fresh(1,1,100,0));
}
/* A byte-level card fake verifies CS-delimited command frames and read/write tokens. */
typedef struct{
 uint32_t now,arg;uint8_t frame[6],reply[520],written[512];unsigned pos,head,tail,commands[512],n,write_pos;
 int selected,writing,accepted,bad_echo,never_ready,fail_cmd;
} card;
static void queue(card*c,uint8_t v){c->reply[c->tail++]=v;}
static uint8_t card_byte(void*ctx,uint8_t v){
 card*c=ctx;c->now++;
 if(c->head<c->tail)return c->reply[c->head++];
 c->head=c->tail=0;
 if(!c->selected)return 255;
 if(c->writing){
  if(c->writing==1){if(v==255)return 255;CHECK(v==254);c->writing=2;c->write_pos=0;return 255;}
  if(c->write_pos<512){c->written[c->write_pos++]=v;return 255;}
  if(c->write_pos++==513){c->writing=0;c->accepted=1;queue(c,5);}return 255;
 }
 if(!c->pos&&((v&0xc0)!=0x40))return 255;
 c->frame[c->pos++]=v;
 if(c->pos==6){
  c->pos=0;unsigned cmd=c->frame[0]&63;c->commands[c->n++]=cmd;
  c->arg=((uint32_t)c->frame[1]<<24)|((uint32_t)c->frame[2]<<16)|(c->frame[3]<<8)|c->frame[4];
  if((int)cmd==c->fail_cmd){queue(c,4);return 255;}
  queue(c,(cmd==0||cmd==8||cmd==55||c->never_ready)?1:0);
  if(cmd==0)CHECK(c->frame[5]==0x95);
  if(cmd==8){CHECK(c->frame[5]==0x87);queue(c,0);queue(c,0);queue(c,1);queue(c,c->bad_echo?0:0xaa);}
  if(cmd==58){queue(c,0x40);queue(c,0);queue(c,0);queue(c,0);}
  if(cmd==17){queue(c,254);for(int i=0;i<512;i++)queue(c,(uint8_t)i);queue(c,255);queue(c,255);}
  if(cmd==24)c->writing=1;
 }return 255;
}
static void card_cs(void*ctx,int high){card*c=ctx;c->selected=!high;if(high)c->pos=0;}
static uint32_t card_ms(void*ctx){return ((card*)ctx)->now++;}
static void test_sd(void){
 card c={.fail_cmd=-1};dw_sd d={&c,card_byte,card_cs,card_ms,0};CHECK(sd_init(&d)&&d.high_capacity);
 CHECK(c.n==5&&c.commands[0]==0&&c.commands[1]==8&&c.commands[2]==55&&c.commands[3]==41&&c.commands[4]==58);
 uint8_t b[512];CHECK(sd_read(&d,2048,b));CHECK(c.arg==2048);for(int i=0;i<512;i++)CHECK(b[i]==(uint8_t)i);
 CHECK(sd_write(&d,2049,b)&&c.accepted);CHECK(!memcmp(b,c.written,512));
 for(int cmd=0;cmd<=58;cmd++){if(cmd!=0&&cmd!=8&&cmd!=41&&cmd!=58)continue;
  memset(&c,0,sizeof(c));c.fail_cmd=cmd;CHECK(!sd_init(&d));}
 memset(&c,0,sizeof(c));c.fail_cmd=-1;c.bad_echo=1;CHECK(!sd_init(&d));
 memset(&c,0,sizeof(c));c.fail_cmd=-1;c.never_ready=1;CHECK(!sd_init(&d));
 memset(&c,0,sizeof(c));c.fail_cmd=17;CHECK(!sd_read(&d,1,b));
 c.fail_cmd=24;CHECK(!sd_write(&d,1,b));
}
int main(void){test_sensors();test_dew();test_sd();printf("DewWindow host: %u assertions passed\n",checks);return 0;}


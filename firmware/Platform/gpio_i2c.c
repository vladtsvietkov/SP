#include "gpio_i2c.h"
#include "board.h"
static void pause_bus(void){board_delay_us(5);}
static void release_pin(uint16_t pin,int release){
 GPIO_InitTypeDef g={0};g.Pin=pin;g.Pull=GPIO_PULLUP;g.Speed=GPIO_SPEED_FREQ_HIGH;
 HAL_GPIO_WritePin(GPIOB,pin,GPIO_PIN_RESET);
 g.Mode=release?GPIO_MODE_INPUT:GPIO_MODE_OUTPUT_OD;HAL_GPIO_Init(GPIOB,&g);
}
static void data(int v){release_pin(GPIO_PIN_9,v);}
static dw_error clock_high(void){
 release_pin(GPIO_PIN_8,1);
 uint16_t t=board_us();
 while(!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8))if((uint16_t)(board_us()-t)>100)return DW_TIMEOUT;
 pause_bus();return DW_OK;
}
static void clock_low(void){release_pin(GPIO_PIN_8,0);pause_bus();}
static dw_error start(void){data(1);dw_error e=clock_high();if(e)return e;data(0);pause_bus();clock_low();return DW_OK;}
static void stop(void){data(0);clock_high();data(1);pause_bus();}
static dw_error put(uint8_t v){
 for(int i=0;i<8;i++){data((v&128)!=0);pause_bus();dw_error e=clock_high();if(e)return e;clock_low();v<<=1;}
 data(1);pause_bus();dw_error e=clock_high();if(e)return e;
 int ack=!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9);clock_low();return ack?DW_OK:DW_NACK;
}
static dw_error get(uint8_t *v,int last){
 *v=0;data(1);
 for(int i=0;i<8;i++){dw_error e=clock_high();if(e)return e;*v=(uint8_t)((*v<<1)|HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9));clock_low();}
 data(last);pause_bus();dw_error e=clock_high();clock_low();data(1);return e;
}
void gpio_i2c_init(void){
 __HAL_RCC_GPIOB_CLK_ENABLE();
 GPIO_InitTypeDef g={0};g.Pin=GPIO_PIN_8|GPIO_PIN_9;g.Mode=GPIO_MODE_INPUT;
 g.Pull=GPIO_PULLUP;g.Speed=GPIO_SPEED_FREQ_HIGH;
 HAL_GPIO_WritePin(GPIOB,g.Pin,GPIO_PIN_SET);HAL_GPIO_Init(GPIOB,&g);
}
dw_error gpio_i2c_read(uint8_t a,uint8_t r,uint8_t*b,size_t n){
 dw_error e=start();if(!e)e=put(a<<1);if(!e)e=put(r);
 if(!e)e=start();
 if(!e)e=put((a<<1)|1);
 for(size_t i=0;!e&&i<n;i++)e=get(&b[i],i+1==n);
 stop();return e;
}
dw_error gpio_i2c_write(uint8_t a,uint8_t r,const uint8_t*b,size_t n){
 dw_error e=start();if(!e)e=put(a<<1);if(!e)e=put(r);
 for(size_t i=0;!e&&i<n;i++)e=put(b[i]);
 stop();return e;
}


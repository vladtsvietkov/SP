#include "sensors.h"
#include <math.h>
dw_error dht_decode(const uint8_t f[5],int32_t *t,int32_t *rh) {
 if((uint8_t)(f[0]+f[1]+f[2]+f[3])!=f[4]) return DW_CHECKSUM;
 int32_t h=((int32_t)f[0]<<8)|f[1],v=((int32_t)(f[2]&127)<<8)|f[3];
 if(f[2]&128) v=-v;
 if(h>1000||v < -400||v>800) return DW_RANGE;
 *t=v; *rh=h; return DW_OK;
}
dw_error ntc_decode(uint16_t adc,float *t) {
 if(adc==0||adc>=4095) return DW_RANGE;
 float v=1.0f/(logf((float)adc/(4095-adc))/3950.0f+1.0f/298.15f)-273.15f;
 if(!isfinite(v)||v < -40||v>125) return DW_RANGE;
 *t=v; return DW_OK;
}
dw_error bmp_init(bmp180 *d,dw_bus b) {
 uint8_t id,raw[22]; d->ready=0; d->bus=b;
 dw_error e=b.read(b.ctx,0x77,0xd0,&id,1); if(e) return e;
 if(id!=0x55) return DW_ID;
 e=b.read(b.ctx,0x77,0xaa,raw,22); if(e) return e;
 for(int i=0;i<11;i++) {
  uint16_t v=(uint16_t)((raw[i*2]<<8)|raw[i*2+1]);
  if(v==0||v==65535) return DW_RANGE;
  d->c[i]=(i>=3&&i<=5)?v:(int16_t)v;
 }
 d->ready=1; return DW_OK;
}
dw_error bmp_compensate(const int32_t c[11],int32_t ut,int32_t up,int32_t *t,int32_t *p) {
 int64_t x1=((int64_t)(ut-c[5])*c[4])>>15;
 if(x1+c[10]==0) return DW_RANGE;
 int64_t x2=((int64_t)c[9]*2048)/(x1+c[10]), b5=x1+x2,b6=b5-4000;
 *t=(int32_t)((b5+8)>>4);
 x1=((int64_t)c[7]*((b6*b6)>>12))>>11;
 x2=((int64_t)c[1]*b6)>>11;
 int64_t b3=(((int64_t)c[0]*4+x1+x2)+2)>>2;
 x1=((int64_t)c[2]*b6)>>13; x2=((int64_t)c[6]*((b6*b6)>>12))>>16;
 int64_t x3=(x1+x2+2)>>2;
 int64_t b4=((int64_t)c[3]*(x3+32768))>>15;
 if(b4<=0||up<b3) return DW_RANGE;
 int64_t b7=(up-b3)*50000, pressure=(b7*2)/b4;
 x1=(pressure>>8)*(pressure>>8); x1=(x1*3038)>>16;
 x2=(-7357*pressure)>>16; pressure+=(x1+x2+3791)>>4;
 if(pressure<30000||pressure>110000||*t < -400||*t>850) return DW_RANGE;
 *p=(int32_t)pressure; return DW_OK;
}
dw_error bmp_read(bmp180 *d,int32_t *t,int32_t *p) {
 uint8_t cmd=0x2e,raw[3]; dw_error e;
 if(!d->ready) { e=bmp_init(d,d->bus); if(e) return e; }
 e=d->bus.write(d->bus.ctx,0x77,0xf4,&cmd,1); if(e) goto failed;
 d->bus.delay_ms(d->bus.ctx,5);
 e=d->bus.read(d->bus.ctx,0x77,0xf6,raw,2); if(e) goto failed;
 int32_t ut=(raw[0]<<8)|raw[1]; cmd=0x34;
 e=d->bus.write(d->bus.ctx,0x77,0xf4,&cmd,1); if(e) goto failed;
 d->bus.delay_ms(d->bus.ctx,5);
 e=d->bus.read(d->bus.ctx,0x77,0xf6,raw,3); if(e) goto failed;
 e=bmp_compensate(d->c,ut,((raw[0]<<16)|(raw[1]<<8)|raw[2])>>8,t,p);
failed:
 if(e) d->ready=0;
 return e;
}


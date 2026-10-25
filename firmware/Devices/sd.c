#include "sd.h"
static uint8_t x(dw_sd*d,uint8_t v){return d->byte(d->ctx,v);}
static void end(dw_sd*d){d->cs(d->ctx,1);x(d,255);}
static uint8_t command(dw_sd*d,uint8_t c,uint32_t a) {
 end(d);d->cs(d->ctx,0);x(d,255);
 x(d,(uint8_t)(64|c));x(d,a>>24);x(d,a>>16);x(d,a>>8);x(d,a);
 x(d,c==0?0x95:c==8?0x87:1);
 for(int i=0;i<16;i++){uint8_t r=x(d,255);if(!(r&128))return r;}return 255;
}
static int ready(dw_sd*d,uint32_t timeout) {
 uint32_t t=d->ms(d->ctx);
 do{if(x(d,255)==255)return 1;}while((uint32_t)(d->ms(d->ctx)-t)<timeout);
 return 0;
}
int sd_init(dw_sd*d) {
 end(d);for(int i=0;i<10;i++)x(d,255);
 if(command(d,0,0)!=1){end(d);return 0;}
 if(command(d,8,0x1aa)!=1){end(d);return 0;}
 uint32_t echo=0;for(int i=0;i<4;i++)echo=(echo<<8)|x(d,255);
 if(echo!=0x1aa){end(d);return 0;}
 uint32_t t=d->ms(d->ctx);uint8_t r;
 do{command(d,55,0);r=command(d,41,0x40000000);
 if(r==0)break;}while((uint32_t)(d->ms(d->ctx)-t)<1000);
 if(r!=0||command(d,58,0)!=0){end(d);return 0;}
 uint8_t o=x(d,255);d->high_capacity=(o&64)!=0;
 for(int i=0;i<3;i++)x(d,255);
 if(!d->high_capacity&&command(d,16,512)!=0){end(d);return 0;}
 end(d);return 1;
}
int sd_read(dw_sd*d,uint32_t sector,uint8_t data[512]) {
 if(command(d,17,d->high_capacity?sector:sector*512)!=0){end(d);return 0;}
 uint32_t t=d->ms(d->ctx);uint8_t token;
 do{token=x(d,255);if(token!=255)break;}while((uint32_t)(d->ms(d->ctx)-t)<200);
 if(token!=254){end(d);return 0;}
 for(int i=0;i<512;i++)data[i]=x(d,255);
 x(d,255);x(d,255);end(d);return 1;
}
int sd_write(dw_sd*d,uint32_t sector,const uint8_t data[512]) {
 if(command(d,24,d->high_capacity?sector:sector*512)!=0){end(d);return 0;}
 if(!ready(d,200)){end(d);return 0;}
 x(d,254);for(int i=0;i<512;i++)x(d,data[i]);x(d,255);x(d,255);
 uint8_t r=x(d,255);int ok=(r&31)==5&&ready(d,500);end(d);return ok;
}


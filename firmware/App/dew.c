#include "dew.h"
#include <math.h>
int dew_fresh(uint32_t now,uint32_t sampled,uint32_t limit,int valid) {
 return valid&&(uint32_t)(now-sampled)<=limit;
}
int dew_compute(float t,float rh,float s,float p,uint8_t valid,dew_result *r) {
 if(valid!=7||!isfinite(t)||!isfinite(rh)||!isfinite(s)||!isfinite(p)) return 0;
 if(t<0||t>50||rh<1||rh>100||s < -40||s>125||p<30000||p>110000) return 0;
 float g=logf(rh/100.0f)+17.62f*t/(243.12f+t);
 float e=611.2f*expf(17.62f*t/(243.12f+t))*rh/100.0f;
 if(p<=e) return 0;
 r->dew_C=243.12f*g/(17.62f-g);r->margin_C=s-r->dew_C;
 r->mixing_g_kg=621.98f*e/(p-e);
 return isfinite(r->mixing_g_kg);
}


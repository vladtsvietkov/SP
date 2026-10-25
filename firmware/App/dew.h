#ifndef DEW_MATH_H
#define DEW_MATH_H
#include <stdint.h>
typedef struct { float dew_C,margin_C,mixing_g_kg; } dew_result;
int dew_compute(float air_C,float rh_pct,float surface_C,float pressure_Pa,uint8_t valid,dew_result *);
int dew_fresh(uint32_t now,uint32_t sampled,uint32_t limit,int valid);
#endif


#ifndef DEW_SD_H
#define DEW_SD_H
#include <stdint.h>
typedef struct {
 void *ctx; uint8_t (*byte)(void *,uint8_t); void (*cs)(void *,int);
 uint32_t (*ms)(void *); int high_capacity;
} dw_sd;
int sd_init(dw_sd *);
int sd_read(dw_sd *,uint32_t,uint8_t[512]);
int sd_write(dw_sd *,uint32_t,const uint8_t[512]);
#endif


#ifndef DEW_SENSORS_H
#define DEW_SENSORS_H
#include <stdint.h>
#include <stddef.h>
typedef enum { DW_OK=0, DW_NACK, DW_TIMEOUT, DW_ID, DW_CHECKSUM, DW_RANGE, DW_BUSY, DW_BUS_ERROR } dw_error;
typedef struct {
 void *ctx;
 dw_error (*read)(void *,uint8_t,uint8_t,uint8_t *,size_t);
 dw_error (*write)(void *,uint8_t,uint8_t,const uint8_t *,size_t);
 void (*delay_ms)(void *,uint32_t);
} dw_bus;
typedef struct { int32_t c[11]; dw_bus bus; int ready; } bmp180;
dw_error bmp_init(bmp180 *,dw_bus);
dw_error bmp_read(bmp180 *,int32_t *temp_dC,int32_t *pressure_Pa);
dw_error bmp_compensate(const int32_t c[11],int32_t ut,int32_t up,int32_t *t,int32_t *p);
dw_error dht_decode(const uint8_t frame[5],int32_t *temp_dC,int32_t *rh_dPct);
dw_error ntc_decode(uint16_t adc,float *temp_C);
#endif


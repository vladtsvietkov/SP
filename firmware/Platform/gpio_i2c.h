#include "sensors.h"
void gpio_i2c_init(void);
dw_error gpio_i2c_read(uint8_t,uint8_t,uint8_t*,size_t);
dw_error gpio_i2c_write(uint8_t,uint8_t,const uint8_t*,size_t);

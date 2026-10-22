#ifndef DEWWINDOW_BOARD_H
#define DEWWINDOW_BOARD_H
#include "main.h"
extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;
void board_init(void);
uint32_t board_us(void);
void board_delay_us(uint16_t us);
#endif


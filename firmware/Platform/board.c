#include "board.h"
ADC_HandleTypeDef hadc1;
SPI_HandleTypeDef hspi1;
void board_init(void) {
  GPIO_InitTypeDef g={0};
  __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE(); __HAL_RCC_ADC_CLK_ENABLE();
  __HAL_RCC_SPI1_CLK_ENABLE(); __HAL_RCC_TIM3_CLK_ENABLE();
  g.Pin=GPIO_PIN_0; g.Mode=GPIO_MODE_ANALOG; g.Pull=GPIO_NOPULL; HAL_GPIO_Init(GPIOA,&g);
  g.Pin=GPIO_PIN_6; g.Mode=GPIO_MODE_INPUT; g.Pull=GPIO_PULLUP; HAL_GPIO_Init(GPIOA,&g);
  g.Pin=GPIO_PIN_1|GPIO_PIN_4; g.Mode=GPIO_MODE_OUTPUT_PP; g.Pull=GPIO_NOPULL; HAL_GPIO_Init(GPIOA,&g);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
  g.Pin=GPIO_PIN_0; HAL_GPIO_Init(GPIOB,&g);
  g.Pin=GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5; g.Mode=GPIO_MODE_AF_PP;
  g.Alternate=GPIO_AF0_SPI1; g.Speed=GPIO_SPEED_FREQ_HIGH; HAL_GPIO_Init(GPIOB,&g);
  hadc1.Instance=ADC1;
  hadc1.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution=ADC_RESOLUTION_12B; hadc1.Init.DataAlign=ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode=ADC_SCAN_DISABLE; hadc1.Init.EOCSelection=ADC_EOC_SINGLE_CONV;
  hadc1.Init.NbrOfConversion=1; hadc1.Init.ExternalTrigConv=ADC_SOFTWARE_START;
  hadc1.Init.SamplingTimeCommon1=ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.SamplingTimeCommon2=ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.Overrun=ADC_OVR_DATA_PRESERVED;
  if(HAL_ADC_Init(&hadc1)!=HAL_OK) Error_Handler();
  if(HAL_ADCEx_Calibration_Start(&hadc1)!=HAL_OK) Error_Handler();
  ADC_ChannelConfTypeDef ch={0}; ch.Channel=ADC_CHANNEL_0; ch.Rank=ADC_REGULAR_RANK_1;
  ch.SamplingTime=ADC_SAMPLINGTIME_COMMON_1;
  if(HAL_ADC_ConfigChannel(&hadc1,&ch)!=HAL_OK) Error_Handler();
  hspi1.Instance=SPI1; hspi1.Init.Mode=SPI_MODE_MASTER;
  hspi1.Init.Direction=SPI_DIRECTION_2LINES; hspi1.Init.DataSize=SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity=SPI_POLARITY_LOW; hspi1.Init.CLKPhase=SPI_PHASE_1EDGE;
  hspi1.Init.NSS=SPI_NSS_SOFT; hspi1.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit=SPI_FIRSTBIT_MSB; hspi1.Init.TIMode=SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE; hspi1.Init.CRCPolynomial=7;
  hspi1.Init.CRCLength=SPI_CRC_LENGTH_DATASIZE; hspi1.Init.NSSPMode=SPI_NSS_PULSE_DISABLE;
  if(HAL_SPI_Init(&hspi1)!=HAL_OK) Error_Handler();
  TIM3->PSC=47; TIM3->ARR=65535; TIM3->EGR=TIM_EGR_UG; TIM3->CR1=TIM_CR1_CEN;
}
uint32_t board_us(void) { return TIM3->CNT; }
void board_delay_us(uint16_t us) { uint16_t t=(uint16_t)board_us(); while((uint16_t)(board_us()-t)<us) {} }


# Підключення DewWindow

Джерела: [DS13867 Rev4](https://www.st.com/resource/en/datasheet/stm32c031c6.pdf),
табл.12 с.31–34, AF таблиці с.35–38; [UM2953 Rev2](https://www.st.com.cn/resource/en/user_manual/um2953-stm32-nucleo64-board-mb1717-stmicroelectronics.pdf),
§6.10, pinout та Arduino/morpho; [RM0490](https://www.st.com/resource/en/reference_manual/rm0490-stm32c0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf),
розділи RCC, I²C, SPI, ADC, TIM. З'єднувати за надписами PA/PB на Wokwi;
фізичний монтаж за Arduino-позначеннями нижче. Всі землі спільні.

| MCU | AF / блок | Роз'єм Arduino | Пристрій | Рівні / підтягування | Чому цей пін |
|---|---|---|---|---|---|
| PA0 | analog ADC1_IN0 | A0 | NTC OUT | 0…3.3 В, no pull | окремий аналоговий канал |
| PA6 | GPIO input/OD | D12 | DHT DATA | 3.3 В, зовнішні 5.1к, MCU pull-up як резерв у моделі | не займає I²C/SWD, програмний протокол |
| PB8 | AF6 I²C1 SCL | D15/SCL | BMP SCL | OD 3.3 В, зовнішні 4.7к | штатний SCL плати |
| PB9 | AF6 I²C1 SDA | D14/SDA | BMP SDA | OD 3.3 В, зовнішні 4.7к | штатний SDA, адреса 0x77 |
| PA2/PA3 | AF1 USART2 TX/RX | ST morpho PA2/PA3, VCP | serial monitor | 3.3 В, no pull | VCP через SB27/SB32; Arduino D1/D0 — PB6/PB7 |
| PA5 | GPIO output | D13 | LED LD4 | бортова обв'язка | збережено LED |
| PA1 | GPIO output | A1 | ADC marker | 3.3 В, no pull | ADC не використовує канал1 |
| PA4 | GPIO output | A2 | work marker | 3.3 В, no pull | незалежний від SD CS |
| PB3/4/5 | AF0 SPI1 SCK/MISO/MOSI | D3/D5/D6 | SD SCK/DO/DI | 3.3 В, CS high при старті | не конфліктує з PA5 LED/PA6 DHT |
| PB0 | GPIO output | D10 | SD CS | 3.3 В, 10к до3.3 В на фізичному стенді | керування CS на весь command/data; model alias PB0.1 |

BMP180 ID D0=0x55, адреса 0x77 фіксована. DHT і NTC не мають chip ID;
їх self-test — checksum/range та правдоподібний ADC. SD: CMD8 echo=0x1AA,
OCR через CMD58; bring-up окремий, не є журналом епізодів.

HSI48/SYSCLK/AHB/APB=48 МГц. I²C TIMINGR=0x20303E5D:
PRESC=2 →62.5 нс, SCLL=93 →5.875 мкс, SCLH=62 →3.9375 мкс;
близько100 кГц з урахуванням фільтра/синхронізації, остаточно звірити VCD.
SPI=48М/128=375кГц, mode0. TIM3=48М/(47+1)=1МГц; 16-бітний rollover.
ADC=48М/4=12МГц, sample160.5 cycles, 12bit; близько14.4 мкс/вибірку.
USART2=115200 8N1, байт=86.8 мкс. DMA не використовується, конфліктів channel немає.
SWD PA13/PA14, кварцові PF0/PF1 і PC14/PC15, кнопка PC13 залишені вільними від датчиків.

Відхилено SPI PA5/PA6/PA7: спільні PA5 LED і PA6 DHT.
Відхилено I²C PB6/PB7: штатні PB8/PB9 вже налаштовані й доступні на SCL/SDA.
Відхилено probes PC0/PC1: їх немає у C031C6 LQFP48.

Пізніша ізольована перевірка C031 у ЛР2 показала OUTPUT_OD+ODR1 → IDR0,
а INPUT_PULLUP → IDR1. Тому адресний ACK апаратного bring-up ЛР1 не доводить
готовності BMP180; доведені реальні такти SCL. Повний обмін сенсорів належить ЛР2.

Живлення і струми: [hardware-components.md](hardware-components.md).
В моделі pull-up можуть бути ідеалізовані; на фізичній платі перевірити
rise time, сумарний опір паралельних pull-up, VDDA і струм носія.
Окремий стабілізатор SD 3.3 В із запасом≥250 мА; 5V виходи до ADC не підключати.

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
| PB8 | GPIO, I²C SCL (AF6 доступна, неактивна) | CN5-10 D15/SCL | BMP SCL | 3.3 В, 4.7к; LOW=OD output, HIGH=input pull-up | стандартний SCL; обхід спостереженої поведінки OD моделі |
| PB9 | GPIO, I²C SDA (AF6 доступна, неактивна) | CN5-9 D14/SDA | BMP SDA | 3.3 В, 4.7к; LOW=OD output, HIGH=input pull-up | стандартний SDA, адреса0x77 |
| PA2/PA3 | AF1 USART2 TX/RX | ST morpho PA2/PA3, VCP | serial monitor | 3.3 В, no pull | VCP через SB27/SB32; Arduino D1/D0 — PB6/PB7, не ці піни |
| PA5 | GPIO output | D13 | LED LD4 | бортова обв'язка | збережено LED |
| PA1 | GPIO output | A1 | ADC marker | 3.3 В, no pull | ADC не використовує канал1 |
| PA4 | GPIO output | A2 | work marker | 3.3 В, no pull | незалежний від SD CS |
| PB3/4/5 | AF0 SPI1 SCK/MISO/MOSI | CN9-4 D3 / CN9-6 D5 / CN9-7 D6 | SD SCK/DO/DI | 3.3 В, CS high при старті | не конфліктує з PA5 LED/PA6 DHT |
| PB0 | GPIO output | CN5-3 D10 | SD CS | 3.3 В, 10к до3.3 В на фізичному стенді | керування CS на весь command/data; Wokwi alias PB0.1 |

BMP180 ID D0=0x55, адреса 0x77 фіксована. DHT і NTC не мають chip ID;
їх self-test — checksum/range та правдоподібний ADC. SD: CMD8 echo=0x1AA,
OCR через CMD58; bring-up окремий, не є журналом епізодів.

HSI48/SYSCLK/AHB/APB=48 МГц. У ЛР1 апаратний I²C TIMINGR=0x20303E5D давав
такти100 кГц, але ACK не доводив правильної роботи SDA. Ізольований тест ЛР2
показав OUTPUT_OD+ODR=1 → IDR=0, тоді як INPUT_PULLUP → IDR=1.
Тому фінальний I²C — програмний стандартний wire protocol через HAL GPIO:
відпускання входом із pull-up, керування лише LOW; TIM3 delay5 мкс на фазу
плюс витрати HAL. Фактичну частоту слід брати з VCD, а не називати100 кГц.
Timeout clock-stretch100 мкс, кожна помилка завершується STOP.
SPI=48М/128=375кГц, mode0. TIM3=48М/(47+1)=1МГц; 16-бітний rollover.
ADC=48М/4=12МГц, sample160.5 cycles, 12bit; близько14.4 мкс/вибірку.
USART2=115200 8N1, байт=86.8 мкс. DMA не використовується, конфліктів channel немає.
SWD PA13/PA14, кварцові PF0/PF1 і PC14/PC15, кнопка PC13 залишені вільними від датчиків.

Відхилено SPI PA5/PA6/PA7: спільні PA5 LED і PA6 DHT.
Відхилено I²C PB6/PB7: штатні PB8/PB9 вже налаштовані й доступні на SCL/SDA.
Відхилено probes PC0/PC1: їх немає у C031C6 LQFP48.

Точні імена power/CS у diagram.json: nucleo:3V3.2, nucleo:PB0.1.
Лінтер Wokwi0.29.1 перевірив усі pin IDs; board-bmp180 має інформаційне
попередження outdated registry, хоча модель документована поточним Wokwi.

Живлення і струми: [hardware-components.md](hardware-components.md).
В моделі pull-up можуть бути ідеалізовані; на фізичній платі перевірити
rise time, сумарний опір паралельних pull-up, VDDA і струм носія.
Окремий стабілізатор SD 3.3 В із запасом≥250 мА; 5V виходи до ADC не підключати.

# Компоненти DewWindow
Плата Nucleo-C031C6, STM32C031C6T6 LQFP48, 48 МГц, 32 КіБ flash/12 КіБ RAM.
Джерела різних рівнів не взаємозамінні: DS13867 — піни/AF, RM0490 — периферія,
UM2953 — роз'єми/обв'язка плати. Pinout і clock показано у firmware.ioc,
реальні init — Core і Platform/board.c.

| Пристрій | Дані, інтерфейс і живлення | Конкретне джерело |
|---|---|---|
| AM2302/DHT22 | 3.3…5.5 В, типовий струм вимірювання 1.2 мА, sleep 15 мкА; цикл не частіше 2 с. DATA open-drain, зовнішній pull-up 5.1 кОм до 3.3 В; немає ID-регістру. 40 біт RH/T/checksum, sign-magnitude T. | [Aosong Technical Manual V1.0](https://www.aosong.com/uploadfiles/2025/04/20250417105409216.pdf), с.2 таблиці 1–3; с.5–7 схема/протокол. |
| NTC | TDK B57891M0103J000: 10 кОм при 25 °C, B25/100=3950 K ±3%, R ±5%. Пасивний дільник 10 кОм; 3.3 В, при 25 °C 0.165 мА і 0.272 мВт на NTC. ADC OUT≤VDDA; немає ID. | [TDK B57891M](https://product.tdk.com/en/system/files/dam/doc/product/sensor/ntc/ntc_element/data_sheet/50/db/ntc/ntc_leaded_disks_m891.pdf), електрична таблиця с.3, характеристика 4901. [Wokwi NTC](https://docs.wokwi.com/parts/wokwi-ntc-temperature-sensor), модель 10к/10к, beta=3950. |
| BMP180 | VDD 1.8…3.6 В, VDDIO 1.62…3.6 В; I²C 0x77, ID D0=55; калібрування AA…BF, control F4, data F6…F8. UT:2E/4.5 мс, UP:34/4.5 мс при OSS0. Peak 650 мкА, standby 0.1 мкА (тип.). | [Bosch BST-BMP180-DS000-09](https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf), оригінальний документ Bosch у дзеркалі дистриб'ютора; с.6 таблиця1, с.11–15 conversion/compensation, с.18 register map, с.19–23 I²C. |
| microSD SPI | 3.3 В, mode0, init 375 кГц; CMD0/8/55/41/58/16/17/24. Окремий носій для ЛР3, raw sectors, без FAT у прошивці. Реальна карта може мати струм понад можливості 3V3 Nucleo: окремий стабілізатор 3.3 В ≥250 мА і спільний GND. | [SD Association simplified specifications](https://www.sdcard.org/downloads/pls/), Physical Layer SPI mode; [Wokwi microSD](https://docs.wokwi.com/parts/wokwi-microsd-card). Конкретну фізичну карту не придбано, її peak ще треба перевірити. |

У моделі NTC немає гарантованого виробника, це параметричний аналог, а не
цифровий двійник конкретного TDK. B-формула є наближенням; не заявляється
точність контактного термометра без калібрування. RH біля насичення також
не гарантує точність: довга експозиція високій вологості спричиняє drift.

Сенсорний бюджет 3V3: DHT 1.2 мА + BMP 0.65 мА + NTC 0.165 мА +
два I²C pull-up 4.7к (до 1.4 мА) + DATA pull-up (0.65 мА) ≈4.1 мА;
виділяється 10 мА. Це розрахунок, не вимір струму симулятором.
Усі сигнали на 3.3 В, силових приводів немає.
microSD не включено в цей сенсорний бюджет.
Для DHT таблиця2 Aosong задає типові струми, але не числовий maximum measuring peak;
10мА є виділеним запасом сенсорної лінії, а не виміряним або гарантованим worst-case.
У NTC-дільнику струм обмежений резистором10к:≤0.33мА при3.3В; найбільша потужність
на самому NTC≈0.272мВт при Rntc=10к. Фізичні піки DHT/SD потрібно виміряти окремо.

[ST DS13867 Rev4](https://www.st.com/resource/en/datasheet/stm32c031c6.pdf):
§4, таблиці pin definitions/alternate functions.
[ST RM0490](https://www.st.com/resource/en/reference_manual/rm0490-stm32c0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf):
RCC/GPIO/ADC/I²C/SPI/TIM.
[ST UM2953 Rev2](https://www.st.com.cn/resource/en/user_manual/um2953-stm32-nucleo64-board-mb1717-stmicroelectronics.pdf):
§6.3 power, §6.7 VCP, §6.10 connectors, таблиці Arduino/morpho.
Камера не застосовується у формі B.


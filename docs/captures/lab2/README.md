# Власні записи DewWindow, 06.11.2026

ELF SHA-256: 19a76b45f77a8d153b7869e900059f516d3c6c081193cac7b3be45f2c6767529.
Native Wokwi VCD має1 нс timebase, buffer2000000 transitions; це події моделі, не1 ГГц фізичний ADC.
Запис починається при startup. Збережені VCD не використовуються як writable demo output.

| Запис | Карта / параметри | Призначення |
|---|---|---|
| sensors-fault-recovery.vcd | diagram-sensors.json: D0 PB8 SCL, D1 PB9 SDA, D2 PA2 UART, D3 PA6 DHT, D4 PA1 ADC marker, D5 PA5 LED, D6 PA4, D7 PB3 SCK |744761.973 мс моделі, raw основного bring-up та відмови0x76/recover |
| startup-5s.vcd, startup.sr | Перші5 с canonical, абсолютні timestamps збережені; .sr1 МГц | ID, calibration, raw BMP, DHT, UART, ADC marker |
| fault-first-105s.vcd | Перші105 с canonical,1 МГц при декодуванні | nack/recover і нові дані |
| i2c-id-35ms.vcd, i2c-id.sr | Перші35 мс canonical; .sr1 МГц | Читабельний ID-транзакційний вид PulseView |
| spi-reset.vcd | diagram-spi.json: D0 PB3 SCK, D1 PB5 MOSI, D2 PB4 MISO, D3 PB0 CS, D4 PA1 marker, D5 PA2 UART, D6 PA6 DHT, D7 PB8 SCL | Новий запуск збереженого образу: prior1 |
| spi-write-read.vcd | Та сама SPI mapping і незмінний ELF; новий blank demo image | Повний CMD24/payload/busy/CMD17 |
| spi-startup-500ms.vcd, spi-startup.sr | Перші500 мс попереднього; .sr10 МГц | Повний startup SD, до265.933 мс |

Сенсорний downsample1000 з ns дає1 МГц; SPI downsample100 —10 МГц.
При375кГц останній дає≈26.7 samples/clock;1 МГц для SPI недостатній для точного timing.
Native timing рахується без ресемплінгу. Після зупинки симулятора SD writeback зберігає образ.
Reset/read-back не дорівнює аварійному power-loss test.

З кореня проєкту (спочатку звичайний cd, як у README):

```sh
sigrok-cli -i docs/captures/lab2/startup.sr -P i2c:scl=D0:sda=D1 --protocol-decoder-samplenum
sigrok-cli -i docs/captures/lab2/startup.sr -P uart:rx=D2:baudrate=115200:format=hex -A uart=rx-data
sigrok-cli -i docs/captures/lab2/startup.sr -P am230x:sda=D3 --protocol-decoder-samplenum
sigrok-cli -i docs/captures/lab2/spi-startup.sr -P spi:clk=D0:mosi=D1:miso=D2:cs=D3:cpol=0:cpha=0 --protocol-decoder-samplenum
python3 tools/capture_fixture.py
python3 tools/vcd_timing.py docs/captures/lab2/sensors-fault-recovery.vcd
python3 tools/decode_uart.py docs/captures/lab2/sensors-fault-recovery.vcd docs/metrics/demo-uart-decoded.log
python3 tools/decode_spi.py docs/captures/lab2/spi-startup-500ms.vcd
```

Байти з sigrok: i2c-startup.txt, dht-startup.txt, uart-startup.txt, spi-startup.txt.
manual-i2c.json містить SDA на дев’яти фронтах для EE/ACK і D0/ACK.
fixtures/captured.h у tests отримано з власного capture, не із чужого результату.
uart-full-from-vcd.log декодовано з native RX; старт збігається з sigrok,0 bad stop bits.
Серед2979 NTC-рядків всі сусідні t_ms відрізняються на250 мс. Останній неповний frame відкинуто;
кінцевий неповний текстовий рядок не означає помилку UART. Це не годинний S2.

Невдалі hardware I²C записи мають failed/cold/hardware назви й не є pass-доказами.
Candidate ELF для двох timeout спроб збережені окремо від фінального firmware.elf.
Зміна на GPIO direction release перевірена окремим OD-пробником, потім інтегрованою прошивкою.
Нормальний останній master NACK читання не слід плутати з address NACK0x76.
Input probe було передано після recover, але його виконання не підтвердилося: прийняття цього
додаткового запиту не зараховано. Власне nack/recover мають DIAG ACK і зміни стану на сигналах.

Детальний аналіз і виміряні межі — ../../lab2-report.md; hashes — manifest.json.

## Детальні вікна PulseView

| Сесія | Вихідне вікно | Канали й імпорт |
|---|---|---|
| dht-detail.sr | sensors:2779…2788 мс | SDA (у GUI DATA),1 МГц, AM230x SDA=DATA |
| uart-detail.sr | sensors:3.4…5.4 мс | RX,10 МГц, UART115200/8N1 |
| adc-detail.sr | sensors:200…1200 мс | ADC,1 МГц, GPIO marker8 конверсій |
| spi-detail.sr | SPI:157…217 мс | CLK/MOSI/MISO/CS#,10 МГц, mode0 active-low |

Вікна отримані tools/vcd_window.py: початкові стани збережено, час відлічено від початку
вікна; *.window.json містять origin та SHA-256 джерела. Нових переходів не синтезовано.
*-named.sr змінюють лише назви каналів; logic bytes порівняні з первинними .sr.
Детальні скриншоти PulseView зберігаються в docs/screenshots/lab2; raw та crop provenance
залишені локально поруч із GUIDE. Відносний час GUI не є новим запуском прошивки.

# DewWindow
Цвєтков Владислав Олександрович, ІО-32.

Оцінює ризик конденсації за T/RH повітря, температурою поверхні й тиском.
Ubuntu 24.04, Docker, STM32 HAL, Wokwi; Nucleo-C031C6.

Гілка `lab1` містить перевірені HAL init, UART, LED і SCL probe;
дві чисті збірки дали однакові ELF/HEX/BIN.
Гілка `lab2` додає одночасні DHT22, NTC, BMP180 та SPI microSD bring-up.
[Звіт ЛР1](docs/report.md), [звіт ЛР2](docs/lab2-report.md), PDF у Reports;
[власні захвати](docs/captures/lab2), [метрики й помилки](docs/metrics/lab2).

## Збірка і запуск
```sh
cd "/Users/pliffdax/Projects/university/Study/People/Цвєтков Владислав Олександрович/2026-Fall/SP/Lab01/Work/SP-Labs"
make build IMAGE=dewwindow-build:lab2
make test
make coverage
make media
python3 tools/vcd_timing.py docs/captures/lab2/sensors-fault-recovery.vcd
```
На Mac за потреби: DOCKER=/Applications/Docker.app/Contents/Resources/bin/docker.
Відкрити цю папку у VS Code, Wokwi: Start Simulator. make flash/monitor показують
відповідні дії для форми B. Звичайний wokwi.toml використовує firmware/build/firmware.hex
та ELF цієї збірки; RFC2217 localhost:4417 працює лише під час симуляції.
Не запускати два симулятори цього проєкту одночасно.
Python-колектори потребують pyserial. Під час симуляції, з цього самого кореня:
`python3 tools/collect_uart.py docs/metrics/demo-uart.log --seconds 10`.
Для контрольованого NACK і recovery: `python3 tools/run_fault_recovery.py`.
Власний ELF вимірювань: `docs/firmware/lab2/firmware.elf`; SHA-256 починається `19a76b45`.
Нова збірка перевіряється за повним SHA-256 із `docs/metrics/lab2/firmware-final.sha256`.
Основний diagram має sensor mapping; `diagram-spi.json` зберігає окремий mapping SPI-запису.
Новий `wokwi.vcd` і writable media відокремлені від збережених результатів.

## Документи
[PRD](docs/PRD.md) · [контракт FR01–10](docs/requirements.md) ·
[компоненти](docs/hardware-components.md) · [pinout](docs/pinout.md) ·
[архітектура](docs/architecture.md) · [план ЛР3–4](docs/future-experiments.md) ·
[походження дозволеної заготовки](docs/starter-template.md).

Повний журнал/епізоди/команди реалізуються в ЛР3, FreeRTOS — у ЛР4.
Baseline ще не виміряна. Відео і PR є окремими матеріалами здачі.
Локальні перевірки: 721 assertion; рядки Devices/App 97.3–100%.
GPIO-I²C у моделі працює приблизно3.95кГц; це враховане обмеження, не100кГц.
Фізичні струми, аварійний power-cut, повний продуктовий FR і RTOS тут не перевірені.


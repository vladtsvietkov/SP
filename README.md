# DewWindow
Цвєтков Владислав Олександрович, ІО-32.

Оцінює ризик конденсації за T/RH повітря, температурою поверхні й тиском.
Ubuntu 24.04, Docker, STM32 HAL, Wokwi; Nucleo-C031C6.

## ЛР1
HAL init, UART 115200 8N1, LED і I²C probe перевірено у Wokwi.
Дві чисті збірки дали однакові ELF/HEX/BIN; [логи/хеші](docs/metrics/lab1),
[UART/VCD](docs/captures/lab1), [звіт](docs/report.md), PDF у Reports.
Сенсорні драйвери не входять до гілки lab1.

## Збірка і запуск
```sh
cd "/Users/pliffdax/Projects/university/Study/People/Цвєтков Владислав Олександрович/2026-Fall/SP/Lab01/Work/SP-Labs"
make build IMAGE=dewwindow-build:lab1
make clean
```
На Mac за потреби: DOCKER=/Applications/Docker.app/Contents/Resources/bin/docker.
Відкрити цю папку у VS Code, Wokwi: Start Simulator. make flash/monitor показують
відповідні дії для форми B. Звичайний wokwi.toml використовує firmware/build/firmware.hex
та ELF цієї збірки; RFC2217 localhost:4417 працює лише під час симуляції.
Не запускати два симулятори цього проєкту одночасно.

## Документи
[PRD](docs/PRD.md) · [контракт FR01–10](docs/requirements.md) ·
[компоненти](docs/hardware-components.md) · [pinout](docs/pinout.md) ·
[архітектура](docs/architecture.md) · [план ЛР3–4](docs/future-experiments.md) ·
[походження дозволеної заготовки](docs/starter-template.md).

Повний журнал/епізоди/команди реалізуються в ЛР3, FreeRTOS — у ЛР4.
Baseline ще не виміряна. Відео і PR є окремими матеріалами здачі.


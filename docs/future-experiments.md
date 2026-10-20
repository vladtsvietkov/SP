# План ЛР3–4 (ще не результати)
## ЛР3
На тій самій схемі реалізувати FR01–10, status/limits/episodes/export плюс STATS/MEM.
Залишити драйвери ЛР2 й простий super loop. Реальний запис SD, без штучної затримки.
TIM3 1 МГц (16 біт, 65.536 мс): короткі інтервали modulo uint16; для довгих інтервалів
розширити лічильник через overflow ISR або звірений SysTick. C031 Cortex-M0+ не має DWT CYCCNT.
Перевірити TIM3 частоту GPIO проти VCD та SysTick; зазначити обмеження точності симулятора.
PA1 marker ADC/задачі, PA4 marker обчислення/запису; логи містять t_ms MCU і ELF SHA256.
UART час прибуття на Mac не є часом вимірювання.

S1: 300 с MCU, T24/RH60/NTC24/p101325, без команд, без епізодів.
S2: неперервні ≥3600000 мс MCU; раз на 120 с NTC 24→14 на 20 с→24,
status кожні 60 с, export кожні 600 с. Автоматичний collector перевіряє continuity,
sequence, errors, heap, журнал; початковий контроль≤10 хв, потім один wakeup на решту.
S3: 300 с MCU; export і реальні commits SD, потік status 10/с,
NTC перетинає 0 °C margin у 30 різних фазах відносно запису. Не підміняти довгу операцію delay.
Додатково §6.2: запис проти періодичних release; ≥30 подій під час запису;
UART без пауз/overflow; BMP відсутній і SDA low/recovery; підвищувати команди 1/5/10/20/50 Гц
до першого порушення FR; ≥30 startup; power-cut у команді, data token, payload і busy SD.
У кожній фазі ≥10 повторів; попередній committed запис має вижити.
Зберігати failed runs і inputs, UART/VCD/образи/хеші.

Для кожної задачі min/avg/max, n, jitter, missed; ≥тисячі release (NTC 14400 за годину).
Для подій min/avg/max/p99 ≥30. CPU: рахувати лише виконану роботу, окремо UART/idle,
не видавати polling wait за корисну роботу. Size/map за модулями, -fstack-usage +
stack painting у S3, heap arena/peak або доведена відсутність malloc.
Скласти ≥5 виміряних обмежень різних категорій; зараз це лише питання дослідження.
Baseline ЛР4 заповнюється після ЛР3, тут немає виміряних колонок.

## ЛР4
Офіційний FreeRTOS-Kernel із зафіксованою версією, порт для Cortex-M0 (не ARM_CM4F).
Зберегти buildable lab3 і повторити обидва бінарники на тому самому стенді в один день.
Драйвери/FR/команди/формати зберегти; дизайн задач визначити за виміряними межами ЛР3.
Окремі експерименти §7.2: priority inversion mutex vs binary semaphore; stack overflow hook
і варіант без контролю; starvation; queue overflow; heap exhaustion; tick 1000 vs 100 Гц.
Виміряти runtime stats, idle, MSP/стек кожної задачі, heap reserved/used,
tick/context-switch/ISR→task та flash/RAM ядра. TIM3 замість недоступного DWT.
Кожну гіпотезу ЛР3 перевірити, включно з можливими перевагами super loop.
Не вимагати, щоб RTOS вигравав кожне число.
ЛР5–6 не визначаються до отримання оригінальних завдань.


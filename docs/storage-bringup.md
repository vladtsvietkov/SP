# Носій: SPI microSD, межі bring-up

Обрано штатну модель wokwi-microsd-card, не RAM і не internal flash.
SPI1 PB3/4/5, CS PB0; mode0, 375кГц. Один статичний512-байтовий буфер.
Драйвер HAL_SPI_TransmitReceive у Platform; протокол CMD0/8/55/41/58/16/17/24
у Devices/sd.c. Timeout init1с, read200мс, write busy500мс.

make media створює лише відсутній media/dew-demo.img, не стирає наявний.
Звичайний wokwi.toml має raw image, size4M, writeback=true. Це демонстраційний
носій; виміряні образи зберігаються окремо від нього.
При startup: прочитати sector2048, перевірити magic DewWindow-storage-v1.
Якщо його немає — записати block512 байт з magic і payload0xA5, прочитати назад.
SELF dev=SD повідомляє init/prior/write/readback/hc. При наступному запуску prior=1
має підтвердити попередню операцію. Перед/після зупинки зберегти образ/хеш.

Це перевірка носія, не реалізація журналу епізодів ЛР3. Нормальна зупинка Wokwi
експортує writeback; сама по собі не доводить стійкості до аварійного power-cut.
У ЛР3 потрібні два слоти, CRC32, sequence і commit marker, перевірки reset/обриву
в різних фазах реального запису. При reset не з'єднувати перервані епізоди.
RAM та штучні delay не використовуються як докази збереження.

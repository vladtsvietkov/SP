IMAGE ?= stm32c0-wokwi-build:local
DOCKER ?= docker

.PHONY: image build clean

image:
	$(DOCKER) build -t $(IMAGE) docker

build: image
	$(DOCKER) run --rm -v "$(CURDIR):/workspace" -w /workspace \
		--user "$(shell id -u):$(shell id -g)" $(IMAGE) \
		make -C firmware all

clean:
	$(MAKE) -C firmware clean

.PHONY: flash monitor
flash:
	@echo "VS Code: Wokwi: Start Simulator (or restart); firmware/build/firmware.hex"
monitor:
	@echo "Wokwi Serial Monitor, 115200 8N1; RFC2217 localhost:4417 while running"

HOST_CC ?= cc
.PHONY: test coverage check-layers media
test: check-layers
	mkdir -p build-tests
	rm -f build-tests/*.gcda build-tests/*.gcno
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror --coverage -Ifirmware/Devices -Ifirmware/App firmware/Devices/sensors.c firmware/Devices/sd.c firmware/App/dew.c tests/test_dewwindow.c -lm -o build-tests/test
	./build-tests/test
coverage: test
	python3 tools/coverage.py
check-layers:
	python3 tools/check_layers.py
media:
	python3 tools/init_media.py

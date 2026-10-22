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

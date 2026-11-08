QMAKE ?= qmake6
BUILD_DIR := build
TARGET := ZomboGIG

all:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(QMAKE) ../ZomboGIG.pro
	$(MAKE) -C $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/$(TARGET)

clean:
	@if [ -f $(BUILD_DIR)/Makefile ]; then $(MAKE) -C $(BUILD_DIR) clean; fi
	rm -rf $(BUILD_DIR)

re: clean all

.PHONY: all run clean re

CROSS_COMPILE = 

cc = gcc

ifneq ($(GCC), )
	cc = $(GCC)
else ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
	cc = aarch64-none-linux-gnu-gcc
else
	cc = $(CROSS_COMPILE)gcc
endif

BIN = aesdsocket

all:$(BIN)

aesdsocket:./aesdsocket.c
	$(cc) $^ -o $(BIN)

default:$(all)

clean:
	rm -rf $(BIN)
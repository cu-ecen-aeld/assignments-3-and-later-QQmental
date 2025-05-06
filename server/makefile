CROSS_COMPILE = 

cc = gcc

ifneq ($(GCC), )
	cc = $(GCC)
else ifneq ($(CROSS_COMPILE), )
	cc = $(CROSS_COMPILE)gcc
else
	cc = aarch64-none-linux-gnu-gcc
endif

BIN = aesdsocket

all:$(BIN)

aesdsocket:./aesdsocket.c
	$(cc) $^ -o $(BIN)

default:$(all)

clean:
	rm -rf $(BIN)
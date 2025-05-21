CROSS_COMPILE = 
GCC = gcc
cc = $(GCC)

ifneq ($(GCC), )
	cc = $(GCC)
else ifneq ($(CROSS_COMPILE), )
	cc = $(CROSS_COMPILE)gcc
else
	cc = aarch64-none-linux-gnu-gcc
endif

CFLAG += -lpthread -g

BIN = aesdsocket

all:$(BIN)

aesdsocket:./aesdsocket.c
	$(cc) $^ $(CFLAG) -o $(BIN)

default:$(all)

clean:
	rm -rf $(BIN)

#make GCC=x86_64-linux-gnu-gcc
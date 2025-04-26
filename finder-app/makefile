CROSS_COMPILE = 

cc = gcc

ifneq ($(GCC), )
	cc = $(GCC)
else ifeq ($(CROSS_COMPILE), )
	cc = aarch64-none-linux-gnu-gcc
else
	cc = $(CROSS_COMPILE)gcc
endif


BIN = ./writer

all:$(BIN)
	@exit 0 

$(BIN):writer.c
	$(cc) writer.c -o $@
#ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
#	@file $(BIN) | cat > ../assignments/assignment2/fileresult.txt
#endif

clean:
	rm -rf $(BIN)

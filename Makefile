TARGET=gprs
OBJ=lnklst.o serial.o telLibrary.o uni.o utils.o sms_menu.o green_ctrl.o
CFLAG=-Wall -O2
GCC=arm-linux-gcc

OFFSET=\x1b[41G
COLOR=\x1b[1;34m
RESET=\x1b[0m
CLEAR=\x1b[H\x1b[J

$(TARGET): main.c $(OBJ)
	@echo -n "Generating $@..."
	@if $(GCC) $(CFLAG) -o $@ $^ -lpthread; then echo -e "$(OFFSET)$(COLOR)[ OK ]$(RESET)"; fi

%.o: %.c
	@echo -n "Compiling $<..."
	@if $(GCC) $(CFLAG) -c -o $@ $<; then echo -e "$(OFFSET)$(COLOR)[ OK ]$(RESET)"; else exit 1; fi

%.o: %.c %.h
	@echo -n "Compiling $<..."
	@if $(GCC) $(CFLAG) -c -o $@ $<; then echo -e "$(OFFSET)$(COLOR)[ OK ]$(RESET)"; else exit 1; fi

clean:
	@echo -n "Cleanning up..."
	@rm -rf *.o *.bak *~ $(TARGET)
	@echo -e "$(OFFSET)$(COLOR)[ OK ]$(RESET)"

clear:
	@echo -ne "$(CLEAR)Cleanning up..."
	@rm -rf *.o *.bak *~ $(TARGET)
	@echo -e "$(OFFSET)$(COLOR)[ OK ]$(RESET)"

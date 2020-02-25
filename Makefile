PROJECTNAME = bootloader

COMBONAME = ../DLD/build/combo
MAINAPP_DIR = ../DLD/
MAINAPP_HEX = ../DLD/build/main.hex

BUILDDIR = build

PROJECT = $(BUILDDIR)/$(PROJECTNAME)
COMBO = $(BUILDDIR)/$(COMBONAME) 

STARTUP = startup_stm32f427_437xx.s

SOURCES = 	$(STARTUP) \
			system_stm32f4xx.c \
			bootloader.cc \
			dig_inouts.c \
			bootloader_utils.cc \
			system_clock.cc \
			system.cc \
			misc.c \
			stm32f4xx_flash.c \
			stm32f4xx_gpio.c \
			stm32f4xx_rcc.c \
			stm32f4xx_tim.c \
			stm32f4xx_i2c.c \
			stm32f4xx_spi.c \
			stm32f4xx_dma.c \
			codec.c \
			i2s.c \
			encoding/fsk/packet_decoder.cc 
		
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

LDSCRIPT = stm32f429xx.ld

ARCHFLAGS = -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4 -mfloat-abi=soft -mfpu=fpv4-sp-d16 
F_CPU = 168000000L

FLAGS = -g2 -Os $(ARCHFLAGS)
FLAGS += -I. -DARM_MATH_CM4 -D'__FPU_PRESENT=1' -DF_CPU=$(F_CPU)
FLAGS += -fsingle-precision-constant -Wdouble-promotion 


CFLAGS = -std=c99
CFLAGS += $(FLAGS)

CPPFLAGS = -fno-exceptions
CPPFLAGS += $(FLAGS)

AFLAGS  = $(ARCHFLAGS)
LFLAGS  = -Wl,-Map=$(PROJECT).map -Wl,--gc-sections \
	-T $(LDSCRIPT) \
	-I.

# Executables


ARCH 	= arm-none-eabi
CC 		= $(ARCH)-gcc
CXX 	= $(ARCH)-g++
LD 		= $(ARCH)-gcc
AS 		= $(ARCH)-as
OBJCPY 	= $(ARCH)-objcopy
OBJDMP 	= $(ARCH)-objdump
GDB 	= $(ARCH)-gdb
SZ 		= $(ARCH)-size

CPFLAGS = -O binary
ODFLAGS	= -x --syms

FLASH = st-flash

# Targets
all: $(PROJECT).bin

clean:
	-rm -rf build/

flash: stlink_flash

stlink_flash: $(PROJECT).bin
	$(FLASH) write $(PROJECT).bin 0x08000000

combo_flash: combo
	$(FLASH) write $(COMBO).bin 0x08000000
	
$(PROJECT).hex: $(PROJECT).elf 
	$(OBJCPY) -O ihex $< $@
		

combo: $(COMBO).bin 

$(COMBO).bin:  $(MAINAPP_HEX) $(PROJECT).hex
	cat  $(MAINAPP_HEX) $(PROJECT).hex | \
	awk -f util/merge_hex.awk > $(COMBO).hex
	$(OBJCPY) -I ihex -O binary $(COMBO).hex $(COMBO).bin


$(PROJECT).bin: $(PROJECT).elf 
	$(OBJCPY) $(CPFLAGS) $(PROJECT).elf $(PROJECT).bin
	$(OBJDMP) $(ODFLAGS) $(PROJECT).elf > $(PROJECT).dmp
	$(OBJCPY) -O ihex $(PROJECT).elf $(PROJECT).hex
	$(SZ) -d $(PROJECT).elf
	ls -l $(PROJECT).elf $(PROJECT).bin $(PROJECT).hex

$(PROJECT).elf: $(OBJECTS) $(LDSCRIPT)
	$(LD) $(LFLAGS) -o $(PROJECT).elf $(OBJECTS)

archive: $(COMBO).bin
	zip -r ../DLD/firmwares/dld-$(shell date +'%Y%m%d-%H%M%S').zip ../DLD/* -x ../DLD/firmwares/\* ../DLD/.\*

$(BUILDDIR)/%.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) -c $(CPPFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.c 
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))


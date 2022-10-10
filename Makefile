PROJECTNAME = bootloader

ifeq ($(TARGET),)
$(warning Specify TARGET=f446 or TARGET=f427. Assuming f446)
endif

TARGET ?= f446

ifeq ($(TARGET),f427)
STARTUP = startup_stm32f427.s
SYSTEM = system_stm32f4xx.c
LOADFILE = stm32f429.ld
DEVICE = stm32/device/f427
CORE = stm32/core
periphfiles = stm32f4xx_flash.c \
			stm32f4xx_gpio.c \
			stm32f4xx_rcc.c \
			stm32f4xx_tim.c \
			stm32f4xx_i2c.c \
			stm32f4xx_spi.c \
			stm32f4xx_dma.c 
PERIPHDIR = stm32/periph/stdperiph
PERIPH = $(addprefix  $(PERIPHDIR)/src/,$(periphfiles))
BUILDDIR = build/f427
LDSCRIPT = $(DEVICE)/stm32f429xx.ld
target_incs = -Isrc/f427
target_srcs = $(wildcard src/f427/*.c) src/f427/flash.cc
target_defs = -DUSE_STDPERIPH_DRIVER \
			  -DSTM32F427_437xx \
			  -DHSE_VALUE=16000000 \
			  -D'__FPU_PRESENT=1'  \
			  -DF_CPU=168000000L

else ifeq ($(TARGET),f446)
STARTUP = startup_stm32f446xx.s
SYSTEM = system_stm32f4xx.c
LOADFILE = STM32F446ZCHx_FLASH.ld
DEVICE = stm32/device/f446
CORE = stm32/core
PERIPHDIR = stm32/periph/STM32F4xx_HAL_Driver
PERIPH = $(wildcard $(PERIPHDIR)/src/*.c)
BUILDDIR = build/f446
LDSCRIPT = $(DEVICE)/STM32F446ZCHx_FLASH.ld
target_incs = -Isrc/f446
target_srcs = $(wildcard src/f446/*.c)
target_defs = -DSTM32F446xx -DUSE_HAL_DRIVER
endif

MAINAPP_DIR = ../DLD/
COMBO = ../DLD/build/$(TARGET)/combo
MAINAPP_HEX = ../DLD/build/$(TARGET)/main.hex

PROJECT = $(BUILDDIR)/$(PROJECTNAME)

SOURCES = 	$(DEVICE)/src/$(STARTUP) \
			$(DEVICE)/src/$(SYSTEM) \
			$(PERIPH) \
			$(target_srcs) \
			bootloader.cc \
			dig_inouts.c \
			bootloader_utils.cc \
			system_clock.cc \
			system.cc \
			misc.c \
			codec.c \
			i2s.c \
			encoding/fsk/packet_decoder.cc 

INCLUDES += -I$(DEVICE)/include \
			-I$(CORE)/include \
			-I$(PERIPHDIR)/include \
			-I$(PERIPHDIR)/Inc \
			$(target_incs)
		
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
DEPS = $(OBJECTS:.o=.d)


ARCHFLAGS = -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4 
ARCHFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 

FLAGS = -g2 -Os $(ARCHFLAGS)
FLAGS += -DARM_MATH_CM4 
FLAGS += -fsingle-precision-constant -Wdouble-promotion
FLAGS += -ffreestanding
FLAGS += -fcommon
FLAGS += -fdata-sections -ffunction-sections
FLAGS += -fno-exceptions -fno-unwind-tables
FLAGS += $(target_defs)
FLAGS += -I. $(INCLUDES)

CFLAGS = -std=c99
CFLAGS += $(FLAGS)

CXXFLAGS = -std=c++17
CXXFLAGS += -Wno-register
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics
CXXFLAGS += $(FLAGS)

AFLAGS  = $(ARCHFLAGS)
LFLAGS  = $(CFLAGS) -Wl,-Map=$(PROJECT).map -Wl,--gc-sections -T $(LDSCRIPT)

DEPFLAGS = -MMD -MP -MF $(BUILDDIR)/$(basename $<).d

# Executables

ARCH 	= arm-none-eabi
CC 		= $(ARCH)-gcc
CXX 	= $(ARCH)-g++
LD 		= $(ARCH)-g++
AS 		= $(ARCH)-as
OBJCPY 	= $(ARCH)-objcopy
OBJDMP 	= $(ARCH)-objdump
GDB 	= $(ARCH)-gdb
SZ 		= $(ARCH)-size
FLASH = st-flash

# Targets
#
all: Makefile $(PROJECT).bin

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
	$(OBJCPY) -O binary $(PROJECT).elf $(PROJECT).bin
	$(OBJDMP) -x --syms $(PROJECT).elf > $(PROJECT).dmp
	$(OBJCPY) -O ihex $(PROJECT).elf $(PROJECT).hex
	$(SZ) -d $(PROJECT).elf
	ls -l $(PROJECT).elf $(PROJECT).bin $(PROJECT).hex

$(PROJECT).elf: $(OBJECTS) $(LDSCRIPT)
	@echo "Linking..."
	@$(LD) $(LFLAGS) -o $@ $(OBJECTS)

archive: $(COMBO).bin
	zip -r ../DLD/firmwares/dld-$(shell date +'%Y%m%d-%H%M%S').zip ../DLD/* -x ../DLD/firmwares/\* ../DLD/.\*

$(BUILDDIR)/%.o: %.c $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(CC) -c $(DEPFLAGS) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cc $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(CXX) -c $(DEPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cpp $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(CXX) -c $(DEPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))

%.d: ;

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif

# $(BUILDDIR)/%.o: %.cc
# 	mkdir -p $(dir $@)
# 	$(CXX) -c $(CXXFLAGS) $< -o $@

# $(BUILDDIR)/%.o: %.c 
# 	mkdir -p $(dir $@)
# 	@echo "Compiling:" $<
# 	$(CC) -c $(CFLAGS) $< -o $@

# $(BUILDDIR)/%.o: %.s
# 	mkdir -p $(dir $@)
# 	$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))


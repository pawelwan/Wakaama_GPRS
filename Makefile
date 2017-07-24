# Define programs and commands
CCC	  = "C:\MinGW\bin\gcc"
TOOLCHAIN = arm-none-eabi
CC        = $(TOOLCHAIN)-gcc
OBJCOPY   = $(TOOLCHAIN)-objcopy
SIZE      = $(TOOLCHAIN)-size
OPENOCD   = openocd
STLINK    = "C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"

# Object files directory
# Warning: this will be removed by make clean!
OBJDIR = obj

# Target file name (without extension)
TARGET = $(OBJDIR)/STM32F4_Proj
CHIP = STM32F429_439xx

# Define all C source files (dependencies are generated automatically)
#

WAKAAMA_INC += -I wakaama
WAKAAMA_INC += -I wakaama/er-coap-13
WAKAAMA_INC += -I wakaama/platform
WAKAAMA_INC += -I wakaama/client
WAKAAMA_INC += -I wakaama/client/firmware
WAKAAMA_INC += -I wakaama/client/objects

WAKAAMA_SRC += wakaama/liblwm2m.c
WAKAAMA_SRC += wakaama/objects.c
WAKAAMA_SRC += wakaama/utils.c
WAKAAMA_SRC += wakaama/uri.c
WAKAAMA_SRC += wakaama/tlv.c
WAKAAMA_SRC += wakaama/data.c
WAKAAMA_SRC += wakaama/list.c
WAKAAMA_SRC += wakaama/packet.c
WAKAAMA_SRC += wakaama/transaction.c
WAKAAMA_SRC += wakaama/registration.c
WAKAAMA_SRC += wakaama/bootstrap.c
WAKAAMA_SRC += wakaama/management.c
WAKAAMA_SRC += wakaama/observe.c
WAKAAMA_SRC += wakaama/json.c
WAKAAMA_SRC += wakaama/discover.c
WAKAAMA_SRC += wakaama/block1.c
WAKAAMA_SRC += wakaama/er-coap-13/er-coap-13.c

WAKAAMA_SYMBOL += -DLWM2M_LITTLE_ENDIAN
WAKAAMA_SYMBOL += -DLWM2M_CLIENT_MODE
WAKAAMA_SYMBOL += -DLWM2M_WITH_LOGS  # LOGS
#WAKAAMA_SYMBOL += -DCOAPLOG # download logs

WAKAAMA_OBJECT_SRC += wakaama/client/objects/object_device.c
WAKAAMA_OBJECT_SRC += wakaama/client/objects/object_security.c
WAKAAMA_OBJECT_SRC += wakaama/client/objects/object_server.c
WAKAAMA_OBJECT_SRC += wakaama/client/objects/object_firmware.c
WAKAAMA_OBJECT_SRC += wakaama/client/objects/object_test.c

WAKAAMA_ADDON_SRC += wakaama/client/connection.c
WAKAAMA_ADDON_SRC += wakaama/platform/platform.c
WAKAAMA_ADDON_SRC += wakaama/platform/platformtime.c
WAKAAMA_ADDON_SRC += wakaama/client/firmware/firmware_update.c
WAKAAMA_ADDON_SRC += wakaama/client/firmware/crc32.c

CHECKSUM_SRC += wakaama/client/firmware/gen_header.c

SOURCES += main.c

SOURCES += drivers/board.c
SOURCES += drivers/dbgu.c drivers/term_io.c
SOURCES += drivers/usart3.c
SOURCES += drivers/usart.c
SOURCES += drivers/g510_frt.c
SOURCES += drivers/g510_socket.c
SOURCES += drivers/lis3dh.c
SOURCES += drivers/flash.c
SOURCES += drivers/sd/sd.c drivers/sd/stm32f4_sdio_sd.c

SOURCES += sys/ustime.c
SOURCES += sys/startup_stm32f4xx.s
SOURCES += sys/system_stm32f4xx.c
SOURCES += sys/syscalls.c
SOURCES += sys/vectors.c

SOURCES += fatfs/ff_io.c
SOURCES += fatfs/ff.c

SOURCES += FreeRTOS/Source/tasks.c
SOURCES += FreeRTOS/Source/queue.c
SOURCES += FreeRTOS/Source/list.c
SOURCES += FreeRTOS/Source/croutine.c
SOURCES += FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/misc.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c
SOURCES += libs/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c

SOURCES += $(WAKAAMA_SRC)
SOURCES += $(WAKAAMA_OBJECT_SRC)
SOURCES += $(WAKAAMA_ADDON_SRC)

OBJECTS  = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(SOURCES))))

# Place -D, -U or -I options here for C and C++ sources
CPPFLAGS += -I.
CPPFLAGS += -Isys
CPPFLAGS += -Idrivers
CPPFLAGS += -IFreeRTOS/Source/include
CPPFLAGS += -IFreeRTOS/Source/portable/GCC/ARM_CM4F
CPPFLAGS += -Ilibs/CMSIS/Include
CPPFLAGS += -Ilibs/Device/STM32F4xx/Include
CPPFLAGS += -Ilibs/STM32F4xx_StdPeriph_Driver/inc
CPPFLAGS += -D$(CHIP)
CPPFLAGS += -DFREERTOS
CPPFLAGS += $(WAKAAMA_INC)
CPPFLAGS += $(WAKAAMA_SYMBOL)

#---------------- Compiler Options C ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS  = -std=gnu99
CFLAGS += -gdwarf-2
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -Wall
#CFLAGS += -Wextra
#CFLAGS += -Wpointer-arith
#CFLAGS += -Wstrict-prototypes
#CFLAGS += -Winline
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wundef
CFLAGS += -Wa,-adhlns=$(OBJDIR)/$(*F).lst

# Optimize use of the single-precision FPU
#
CFLAGS += -fsingle-precision-constant

#---------------- Assembler Options ----------------
#  -Wa,...:   tell GCC to pass this to the assembler
#  -adhlns:   create listing
#
ASFLAGS = -Wa,-adhlns=$(OBJDIR)/$(*F).lst


#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS += -lm
LDFLAGS += -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Tsys/stm32_flash.ld

#============================================================================

# Compiler flags to generate dependency files
GENDEPFLAGS = -MMD -MP -MF $(OBJDIR)/$(*F).d


# Combine all necessary flags and optional flags
# Add target processor to flags.
#
CPU = -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16

CFLAGS   += $(CPU)
ASFLAGS  += $(CPU)
LDFLAGS  += $(CPU)

# Default target.
all:  gccversion build checksum showsize

build: elf hex bin

elf: $(TARGET).elf
hex: $(TARGET).hex
bin: $(TARGET).bin

# Display compiler version information
gccversion:
	@$(CC) --version
	
# generate file for checksum
gen_header: $(CHECKSUM_SRC)
	@echo
	@echo compiling checksum programs
	$(CCC) -std=gnu99 -c $(CHECKSUM_SRC) -o gen_header.o
	$(CCC) -std=gnu99 gen_header.o -o gen_header

# calc checksum and add header to binfile
checksum: gen_header
	@echo
	@echo generating header: $<
	gen_header $(TARGET).bin 0x0 0xabcd1234 1
#gen_header $(TARGET).bin 0x1 0xdeadbeef 0 # create update
	@echo adding header: $<
	srec_cat ${CURDIR}/$(OBJDIR)/firmware_header.bin -binary ${CURDIR}/$(TARGET).bin -binary -offset 0x18 -o ${CURDIR}/$(OBJDIR)/firmware.bin -binary

# Show the final program size
showsize: elf
	@echo
	@$(SIZE) $(TARGET).elf 2>/dev/null

# Flash the device
program: hex
	$(STLINK) -c SWD -ME -P $(TARGET).hex -Rst

program_linux: elf
	$(OPENOCD) -f /home/john/openocd-0.9.0/tcl/board/stm32f4discovery.cfg -c "program $(TARGET).elf verify reset exit"

# Target: clean project
clean:
	@echo Cleaning project:
	rm -rf $(OBJDIR)

# Link: create ELF output file from object files
.SECONDARY: $(TARGET).elf
.PRECIOUS:  $(OBJECTS)
$(TARGET).elf: $(OBJECTS)
	@echo
	@echo Linking: $@
	$(CC) $^ $(LDFLAGS) --output $@

# Create final output files (.hex, .bin) from ELF output file.
%.hex: %.elf
	@echo
	@echo Creating hex file: $@
	$(OBJCOPY) -O ihex $< $@

%.bin : %.elf
	$(OBJCOPY) -O binary $< $@

# Compile: create object files from C source files
$(OBJDIR)/%.o : %.c
	@echo
	@echo Compiling C: $<
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(GENDEPFLAGS) $< -o $@

# Assemble: create object files from assembler source files
$(OBJDIR)/%.o : %.s
	@echo
	@echo Assembling: $<
	$(CC) -c $(CPPFLAGS) $(ASFLAGS) $< -o $@

# Create object file directories
$(shell mkdir -p $(OBJDIR) 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/sys 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/drivers 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/drivers/sd 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/fatfs 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/FreeRTOS/Source/ 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/FreeRTOS/Source/portable/GCC/ARM_CM4F 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/libs/STM32F4xx_StdPeriph_Driver/src 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/wakaama/er-coap-13 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/wakaama/platform 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/wakaama/client/firmware 2>/dev/null)
$(shell mkdir -p $(OBJDIR)/wakaama/client/objects 2>/dev/null)

# Include the dependency files
-include $(wildcard $(OBJDIR)/*.d)

# Listing of phony targets
.PHONY: all build clean elf \
        showsize gccversion

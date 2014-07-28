CC = arm-none-eabi-gcc
LINK = arm-none-eabi-gcc
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
CTRULIB = ../libctru
AEMSTROPATH = ../../aemstro
CFLAGS += -Wall -std=c99 -march=armv6 -O3 -I"$(CTRULIB)/include" -I$(DEVKITPRO)/libnds/include
# LDFLAGS += --script=ccd00.ld -L"$(DEVKITARM)/arm-none-eabi/lib" -L"$(DEVKITARM)/lib/gcc/arm-none-eabi/4.7.1" -L"$(CTRULIB)/lib"
# LDFLAGS += --script=ccd00.ld -L"$(DEVKITARM)/arm-none-eabi/lib" -L"$(CTRULIB)/lib"
LDFLAGS += -nostartfiles --specs=ccd00.specs -L"$(DEVKITARM)/arm-none-eabi/lib" -L"$(CTRULIB)/lib"

CFILES = $(wildcard source/*.c)
VSHFILES = $(wildcard source/*.vsh)
SHBINFILES = $(VSHFILES:source/%.vsh=data/%.shbin)
BINFILES = $(wildcard data/*.bin)
OFILES = $(BINFILES:data/%.bin=build/%.bin.o)
OFILES += $(SHBINFILES:data/%.shbin=build/%.shbin.o)
OFILES += $(CFILES:source/%.c=build/%.o)
DFILES = $(CFILES:source/%.c=build/%.d)
SFILES = $(wildcard source/*.s)
OFILES += $(SFILES:source/%.s=build/%.o)
PROJECTNAME = ${shell basename "$(CURDIR)"}

#---------------------------------------------------------------------------------
# canned command sequence for binary data, taken from devkitARM
#---------------------------------------------------------------------------------
define bin2o
	bin2s $< | $(AS) -o $(@)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > source/`(echo $(<F) | tr . _)`.h
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> source/`(echo $(<F) | tr . _)`.h
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> source/`(echo $(<F) | tr . _)`.h
endef

.PHONY:=all dir

all: dir $(PROJECTNAME).bin

dir:
	@mkdir -p build
	@mkdir -p data

$(PROJECTNAME).bin: $(PROJECTNAME).elf
	$(OBJCOPY) -O binary $< $@

$(PROJECTNAME).elf: $(SHBINFILES) $(OFILES)
	# $(LINK) $(LDFLAGS) -o $(PROJECTNAME).elf $(filter-out build/crt0.o, $(OFILES)) -lctru -lc -lgcc
	$(LINK) $(LDFLAGS) -o $(PROJECTNAME).elf $(filter-out build/crt0.o, $(OFILES)) -g -lctru -lm

clean:
	@rm -f build/*.o build/*.d
	@rm -f $(PROJECTNAME).elf $(PROJECTNAME).bin
	@echo "all cleaned up !"

-include $(DFILES)

data/%.shbin: source/%.vsh
	@python $(AEMSTROPATH)/aemstro_as.py $< $@

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $< > build/$*.d

build/%.o: source/%.s
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $< > build/$*.d

build/%.shbin.o: data/%.shbin
	@echo $(notdir $<)
	@$(bin2o)

build/%.bin.o: data/%.bin
	@echo $(notdir $<)
	@$(bin2o)

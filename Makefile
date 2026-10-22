# ------------------------------------------------------------------------------
# 3DS Hangman Game Makefile
# ------------------------------------------------------------------------------

TARGET      := hangman-game
BUILD       := build
SOURCES     := source
INCLUDES    := include
DATA        := data

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment.")
endif

CC          := arm-none-eabi-gcc
CXX         := arm-none-eabi-g++
LD          := arm-none-eabi-g++

include $(DEVKITARM)/3ds_rules

ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard 
CFLAGS      := -g -Wall -O2 -mword-relocations $(ARCH)
CFLAGS      += $(INCLUDE) -D__3DS__
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g $(ARCH)

LDFLAGS     := -specs=3dsx.specs -g $(ARCH)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   -I$(CURDIR)/$(BUILD) \
                   -I$(PORTLIBS)/include \
                   -I$(CTRULIB)/include

export LIBPATHS := -L$(PORTLIBS)/lib -L$(CTRULIB)/lib
export LIBS     := -lcitro2d -lcitro3d -lctru -lm

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo "Cleaning project..."
	@rm -rf $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf

else

$(OUTPUT).3dsx  : $(OUTPUT).elf
$(OUTPUT).elf   : $(OFILES)

-include $(DEPSDIR)/*.d

endif

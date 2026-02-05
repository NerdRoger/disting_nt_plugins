ifndef FINAL_OUTPUT
    $(error FINAL_OUTPUT is not defined. Please define it before including buildPlugin.mk)
endif

ifndef NT_API_PATH
    NT_API_PATH := ../distingNT_API
endif

INCLUDE_PATHS := -I$(NT_API_PATH)/include -I./include -I../Common/include

# Base Flags
CPP_FLAGS := -std=c++23 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-exceptions -fno-rtti -Os -fPIC -Wall
CPP_FLAGS_STATS := $(CPP_FLAGS) -g

# Reusable Stats Display Logic
define STATS_DISPLAY_LOGIC
    @echo "=========================================================="
    @echo "  BINARY STATISTICS (Decimal) for $(notdir $(STATS_OUTPUT))"
    @echo "=========================================================="
    @arm-none-eabi-size -A $(STATS_OUTPUT) | grep -E '\.text|\.rodata|\.data|\.bss'
    @echo "----------------------------------------------------------"
    @echo "Total Symbols: $$(arm-none-eabi-nm $(STATS_OUTPUT) | wc -l)"
    @echo "  Local (t): $$(arm-none-eabi-nm $(STATS_OUTPUT) | grep -c ' t ')"
    @echo "  Global (T): $$(arm-none-eabi-nm $(STATS_OUTPUT) | grep -c ' T ')"
    @echo "----------------------------------------------------------"
    @echo "GOT Relocations with Source Lines:"
    @arm-none-eabi-readelf -r $(STATS_OUTPUT) | grep GOT | \
    while read -r line; do \
        addr=$$(echo $$line | awk '{ print $$1 }'); \
        src=$$(arm-none-eabi-addr2line -e $(STATS_OUTPUT) 0x$$addr); \
        printf "%s  [%s]\n" "$$line" "$$src"; \
    done
    @echo "=========================================================="
endef

CPP_FILES := $(wildcard *.cpp ../Common/*.cpp)
STATS_OUTPUT := $(FINAL_OUTPUT:.o=.stats.o)
UNITY_BUNDLE := .unity_master.cpp
.INTERMEDIATE: $(UNITY_BUNDLE) $(STATS_OUTPUT)

debug: CPP_FLAGS += -DDEBUG

# --- Standard Targets ---

all: $(FINAL_OUTPUT) $(STATS_OUTPUT)

debug: $(FINAL_OUTPUT)

clean:
	rm -f $(FINAL_OUTPUT) $(UNITY_BUNDLE) $(STATS_OUTPUT)

$(UNITY_BUNDLE): $(CPP_FILES)
	@echo "Generating master bundle: $@"
	@echo "// Auto-generated Unity Build" > $@
	@$(foreach file,$(CPP_FILES),echo '#include "$(abspath $(file))"' >> $@;)

# build the actual final output of the plugin
$(FINAL_OUTPUT): $(UNITY_BUNDLE)
	@echo "Starting Build..."
	@mkdir -p $(dir $(FINAL_OUTPUT))
	arm-none-eabi-c++ $(CPP_FLAGS) $(INCLUDE_PATHS) -c -o $(FINAL_OUTPUT) $(UNITY_BUNDLE)
	@echo "Final Output Build Complete: $(FINAL_OUTPUT)"

# build a temporary version of the plugin with degub symbols enabled and report stats and line numbers
$(STATS_OUTPUT): $(UNITY_BUNDLE)
	@mkdir -p $(dir $(STATS_OUTPUT))
	arm-none-eabi-c++ $(CPP_FLAGS_STATS) $(INCLUDE_PATHS) -c -o $(STATS_OUTPUT) $(UNITY_BUNDLE)
	@echo "Stats Output Build Complete: $(STATS_OUTPUT)"
	@$(STATS_DISPLAY_LOGIC)

.PHONY: all clean debug
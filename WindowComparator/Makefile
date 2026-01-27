ifndef NT_API_PATH
    NT_API_PATH := ../distingNT_API
endif

INCLUDE_PATHS := -I$(NT_API_PATH)/include -I./include -I../Common/include

# Base Flags
CPP_FLAGS := -std=c++23 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-exceptions -fno-rtti -Os -fPIC -Wall
CPP_FLAGS_DEBUG := $(CPP_FLAGS) -DDEBUG
CPP_FLAGS_STATS := $(CPP_FLAGS) -g

# --- Multi-Target Logic ---
IS_DEBUG := $(filter debug,$(MAKECMDGOALS))
IS_STATS := $(filter stats,$(MAKECMDGOALS))

ifneq ($(IS_DEBUG),)
    CPP_FLAGS := $(CPP_FLAGS_DEBUG)
endif

ifneq ($(IS_STATS),)
    # Stats requires -g for addr2line to work
    CPP_FLAGS := $(CPP_FLAGS_STATS) $(if $(IS_DEBUG),-DDEBUG)
endif

# Reusable Stats Display Logic
define STATS_DISPLAY_LOGIC
	@echo "=========================================================="
	@echo "  BINARY STATISTICS (Decimal) for $(notdir $(FINAL_OUTPUT))"
	@echo "=========================================================="
	@# Removed -x to ensure decimal output
	@arm-none-eabi-size -A $(FINAL_OUTPUT) | grep -E '\.text|\.rodata|\.data|\.bss'
	@echo "----------------------------------------------------------"
	@echo "Total Symbols: $$(arm-none-eabi-nm $(FINAL_OUTPUT) | wc -l)"
	@echo "  Local (t): $$(arm-none-eabi-nm $(FINAL_OUTPUT) | grep -c ' t ')"
	@echo "  Global (T): $$(arm-none-eabi-nm $(FINAL_OUTPUT) | grep -c ' T ')"
	@echo "----------------------------------------------------------"
	@echo "GOT Relocations with Source Lines:"
	@arm-none-eabi-readelf -r $(FINAL_OUTPUT) | grep GOT | \
	while read -r line; do \
		addr=$$(echo $$line | awk '{ print $$1 }'); \
		src=$$(arm-none-eabi-addr2line -e $(FINAL_OUTPUT) 0x$$addr); \
		printf "%s  [%s]\n" "$$line" "$$src"; \
	done
	@echo "=========================================================="
endef

CPP_FILES := $(wildcard *.cpp ../Common/*.cpp)
OBJ_FILES := $(patsubst %.cpp,%.o,$(CPP_FILES))
FINAL_OUTPUT := ../plugins/windowComparator.o

# --- Standard Targets ---

multi: $(FINAL_OUTPUT)

%.o: %.cpp
	arm-none-eabi-c++ $(CPP_FLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(FINAL_OUTPUT): $(OBJ_FILES)
	@mkdir -p $(@D)
	arm-none-eabi-ld -r -o $@ $(OBJ_FILES)
	@rm -f $(OBJ_FILES)
	@$(if $(IS_STATS),$(STATS_DISPLAY_LOGIC))

clean:
	rm -f $(OBJ_FILES) $(FINAL_OUTPUT) .unity_master.cpp

debug stats:
	@:

# --- Unity Build Extension ---

UNITY_BUNDLE := .unity_master.cpp
.INTERMEDIATE: $(UNITY_BUNDLE)

$(UNITY_BUNDLE): $(CPP_FILES)
	@echo "Generating master bundle: $@"
	@echo "// Auto-generated Unity Build" > $@
	@$(foreach file,$(CPP_FILES),echo '#include "$(abspath $(file))"' >> $@;)

all: $(UNITY_BUNDLE)
	@echo "Starting Unity Build (Flags: $(if $(IS_DEBUG),DEBUG) $(if $(IS_STATS),STATS))..."
	@mkdir -p $(dir $(FINAL_OUTPUT))
	arm-none-eabi-c++ $(CPP_FLAGS) $(INCLUDE_PATHS) -c -o $(FINAL_OUTPUT) $(UNITY_BUNDLE)
	@echo "Unity Build Complete: $(FINAL_OUTPUT)"
	@$(if $(IS_STATS),$(STATS_DISPLAY_LOGIC))

.PHONY: all clean debug stats multi
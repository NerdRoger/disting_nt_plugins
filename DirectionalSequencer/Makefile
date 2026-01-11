ifndef NT_API_PATH
	NT_API_PATH := ../distingNT_API
endif

INCLUDE_PATHS := -I$(NT_API_PATH)/include -I./include -I../Common/include

CPP_FLAGS := -std=c++23 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-exceptions -fno-rtti -Os -fPIC -Wall
CPP_FLAGS_DEBUG := $(CPP_FLAGS) -DDEBUG
CPP_FLAGS_GOT := $(CPP_FLAGS) -g

CPP_FILES := $(wildcard *.cpp ../Common/*.cpp)
OBJ_FILES := $(patsubst %.cpp,%.o,$(CPP_FILES))

FINAL_OUTPUT := ../plugins/directionalSequencer.o

all: $(FINAL_OUTPUT)

%.o: %.cpp
	arm-none-eabi-c++ $(CPP_FLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(FINAL_OUTPUT): $(OBJ_FILES)
	mkdir -p $(@D)
	arm-none-eabi-ld -r -o $@ $(OBJ_FILES)
	rm -f $(OBJ_FILES)

clean:
	rm -f $(OBJ_FILES) $(FINAL_OUTPUT)

debug: CPP_FLAGS := $(CPP_FLAGS_DEBUG)
debug: $(FINAL_OUTPUT)

got: CPP_FLAGS := $(CPP_FLAGS_GOT)
got: $(FINAL_OUTPUT)
	@echo "GOT Relocations with Source Lines:"
	@arm-none-eabi-readelf -r $(FINAL_OUTPUT) | grep GOT | \
	while read -r line; do \
		addr=$$(echo $$line | awk '{ print $$1 }'); \
		src=$$(arm-none-eabi-addr2line -e $(FINAL_OUTPUT) 0x$$addr); \
		printf "%s  [%s]\n" "$$line" "$$src"; \
	done

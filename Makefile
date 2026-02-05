PLUGIN_MAKEFILES := $(wildcard */Makefile)
PLUGINS := $(patsubst %/Makefile,%,$(PLUGIN_MAKEFILES))

all clean debug: $(PLUGINS)

list:
	@echo "Discovered plugins: $(PLUGINS)"

$(PLUGINS):
	@$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS)

.PHONY: all clean debug	list $(PLUGINS)
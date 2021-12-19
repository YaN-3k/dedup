include config.mk

src := $(shell find $(src_dir) -name '*.c')
obj := $(src:$(src_dir)/%.c=$(bin_dir)/%.o)
dep := $(obj:.o=.d)

all: options $(bin)

options:
	@echo "# CFLAGS   = ${CFLAGS}"
	@echo "# LDFLAGS  = ${LDFLAGS}"
	@echo "# CC       = ${CC}"

$(bin_dir)/%.o: $(src_dir)/%.c
	@echo $(cmd_cc)
	$(Q)mkdir -p $(dir $@)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(obj): config.mk

$(bin): $(obj)
	@echo $(cmd_ld)
	@$(CC) -o $@ $(obj) $(LDFLAGS)

clean:
	rm -f $(obj) ${dep} $(bin)

-include $(dep)

.PHONY: all options clean

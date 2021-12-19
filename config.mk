version := 1.0

bin_dir := bin
src_dir := src

bin := dedup

CPPFLAGS := -D_DEFAULT_SOURCE -DVERSION=\"$(version)\" -MMD -MP
CFLAGS   := -std=c99 -pedantic -Wall $(incs) $(CPPFLAGS)
LDFLAGS  := -lcrypto -lssl

cflags_debug   := -ggdb
cflags_size    := -Os
cflags_release := -O3

quiet_cmd_cc    = CC      $@
quiet_cmd_ld    = LD      $@
verbose_cmd_cc  = $(CC) -c -o $@ $(CFLAGS) $<
verbose_cmd_ld  = $(CC) -c -o $@ $(LDFLAGS) $^

ifndef build
	build := release
endif

CFLAGS += $(cflags_$(build))

ifndef V
	quiet := quiet
	Q := @
else
	quiet := verbose
	Q :=
endif

cmd_cc = $($(quiet)_cmd_cc)
cmd_ld = $($(quiet)_cmd_ld)

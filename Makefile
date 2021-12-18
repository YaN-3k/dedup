CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -Wall -Wextra -pedantic -std=c11 -ggdb ${CPPFLAGS}
LDFLAGS =

src = main.c recdir.c util.c
obj = $(src:.c=.o)
bin = dirs

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(bin) $(obj)

.PHONY: clean

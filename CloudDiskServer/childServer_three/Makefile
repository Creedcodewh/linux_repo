Srcs := $(wildcard *.c)
Outs := $(patsubst %.c, %.o, $(Srcs))

CC := gcc
CFLAGS := -Wall -g -pthread -lmysqlclient  -ll8w8jwt  -lmbedcrypto -lmbedx509  -lssl 

foo := server
ifdef foo
	foo := $(foo)
endif

$(foo): $(Outs)
	$(CC) $^ -o $@ $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean rebuild

clean:
	$(RM) $(Outs) $(foo)

rebuild: clean $(foo)

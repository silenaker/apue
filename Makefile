PROGS_SRC=$(shell find chapters -name "*.c")
LIB_SRC=$(shell find lib -name "*.c")
PROGS:=$(PROGS_SRC:%.c=%)
LIBS:=$(LIB_SRC:.c=.o)

CFLAGS=-O0 -g -Ilib -Ilibfort/lib
LINK_COMM=cc -o $@ $^

.PHONY: all clean

all: $(PROGS)

clean:
	rm -rf **/*.dSYM $(PROGS) $(PROGS:%=%.o) $(LIBS) libfort/lib/fort.o

$(PROGS): %: %.o $(LIBS) libfort/lib/fort.o
	$(LINK_COMM)

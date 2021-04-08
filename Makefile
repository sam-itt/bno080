#TOP_SRCDIR=..
#SRCDIR=$(TOP_SRCDIR)/src
SRCDIR=.

CC=gcc
CFLAGS=-g3 -O0 -I$(SRCDIR)
LDFLAGS=
EXEC=test-bno
SRC= $(wildcard $(SRCDIR)/*.c)
OBJ= $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)


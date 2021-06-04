CC      = gcc
CFLAGS  = -g
RM      = rm -f


default: all

all: pair_builder

pair_builder: pair_builder.c
	$(CC) $(CFLAGS) -o pair_builder pair_builder.c

clean veryclean:
	$(RM) pair_builder
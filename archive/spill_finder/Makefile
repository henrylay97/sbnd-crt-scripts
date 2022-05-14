CC      = g++
CFLAGS  = -g
RM      = rm -f


default: all

all: spill_extractor

spill_extractor: spill_extractor.C
	$(CC) $(CFLAGS) -o spill_extractor spill_extractor.C

clean veryclean:
	$(RM) spill_extractor
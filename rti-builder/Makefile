BUILDDIR = bin
PTMDIR = test-ptms
IMGDIR = test

CC = gcc -O

CFLAGS = -std=c11 -Wall -Wextra -fopenmp
LDFLAGS = -g
LIBS = -lm -ljpeg -lblas -llapacke -lgomp

.PHONY: all test-images test-exploder

all: $(BUILDDIR)/ptm-decoder $(BUILDDIR)/ptm-encoder $(BUILDDIR)/ptm-exploder

ptmlib.o : ptmlib.c ptmlib.h

ptm-decoder.o : ptm-decoder.c ptmlib.h

ptm-encoder.o : ptm-encoder.c ptmlib.h

ptm-exploder.o : ptm-exploder.c ptmlib.h

$(BUILDDIR)/ptm-decoder: ptm-decoder.o ptmlib.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILDDIR)/ptm-encoder: ptm-encoder.o ptmlib.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILDDIR)/ptm-exploder: ptm-exploder.o ptmlib.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

PTMS        := $(wildcard $(PTMDIR)/*.ptm)
JPEGS       := $(patsubst $(PTMDIR)/%.ptm, $(IMGDIR)/%.jpeg, $(PTMS))

test-images: $(JPEGS)

$(IMGDIR)/%.jpeg : $(PTMDIR)/%.ptm $(BUILDDIR)/ptm-decoder
	-$(BUILDDIR)/ptm-decoder $< 0.5 0.5 > $@

test-exploder: $(BUILDDIR)/ptm-exploder
	$(BUILDDIR)/ptm-exploder $(PTMDIR)/shell6.ptm $(PTMDIR)/sample.lp $(IMGDIR)/shell6.jpg

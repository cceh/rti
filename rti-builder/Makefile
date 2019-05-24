BUILDDIR = build
BINDIR = ../bin
PTMDIR = test-ptms
IMGDIR = test
CC = gcc -O

CFLAGS = -std=c11 -Wall -Wextra -fopenmp
LDFLAGS = -g
LIBS = -lm -ljpeg -lblas -llapacke -lgomp

.PHONY: all test-images test-exploder

all: $(BINDIR)/ptm-decoder $(BINDIR)/ptm-encoder $(BINDIR)/ptm-exploder

VPATH = o:$(BUILDDIR)

$(BUILDDIR)/%.o : %.c
	@mkdir -p $(BUILDDIR)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BUILDDIR)/ptmlib.o : ptmlib.c ptmlib.h

$(BUILDDIR)/ptm-decoder.o : ptm-decoder.c ptmlib.h

$(BUILDDIR)/ptm-encoder.o : ptm-encoder.c ptmlib.h

$(BUILDDIR)/ptm-exploder.o : ptm-exploder.c ptmlib.h

$(BINDIR)/ptm-decoder: ptm-decoder.o ptmlib.o
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BINDIR)/ptm-encoder: ptm-encoder.o ptmlib.o
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BINDIR)/ptm-exploder: ptm-exploder.o ptmlib.o
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

PTMS        := $(wildcard $(PTMDIR)/*.ptm)
JPEGS       := $(patsubst $(PTMDIR)/%.ptm, $(IMGDIR)/%.jpeg, $(PTMS))

test-images: $(JPEGS)

$(IMGDIR)/%.jpeg : $(PTMDIR)/%.ptm $(BINDIR)/ptm-decoder
	-$(BINDIR)/ptm-decoder $< 0.5 0.5 > $@

test-exploder: $(BINDIR)/ptm-exploder
	$(BINDIR)/ptm-exploder $(PTMDIR)/shell6.ptm $(PTMDIR)/sample.lp $(IMGDIR)/shell6.jpg

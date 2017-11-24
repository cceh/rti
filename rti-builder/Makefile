CC = gcc -O

CFLAGS =
LDFLAGS = -g
LIBS = -ljpeg

BUILDDIR = bin
PTMDIR = ../../Downloads
IMGDIR = test

.PHONY: test-images

all: $(BUILDDIR)/ptm-decoder

$(BUILDDIR)/ptm-decoder: ptm-decoder.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS) ptm-decoder.c

PTMS        := $(wildcard $(PTMDIR)/*.ptm)
JPEGS       := $(patsubst $(PTMDIR)/%.ptm, $(IMGDIR)/%.jpeg, $(PTMS))

test-images: $(JPEGS)

$(IMGDIR)/%.jpeg : $(PTMDIR)/%.ptm $(BUILDDIR)/ptm-decoder
	-$(BUILDDIR)/ptm-decoder $< 0.5 0.5 > $@

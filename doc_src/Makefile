# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = python3 -msphinx
SPHINXPROJ    = RTI
SOURCEDIR     = .
BUILDDIR      = ../docs

.PHONY: html clean Makefile

html: Makefile
	for i in light*.png; do convert "$$i" -trim "$$i"; done
	$(SPHINXBUILD) -b html $(SPHINXOPTS) $(SOURCEDIR) $(BUILDDIR)
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)."

clean:
	rm -rf $(BUILDDIR)/*

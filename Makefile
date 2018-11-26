# build the sphinx docs (and github pages)
.PHONY: docs

docs:
	cd doc_src ; make html ; cd ..

builder:
	cd rti-builder; make ; cd ..

# build the sphinx docs (and github pages)
.PHONY: docs

LIGHT_PICS = $(patsubst %,doc_src/light-%.png,2d 3d fit samples-fit samples)

$(LIGHT_PICS) : doc_src/draw_light_illustrations.m
	cd doc_src ; octave draw_light_illustrations.m ; cd ..
	for i in $(LIGHT_PICS); do convert "$$i" -trim "$$i"; done

docs: $(LIGHT_PICS)
	cd doc_src ; make html ; cd ..

builder:
	cd rti-builder; make ; cd ..

view:
	wine64 ~/.wine/drive_c/Program\ Files/RTIViewer/RTIViewer.exe

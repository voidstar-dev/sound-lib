sound_lib.html: sound_lib.md generate_documentation.py ../src/sound_lib.h
	python generate_documentation.py

install: sound_lib.html
	cp $< $(INSTALL_DIR)

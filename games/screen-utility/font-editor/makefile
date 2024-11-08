CODE_NAME = font_editor
SOURCES = screen.h textHandler.h menu.h
SOURCE_DEFINITIONS = menu.cpp textHandler.cpp screen.cpp
XLIB_TAGS = -lX11 -pthread

all: $(SOURCES)
	g++ main.cpp -o $(CODE_NAME) $(SOURCE_DEFINITIONS) $(XLIB_TAGS)

nrun: all
	./$(CODE_NAME)
GAME_NAME = rhythm
SOURCES = note.h screen.h textHandler.h menu.h
SOURCE_DEFINITIONS = textHandler.cpp menu.cpp note.cpp screen.cpp
XLIB_TAGS = -lX11 -pthread

all: $(SOURCES)
	g++ main.cpp -o $(GAME_NAME) $(SOURCE_DEFINITIONS) $(XLIB_TAGS)

nrun: all
	./$(GAME_NAME)
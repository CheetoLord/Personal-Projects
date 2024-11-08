GAME_NAME = CYOA
SOURCES =  screen.h textHandler.h adventure.h
SOURCE_DEFINITIONS = textHandler.cpp screen.cpp
XLIB_TAGS = -lX11 -pthread

all: $(SOURCES)
	g++ main.cpp -o $(GAME_NAME) $(SOURCE_DEFINITIONS) $(XLIB_TAGS)

nrun: all
	./$(GAME_NAME)

ndebug: all
	./$(GAME_NAME) debug

nnocrash: all
	./$(GAME_NAME) nocrash
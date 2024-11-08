PGRM_NAME := soundTest
SOURCES := soundHandler.cpp
INCLUDES := soundHandler.h
SOUND_TAGS := -lasound

all: $(INCLUDES)
	g++ main.cpp -o $(PGRM_NAME) $(SOURCES) $(SOUND_TAGS)

nrun: all
	./$(PGRM_NAME)

chatGPT:
	g++ chatGPTtry.cpp -o $(PGRM_NAME) $(SOUND_TAGS)
	./soundTest

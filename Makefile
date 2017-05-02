CC=g++
#DEBUG
#CFLAGS= -std=c++11 -c -Wall -g
#RELEASE
CFLAGS= -std=c++11 -c -Wall -g -DNDEBUG
LDFLAGS=

SOURCES=src/defines.cpp src/memory.cpp src/fu.cpp src/scoreboard.cpp src/pipeline.cpp src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: simulator clean

simulator: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f src/*.o *~ simulator

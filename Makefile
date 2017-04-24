CC=g++
CFLAGS= -std=c++11 -c -Wall -g
LDFLAGS=

SOURCES=src/defines.cpp src/memory.cpp src/fu.cpp src/scoreboard.cpp src/pipeline.cpp src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: pipeline clean

pipeline: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f src/*.o *~ pipeline

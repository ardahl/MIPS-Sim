CC=g++
CFLAGS= -std=c++11 -c -Wall -g
LDFLAGS=

SOURCES=defines.cpp fu.cpp scoreboard.cpp pipeline.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: pipeline clean

pipeline: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~ pipeline

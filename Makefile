CC=g++
CFLAGS=-std=c++11
LDFLAGS=
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Oving3

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -o $@ $^ $(LDFLAGS) 

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o
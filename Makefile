
# Makefile for htwlib.c
CXX = gcc
CXXFLAGS = -Wall

FILE = main

TARGET = $(FILE).out

.PHONY: clean

$(TARGET): $(FILE).c htwlib.c user-htwlib.c
	$(CC) $(CXXFLAGS) $(FILE).c -o $(TARGET) 	

clean: $(TARGET)
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

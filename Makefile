# Makefile
CC = gcc
CFLAGS = -Wall
LDFLAGS = -ldl

TARGET = dynaspy	
SOURCES = dynaspy.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

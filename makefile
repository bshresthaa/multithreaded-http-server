CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

TARGET = server
SOURCES = main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) serverheader.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
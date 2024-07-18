# Define the compiler
CXX = g++

# Define compiler flags
CXXFLAGS = -Wall -std=c++11

# Define the target executable
TARGET = VirtualMemory

# Define source files
SRCS = VirtualMemory.cpp

# Default target to build the executable
all: $(TARGET)

# Rule to link object files and create the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up generated files
clean:
	rm -f $(TARGET)

# Run the program with the addresses.txt file
run: $(TARGET)
	./$(TARGET) addresses.txt

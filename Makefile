# Makefile for BFS traversal program

# Compiler and flags
CXX = g++
CXXFLAGS = -I ~/body/rapidjson/include -lcurl

# Target executable name
TARGET = bfs

# Source files
SRCS = bfs.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target to build the executable
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(CXXFLAGS)

# Rule to compile C++ source files into object files
%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS)

# Rule to clean object files and executable
clean:
	rm -f $(OBJS) $(TARGET)


# This make file will basically enable us to just speed up the compile process and just type 'make' to compile. 

# This is the compiler we'll use — g++ is the standard for C++
CXX = g++


# Show all warnings (so we catch bugs early)
CXXFLAGS = -std=c++17 -Wall -Iinclude

# These are the SFML libraries we need to link: graphics for drawing stuff, window for handling the game window, system for timing and other utilities
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Grab all .cpp files from the src folder
SRC = $(wildcard src/*.cpp)

# Convert those .cpp files into .o object files
OBJ = $(SRC:.cpp=.o)

# This is the name of the final executable we'll build
TARGET = game

# The default rule: build the game
all: $(TARGET)

# Link all object files together to create the final game executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

# Makefile for Linux terminal application

CXX          := g++
CXXFLAGS     := -Wall -Wextra -pedantic -g -c -x c++ -std=c++11
INC          :=
LDLIBS       := -lncurses
LDFLAGS      :=
SRCFILES_CPP := $(shell find . -maxdepth 2 -name "*.cpp")
SRCFILES_INO := $(shell find . -maxdepth 2 -name "*.ino")
SRCFILES     := $(SRCFILES_CPP) $(SRCFILES_INO)
OBJECTS      := $(OBJECTS) $(patsubst %.cpp, %.o, $(SRCFILES_CPP))
OBJECTS      := $(OBJECTS) $(patsubst %.ino, %.o, $(SRCFILES_INO))
EXECUTABLE   := PixelShirtV2

all: $(SRCFILES) $(EXECUTABLE)

clean:
	-rm -f $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS) $(LDLIBS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< -o $@
	
%.o: %.ino
	$(CXX) $(CXXFLAGS) $(INC) $< -o $@

# To print a variable to the terminal:
# make print-VARIABLE
print-%  : ; @echo $* = $($*)

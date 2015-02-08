CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -std=c++0x -fopenmp
CC_FLAGS := -std=c++0x -MMD  -fopenmp

clear:
	rm main debug obj/*

all: debug main

debug: LD_FLAGS += -ggdb
debug: CC_FLAGS += -ggdb
debug: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

main: LD_FLAGS += -O3
main: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

-include $(OBJFILES:.o=.d)
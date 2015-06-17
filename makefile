CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -std=c++0x -fopenmp -D_GLIBC_XX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC 
CC_FLAGS := -std=c++0x -MMD -fopenmp -D_GLIBC_XX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC 

clear:
	rm main debug obj/*

#all: LD_FLAGS += -DOPT_NSGA2
#all: CC_FLAGS += -DOPT_NSGA2
all: debug main

debug: LD_FLAGS += -ggdb
debug: CC_FLAGS += -ggdb
debug: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

main: LD_FLAGS += -O3
main: CC_FLAGS += -ggdb
main: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^



obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

-include $(OBJFILES:.o=.d)
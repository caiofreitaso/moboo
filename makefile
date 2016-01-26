CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -std=c++0x -D_GLIBC_XX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fopenmp 
CC_FLAGS := -std=c++0x -MMD -D_GLIBC_XX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fopenmp 

#all: LD_FLAGS += -DOPT_NSGA2
#all: CC_FLAGS += -DOPT_NSGA2
all: add

add: debug main
	git add makefile src/* include/* model/* sys/* state/* hv/* irace/*

debug: LD_FLAGS += -ggdb
debug: CC_FLAGS += -ggdb
debug: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

main: LD_FLAGS += -Ofast
main: CC_FLAGS += -ggdb
main: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

clean:
	rm main debug obj/*


obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

-include $(OBJFILES:.o=.d)

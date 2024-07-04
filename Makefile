SRC=$(wildcard src/*.cpp) $(wildcard src/*.c)
HEADER=$(wildcard src/*.hpp) $(wildcard src/*.h)
OBJ=$(SRC:%=%.o)
CPP=c++
BIN_NAME=flightsim
INCLUDE=-Iinclude
FLAGS=$(INCLUDE) -std=c++17 -O2 -DDISALLOW_ERRORS
LD_FLAGS=-lglfw3

ifeq ($(OS), Windows_NT)
	LD_FLAGS+=-static-libgcc -static-libstdc++ -lopengl32 -lgdi32
else
	LD_FLAGS+=-lGL
endif

output: $(OBJ)
	$(CPP) $(OBJ) -o $(BIN_NAME) $(FLAGS) $(LD_FLAGS)

%.cpp.o: %.cpp $(HEADER)
	$(CPP) $(FLAGS) -c $< -o $@ 

%.c.o: %.c $(HEADER)
	$(CPP) $(FLAGS) -c $< -o $@ 

clean:
	rm -f $(OBJ) $(BIN_NAME)

run: output
	./$(BIN_NAME)

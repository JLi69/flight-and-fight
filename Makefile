SRC=$(wildcard src/*.cpp) $(wildcard src/*.c)
HEADER=$(wildcard src/*.hpp) $(wildcard src/*.h)
OBJ=$(SRC:%=%.o)
CPP=c++
BIN_NAME=flight-and-fight
INCLUDE=-Iinclude
FLAGS=$(INCLUDE) -std=c++17 -O2
LD_FLAGS=-lglfw3

ifeq ($(OS), Windows_NT)
	LD_FLAGS+=-lOpenAL32
	LD_FLAGS+=-static-libgcc -static-libstdc++ -lopengl32 -lgdi32 -mwindows
	OBJ+=flight-and-fight.res
else
	# Comment out the line below if you want to build a copy to distribute
	# that has the binary search in the system path only for OpenAL
	LD_FLAGS+=-Wl,-rpath='$$ORIGIN'
	LD_FLAGS+=-lopenal
	LD_FLAGS+=-lGL
endif

output: $(OBJ)
	$(CPP) $(OBJ) -o $(BIN_NAME) $(FLAGS) $(LD_FLAGS)

%.cpp.o: %.cpp $(HEADER)
	$(CPP) $(FLAGS) -c $< -o $@ 

%.c.o: %.c $(HEADER)
	$(CPP) $(FLAGS) -c $< -o $@ 

flight-and-fight.res: flight-and-fight.rc
	windres flight-and-fight.rc -O coff -o flight-and-fight.res

clean:
	rm -f $(OBJ) $(BIN_NAME)

run: output
	./$(BIN_NAME)

test: $(OBJ)
	@cd tests && make -j$(nproc)

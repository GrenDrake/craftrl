PLATFORM=OSX
BEARLIBTERM=../lib/BearLibTerminal

CXXFLAGS=-std=c++11 -Wall -g -I$(BEARLIBTERM)/Include/C
LIBS=-L$(BEARLIBTERM)/$(PLATFORM) -lBearLibTerminal
OBJS=src/startup.o src/craftrl.o src/build_map.o src/world.o src/lodepng.o src/data_lexer.o src/data_load.o src/input.o src/crafting.o src/actions.o src/ui.o
TARGET=craftrl

TEST_SYS_OBJS=src/build_map.o src/world.o src/lodepng.o src/data_lexer.o src/data_load.o

all: $(TARGET) tests

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $(TARGET)

tests: tests/buildmap

tests/buildmap: tests/buildmap.o $(TEST_SYS_OBJS)
	$(CXX) tests/buildmap.o $(TEST_SYS_OBJS) -o tests/buildmap
	tests/buildmap

clean:
	$(RM) src/*.o $(TARGET)

.PHONY: all tests clean

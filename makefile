PLATFORM=OSX
BEARLIBTERM=../lib/BearLibTerminal
PHYSICFS=../lib/physfs-3.0.2

CXXFLAGS=-std=c++11 -Wall -g -I$(BEARLIBTERM)/Include/C -I$(PHYSICFS)/src
LIBS=-L$(BEARLIBTERM)/$(PLATFORM) -lBearLibTerminal -L$(PHYSICFS)/build -lphysfs
OBJS=src/startup.o src/craftrl.o src/build_map.o src/world.o src/lodepng.o src/data_lexer.o src/data_load.o src/input.o src/crafting.o src/actions.o src/ui.o src/point.o src/runmenu.o src/utility.o src/logger.o src/debug.o src/dump_map.o src/trading.o src/config.o
TARGET=craftrl

all: $(TARGET) tests

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $(TARGET)

tests: tests/test_utility

tests/test_utility: tests/test.o tests/test_utility.o src/utility.o
	$(CXX) tests/test.o tests/test_utility.o src/utility.o -L$(PHYSICFS)/build -lphysfs -o tests/test_utility
	tests/test_utility

clean:
	$(RM) src/*.o $(TARGET)

.PHONY: all tests clean

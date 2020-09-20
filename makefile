
BEARLIBTERM=../lib/BearLibTerminal

CXXFLAGS=-std=c++11 -Wall -g -I$(BEARLIBTERM)/Include/C
LIBS=-L$(BEARLIBTERM)/Windows32 -lBearLibTerminal
OBJS=src/startup.o src/craftrl.o src/build_map.o src/world.o src/lodepng.o src/data_lexer.o src/data_load.o
TARGET=craftrl

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $(TARGET)

clean:
	$(RM) src/*.o $(TARGET)

.PHONY: clean

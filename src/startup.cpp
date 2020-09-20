#include <iostream>
#include <BearLibTerminal.h>

#include "world.h"


void gameloop(World &w);
bool loadGameData(World &w);



int main() {
    World w;

    if (!loadGameData(w)) return 1;

    terminal_open();
    terminal_set("window: size=80x25; window.title='CraftRL'");
    terminal_set("input.filter=keyboard,mouse");

    gameloop(w);

    terminal_close();

    return 0;
}


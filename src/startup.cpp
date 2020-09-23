#include <ctime>
#include <BearLibTerminal.h>

#include "world.h"


void mainmenu(World &w);
void gameloop(World &w);
bool loadGameData(World &w);



int main() {
    World w;
    w.getRandom().seed(time(nullptr));

    if (!loadGameData(w)) return 1;

    terminal_open();
    terminal_set("window: size=80x25; window.title='CraftRL'");
    terminal_set("input.filter=keyboard,mouse");

    mainmenu(w);

    terminal_close();

    return 0;
}


void setColourIfSelected(int selection, int position, bool enabled) {
    if (!enabled) {
        terminal_color(0xFF777777);
    } else if (selection == position)  {
        terminal_color(0xFFFFFF77);
        terminal_print(0, 6 + position, "-->");
    } else {
        terminal_color(0xFFAAAA66);
    }
}
void mainmenu(World &w) {

    int selection = 0;
    unsigned long seed = w.getRandom().next64();
    while (1) {
        terminal_clear();
        terminal_color(0xFFFFFFFF);
        terminal_print(5, 3, "CraftRL Tech Demo");

        setColourIfSelected(selection, 0, true);
        terminal_print(5, 6, "New Game");
        setColourIfSelected(selection, 1, true);
        terminal_printf(5, 7, "Seed: %lu", seed);
        setColourIfSelected(selection, 2, w.inProgress);
        terminal_print(5, 8, "Continue Game");
        setColourIfSelected(selection, 3, true);
        terminal_print(5, 9, "Quit");
        terminal_refresh();

        int key = terminal_read();
        if (key == TK_CLOSE || key == TK_ESCAPE || key == TK_Q) return;
        if (key == TK_DOWN && selection < 3) {
            ++selection;
            if (selection == 2 && !w.inProgress) ++selection;
        }
        if (key == TK_UP && selection > 0) {
            --selection;
            if (selection == 2 && !w.inProgress) --selection;
        }
        if (key == TK_ENTER || key == TK_KP_ENTER || key == TK_SPACE) {
            if (selection == 0) {
                w.allocMap(80, 80);
                buildmap(w);
                Actor *player = new Actor(w.getActorDef(1));
                Point starting = w.findOpenTile(false, true);
                player->reset();
                w.moveActor(player, starting);
                w.mode = w.selection = 0;
                w.inProgress = true;
                ++selection;

                gameloop(w);
            } else if (selection == 2) {
                if (w.inProgress) {
                    gameloop(w);
                }
            } else if (selection == 3) {
                return;
            }
        }
        if (selection == 1) {
            if (key == TK_R) seed = w.getRandom().next64();
            if (key == TK_C) seed = 0;
            if (key == TK_LEFT) --seed;
            if (key == TK_RIGHT) ++seed;
            if (key == TK_BACKSPACE && seed != 0) seed /= 10;
            if (key == TK_1) { seed *= 10; seed += 1; }
            if (key == TK_2) { seed *= 10; seed += 2; }
            if (key == TK_3) { seed *= 10; seed += 3; }
            if (key == TK_4) { seed *= 10; seed += 4; }
            if (key == TK_5) { seed *= 10; seed += 5; }
            if (key == TK_6) { seed *= 10; seed += 6; }
            if (key == TK_7) { seed *= 10; seed += 7; }
            if (key == TK_8) { seed *= 10; seed += 8; }
            if (key == TK_9) { seed *= 10; seed += 9; }
            if (key == TK_0) { seed *= 10; }
        }
    }
}


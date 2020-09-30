#include <iostream>
#include <ctime>
#include <sstream>
#include <BearLibTerminal.h>

#include "world.h"


void mainmenu(World &w);
void gameloop(World &w);
bool loadGameData(World &w);
bool buildmap(World &w, unsigned long seed);
void keybinds();
std::string keyName(int key);



int main() {
    World w;
    w.getRandom().seed(time(nullptr));

    if (!loadGameData(w)) return 1;

    std::stringstream nameString;
    nameString << "CraftRL" << " v" << VER_MAJOR << '.' << VER_MINOR << '.' << VER_PATCH;
    terminal_open();
    terminal_set("window: size=80x25");
    terminal_set(("window.title='" + nameString.str() + "'").c_str());
    terminal_set("input.filter=keyboard,mouse_left,mouse_middle,mouse_right");

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
    int selection = 1;
    unsigned long seed = w.getRandom().next64();
    unsigned long usedSeed = 0;
    while (1) {
        terminal_bkcolor(0xFF000000);
        terminal_color(0xFFFFFFFF);
        terminal_clear();
        terminal_print(5, 3, "CraftRL");
        terminal_printf(70, 24, "%d.%d.%d", VER_MAJOR, VER_MINOR, VER_PATCH);

        setColourIfSelected(selection, 0, true);
        terminal_print(5, 6, "New Game");
        setColourIfSelected(selection, 1, true);
        terminal_print(5, 7, "Load Game");
        setColourIfSelected(selection, 2, true);
        terminal_printf(5, 8, "Seed: %lu", seed);
        setColourIfSelected(selection, 3, w.inProgress);
        terminal_print(5, 9, "Continue Game");
        setColourIfSelected(selection, 4, true);
        terminal_print(5, 10, "Keybindings");
        setColourIfSelected(selection, 5, true);
        terminal_print(5, 11, "Quit");
        terminal_refresh();

        int key = terminal_read();
        if (key == TK_CLOSE || key == TK_ESCAPE || key == TK_Q) return;
        if (key == TK_DOWN && selection < 5) {
            ++selection;
            if (selection == 3 && !w.inProgress) ++selection;
        }
        if (key == TK_UP && selection > 0) {
            --selection;
            if (selection == 3 && !w.inProgress) --selection;
        }
        if (key == TK_ENTER || key == TK_KP_ENTER || key == TK_SPACE) {
            if (selection == 0) {
                w.allocMap(160, 160);
                usedSeed = seed;
                buildmap(w, usedSeed);
                w.inProgress = true;
                selection = 3;
                std::cerr << "mainmenu (info): created new map (size " << w.width() << ',' << w.height() << ", seed " << seed << ").\n";
                std::cerr << "mainmenu (info): player starting position is " << w.getPlayer()->pos << ".\n";
                gameloop(w);
                std::cerr << "mainmenu (info): returned to menu.\n";
            } else if (selection == 1) {
                if (w.loadgame("game.sav")) {
                    w.inProgress = true;
                    selection = 3;
                    std::cerr << "mainmenu (info): loaded game from save.\n";
                    std::cerr << "mainmenu (info): initial player position is " << w.getPlayer()->pos << ".\n";
                    gameloop(w);
                    std::cerr << "mainmenu (info): returned to menu.\n";
                } else {
                    std::cerr << "mainmenu: failed to load save game.\n";
                    ui_MessageBox("Error", "Failed to load game.");
                }
            } else if (selection == 3) {
                if (w.inProgress) {
                    std::cerr << "mainmenu (info): resuming on previous map.\n";
                    gameloop(w);
                    std::cerr << "mainmenu (info): returned to menu.\n";
                    seed = usedSeed;
                }
            } else if (selection == 4) {
                keybinds();
            } else if (selection == 5) {
                return;
            }
        }
        if (selection == 2) {
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


void keybinds() {
    int commandCount = 0;
    for (int i = 0; gameCommands[i].command != CMD_NONE; ++i) ++commandCount;

    int selection = 0;
    while (1) {
        terminal_bkcolor(0xFF000000);
        terminal_clear();
        terminal_color(0xFF000000);
        terminal_bkcolor(0xFFAAAAAA);
        terminal_clear_area(0, 0, 80, 1);
        terminal_clear_area(0, 24, 80, 1);
        terminal_print(0, 0,  "Action");
        terminal_print(27, 0, "Direction");
        terminal_print(40, 0, "Key 1");
        terminal_print(50, 0, "Key 2");
        terminal_print(60, 0, "Key 3");
        terminal_print(0, 24,  "UP/DOWN - select row   ESCAPE - return to menu");
        terminal_bkcolor(0xFF000000);

        int top = selection - 10;
        for (int i = 0; gameCommands[i].command != CMD_NONE; ++i) {
            int yPos = i - top;
            if (yPos < 2) continue;
            if (yPos > 22) continue;

            if (i == selection) {
                terminal_bkcolor(0xFFAAAAAA);
                terminal_color(0xFF000000);
                terminal_clear_area(0, yPos, 80, 1);
            } else {
                terminal_color(0xFFAAAAAA);
                terminal_bkcolor(0xFF000000);
            }

            terminal_print(0, yPos, commandName(gameCommands[i].command).c_str());
            if (gameCommands[i].dir != Dir::None) {
                terminal_print(27, yPos, directionName(gameCommands[i].dir).c_str());
            }
            for (unsigned j = 0; j < INPUT_KEY_COUNT; ++j) {
                terminal_print(40 + j * 10, yPos, keyName(gameCommands[i].key[j].key).c_str());
            }
        }
        terminal_refresh();

        int key = terminal_read();
        if (key == TK_ESCAPE || key == TK_CLOSE || key == TK_Q || key == TK_Z) {
            return;
        }
        if (key == TK_DOWN && selection < commandCount - 1) ++selection;
        if (key == TK_UP && selection > 0)                  --selection;
    }
}

std::string keyName(int key) {
    switch(key) {
        case 0:             return "";
        case TK_A:          return "A";
        case TK_B:          return "B";
        case TK_C:          return "C";
        case TK_D:          return "D";
        case TK_E:          return "E";
        case TK_F:          return "F";
        case TK_G:          return "G";
        case TK_H:          return "H";
        case TK_I:          return "I";
        case TK_J:          return "J";
        case TK_K:          return "K";
        case TK_L:          return "L";
        case TK_M:          return "M";
        case TK_N:          return "N";
        case TK_O:          return "O";
        case TK_P:          return "P";
        case TK_Q:          return "Q";
        case TK_R:          return "R";
        case TK_S:          return "S";
        case TK_T:          return "T";
        case TK_U:          return "U";
        case TK_V:          return "V";
        case TK_W:          return "W";
        case TK_X:          return "X";
        case TK_Y:          return "Y";
        case TK_Z:          return "Z";

        case TK_1:          return "1";
        case TK_2:          return "2";
        case TK_3:          return "3";
        case TK_4:          return "4";
        case TK_5:          return "5";
        case TK_6:          return "6";
        case TK_7:          return "7";
        case TK_8:          return "8";
        case TK_9:          return "9";
        case TK_0:          return "0";

        case TK_F1:         return "F1";
        case TK_F2:         return "F2";
        case TK_F3:         return "F3";
        case TK_F4:         return "F4";
        case TK_F5:         return "F5";
        case TK_F6:         return "F6";
        case TK_F7:         return "F7";
        case TK_F8:         return "F8";
        case TK_F9:         return "F9";
        case TK_F10:        return "F10";
        case TK_F11:        return "F11";
        case TK_F12:        return "F12";

        case TK_LEFT:       return "Left";
        case TK_RIGHT:      return "Right";
        case TK_UP:         return "Up";
        case TK_DOWN:       return "Down";
        case TK_ENTER:      return "Enter";
        case TK_SPACE:      return "Space";
        case TK_PAGEDOWN:   return "Pagedown";
        case TK_PAGEUP:     return "Pageup";
        case TK_HOME:       return "Home";
        case TK_END:        return "End";

        case TK_KP_1:       return "KP 1";
        case TK_KP_2:       return "KP 2";
        case TK_KP_3:       return "KP 3";
        case TK_KP_4:       return "KP 4";
        case TK_KP_5:       return "KP 5";
        case TK_KP_6:       return "KP 6";
        case TK_KP_7:       return "KP 7";
        case TK_KP_8:       return "KP 8";
        case TK_KP_9:       return "KP 9";
        case TK_KP_0:       return "KP 0";
        case TK_KP_ENTER:   return "KP Enter";
        case TK_KP_PLUS:    return "KP Plus";
        case TK_KP_MINUS:   return "KP Minus";

        case TK_LBRACKET:   return "[[";
        case TK_RBRACKET:   return "]]";
        case TK_ESCAPE:     return "Escape";
        case TK_CLOSE:      return "Close";
        default: {
            std::stringstream s;
            s << "(key#" << key << ')';
            return s.str(); }
    }
}
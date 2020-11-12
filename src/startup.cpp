#include <iostream>
#include <ctime>
#include <sstream>
#include <BearLibTerminal.h>
#include <physfs.h>

#include "runmenu.h"
#include "world.h"



void mainmenu(World &w);
void gameloop(World &w);
bool loadGameData(World &w, const std::string &filename);
bool buildmap(World &w, unsigned long seed);
void newgame(World &w);
void keybinds();
std::string keyName(int key);



int main(int argc, char *argv[]) {

    if (!PHYSFS_init(argv[0])) {
        std::cerr << "Failed to initialize PhysicsFS: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << ".\n";
        return 1;
    }

    const char *baseDir = PHYSFS_getBaseDir();
    if (!baseDir) {
        std::cerr << "Failed to get base directory: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << ".\n";
        PHYSFS_deinit();
        return 1;
    }
    const char *prefDir = PHYSFS_getPrefDir("grendrake", "craftrl");
    if (!prefDir) {
        std::cerr << "Failed to get pref dir directory: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << ".\n";
        PHYSFS_deinit();
        return 1;
    }
    if (!PHYSFS_setWriteDir(prefDir)) {
        std::cerr << "Failed to set write directory: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << ".\n";
        PHYSFS_deinit();
        return 1;
    }

    PHYSFS_mount(baseDir, "/", true);
    std::cerr << prefDir << '\n';
    PHYSFS_mount(prefDir, "/save", false);

    logger_setFile("game.log");
    World w;
    w.getRandom().seed(time(nullptr));

    if (!loadGameData(w, "game.dat")) return 1;

    std::stringstream nameString;
    nameString << "CraftRL" << " v" << VER_MAJOR << '.' << VER_MINOR << '.' << VER_PATCH;
    if (!terminal_open()) {
        logger_log("main: Failed to initialize BearLibTerm.");
        return 1;
    }
    if (!terminal_set(("window: size=80x25; window.title='" + nameString.str() + "'").c_str())) {
        logger_log("main: failed to set window properties.");
        return 1;
    }
    if (!terminal_set("input.filter=keyboard,mouse_left,mouse_middle,mouse_right")) {
        logger_log("main: failed to set input filter.");
        return 1;
    }
    if (!terminal_set("font: ./data/font.ttf, size=14x28")) {
        logger_log("main: failed to set font.");
        return 1;
    }
    if (!terminal_set("0xE000: ./data/tileset.png, size=28x28")) {
        logger_log("main: failed to set tileset.");
        return 1;
    }

    mainmenu(w);

    logger_close();
    terminal_close();
    PHYSFS_deinit();
    return 0;
}

Menu mainmenuMenu {
    "CraftRL", "Version Text",
    {
        {   "New Game",         0,  false,  MENU_SELECT },
        {   "Load Game",        1,  false,  MENU_SELECT },
        {   "Continue Game",    2,  true,   MENU_SELECT },
        {   "---",              -1, true,   MENU_SPACE },
        {   "Keybindings",      4,  false,  MENU_SELECT },
        {   "---",              -1, true,   MENU_SPACE },
        {   "Quit",             3,  false,  MENU_SELECT },
    }
};

void mainmenu(World &w) {
    mainmenuMenu.titleRight = std::to_string(VER_MAJOR) + "." + std::to_string(VER_MINOR) + "." + std::to_string(VER_PATCH);

    while (1) {
        mainmenuMenu.selection = w.inProgress ? 2 : 1;
        mainmenuMenu.items[2].disabled = !w.inProgress;
        int result = runMenu(mainmenuMenu);

        switch(result) {
            case 0:
                newgame(w);
                break;
            case 1: // load game
                ui_MessageBox_Instant("Loading saved game...");
                if (w.loadgame("game.sav")) {
                    w.inProgress = true;
                    logger_log("mainmenu (info): loaded game from save.");
                    logger_log("mainmenu (info): initial player position is "
                                + w.getPlayer()->pos.toString() + ".");
                    gameloop(w);
                    logger_log("mainmenu (info): returned to menu.");
                } else {
                    logger_log("mainmenu: failed to load save game.");
                    ui_MessageBox("Error", "Failed to load game.");
                }
                break;
            case 2: // continue game
                if (w.inProgress) {
                    logger_log("mainmenu (info): resuming on previous map.");
                    gameloop(w);
                    logger_log("mainmenu (info): returned to menu.");
                } else logger_log("mainmenu: tried to continue non-existant game.");
                break;
            case 3:
            case MENU_CANCELED:
                return;
            case 4:
                keybinds();
                break;
        }
    }
}


Menu newgameMenu {
    "New Game", "CraftRL",
    {
        {   "World Name",       0,  false,  MENU_TEXT },
        {   "Seed",             1,  false,  MENU_TEXT },
        {   "Size",             4,  false,  MENU_INT,   "", 128, 64, 1024 },
        {   "Begin",            2,  false,  MENU_SELECT },
        {   "Cancel",           3,  false,  MENU_SELECT },
    }
};


void newgame(World &w) {
    newgameMenu.items[0].strValue = "New World";
    newgameMenu.items[1].strValue = "";
    newgameMenu.selection = 0;
    while (1) {
        int result = runMenu(newgameMenu);

        switch(result) {
            case 2: {
                unsigned long seed = 0;
                if (newgameMenu.items[1].strValue.empty()) {
                    seed = w.getRandom().next32();
                } else {
                    seed = hashString(newgameMenu.items[1].strValue);
                }
                w.inProgress = true;
                int size = newgameMenu.items[2].intValue;
                w.allocMap(size, size);
                buildmap(w, seed);
                logger_log("newgame (info): created new map (size " + std::to_string(w.width())
                            + "," + std::to_string(w.height()) + ", seed " + std::to_string(seed) + ").");
                logger_log("newgame (info): player starting position is " + w.getPlayer()->pos.toString() + ".");
                gameloop(w);
                logger_log("newgame (info): returned to menu.");
                return; }
            case 3:
            case MENU_CANCELED:
                return;
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
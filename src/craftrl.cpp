#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <BearLibTerminal.h>

#include "world.h"

Dir getDir(World &w, const std::string &reason);
void redraw_main(World &w);


Dir getDir(World &w, const std::string &reason) {
    w.addLogMsg(reason + ". Which way? (Z to cancel)");
    redraw_main(w);
    terminal_refresh();
    while (1) {
        int key = terminal_read();
        const Command &command = findCommand(key, gameCommands);
        if (command.command == CMD_CONTEXTMOVE) {
            return command.dir;
        }
        if (command.command == CMD_QUIT) {
            w.wantsToQuit = true;
            return Dir::None;
        }
        if (command.command == CMD_CANCEL) return Dir::None;
    }
}


void viewLog(World &w) {
    const int screenHeight = 24;
    unsigned top = 0;

    terminal_bkcolor(0xFF000000);
    terminal_color(0xFFFFFFFF);
    while (1) {
        terminal_clear();
        for (int i = 0; i < screenHeight; ) {
            const LogMessage &msg = w.getLogMsg(i + top);
            terminal_color(0xFFCCCCCC);
            dimensions_t size = terminal_measure_ext(80, 3, msg.msg.c_str());
            terminal_print_ext(0, screenHeight - i - size.height, 80, 3, TK_ALIGN_LEFT, msg.msg.c_str());
            i += size.height;
        }
        terminal_refresh();

        int key = terminal_read();
        if (key == TK_ESCAPE || key == TK_Q || key == TK_Z || key == TK_CLOSE) return;
        if (key == TK_DOWN && top > 0) --top;
        if (key == TK_UP) ++top;
        if (key == TK_HOME) top = 0;
        if (key == TK_END) top = w.getLogCount() - screenHeight;
    }
}


void redraw_main(World &w) {
    const Actor *player = w.getPlayer();
    Point camera = w.getCamera();
    const int screenWidth = 80;
    const int screenHeight = 25;
    const int sidebarWidth = 30;
    const int sidebarX = screenWidth - sidebarWidth;
    const int logHeight = 3;
    const int logX = 0;
    const int logY = screenHeight - logHeight;
    const int viewWidth = screenWidth - sidebarWidth - 1;
    const int viewHeight = screenHeight - logHeight - 1;

    terminal_bkcolor(0xFF000000);
    terminal_color(0xFFFFFFFF);
    terminal_clear();
    for (int i = 0; i < logY; ++i) {
        terminal_put(sidebarX - 1, i, LD_VERTICAL);
    }
    for (int i = logX; i < screenWidth; ++i) {
        terminal_put(i, logY - 1, LD_HORIZONTAL);
    }
    terminal_put(sidebarX - 1, logY - 1, LD_TEE_LRU);

    terminal_composition(TK_ON);
    for (int y = 0; y < viewHeight; ++y) {
        for (int x = 0; x < viewWidth; ++x) {
            const auto &tile = w.at(Point(x + camera.x, y + camera.y));

            if (tile.room)  terminal_bkcolor(tile.room->def->colour);
            else            terminal_bkcolor(0xFF000000);

            terminal_color(w.getTileDef(tile.terrain).colour);
            terminal_put(x, y, w.getTileDef(tile.terrain).glyph);
            if (tile.building > 0) {
                terminal_put(x, y, w.getTileDef(tile.building).glyph);
            }
            if (tile.item) {
                terminal_color(tile.item->def.colour);
                terminal_put(x, y, tile.item->def.glyph);
            }
            if (tile.actor) {
                terminal_color(tile.actor->def.colour);
                terminal_put(x, y, tile.actor->def.glyph);
            }
        }
    }
    terminal_composition(TK_OFF);

    int day = -1, hour = -1, minute = -1;
    w.getTime(&day, &hour, &minute);
    terminal_color(0xFFFFFFFF);
    terminal_printf(0, logY - 1, " HP: %d/%d ", player->health, player->def.health);
    terminal_printf(15, logY - 1, " POS:%d,%d ", player->pos.x, player->pos.y);
    terminal_printf(30, logY - 1, " %2d:%02d Day:%d ", hour, minute, day);
    terminal_printf(screenWidth - 15, logY - 1, " Turn: %u ", w.getTurn());

    for (int i = 0; i < player->inventory.size(); ++i) {
        const InventoryRow &row = player->inventory.mContents[i];
        if (i == w.selection)   terminal_color(0xFFFFFFFF);
        else                    terminal_color(0xFF777777);
        terminal_print(sidebarX, i, (row.def->name + "  x" + std::to_string(row.qty)).c_str());
    }

    terminal_color(0xFFFFFFFF);
    for (int i = 0; i < logHeight; ) {
        const LogMessage &msg = w.getLogMsg(i);
        if (i == 0) terminal_color(0xFFFFFFFF);
        else        terminal_color(0xFF777777);
        dimensions_t size = terminal_measure_ext(80, 3, msg.msg.c_str());
        terminal_print_ext(logX, screenHeight - i - size.height, 80, 3, TK_ALIGN_LEFT, msg.msg.c_str());
        i += size.height;
    }
}


void gameloop(World &w) {
    w.mode = w.selection = 0;
    w.wantsToQuit = false;
    Actor *player = w.getPlayer();
    if (player->def.ident != 1) {
        logger_log("gameloop: player is not of type player.");
        return;
    }
    actionCentrePan(w, player, Command{ CMD_NONE }, true);


    bool wantTick = false;
    while (!w.wantsToQuit) {
        redraw_main(w);
        terminal_refresh();
        int key = terminal_read();

        if (key == TK_P) {
            player->inventory.sort(SORT_NAME);
        }

        if (key == TK_MOUSE_RIGHT) {
            const Point &camera = w.getCamera();
            int mx = terminal_state(TK_MOUSE_X);
            int my = terminal_state(TK_MOUSE_Y);
            const Tile &tile = w.at(Point(mx + camera.x, my + camera.y));
            if (tile.terrain < 0) {
                w.addLogMsg("Invalid position.");
            } else {
                std::stringstream s;
                s << "You see: " << w.getTileDef(tile.terrain).name;
                if (tile.building) {
                    s << ", " << w.getTileDef(tile.building).name;
                }
                if (tile.item) {
                    s << ", " << tile.item->getName(false);
                }
                if (tile.actor) {
                    s << ", " << tile.actor->getName();
                }
                if (tile.room) {
                    s << " in " << tile.room->def->name << "(";
                    s << "size:" << tile.room->points.size() << ')';
                }
                w.addLogMsg(s.str());
            }

        } else {
            const Command &command = findCommand(key, gameCommands);
            ActionHandler handler = commandAction(command.command);
            if (handler) {
                wantTick = handler(w, player, command, false);
            }
        }

        if (wantTick) {
            w.tick();
            wantTick = false;
        }
    }
}




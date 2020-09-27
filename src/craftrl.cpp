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

    terminal_clear();
    terminal_color(0xFFFFFFFF);
    for (int i = 0; i < logY; ++i) {
        terminal_put(sidebarX - 1, i, '|');
    }
    for (int i = logX; i < screenWidth; ++i) {
        terminal_put(i, logY - 1, '-');
    }
    terminal_put(sidebarX - 1, logY - 1, '+');

    for (int y = 0; y < viewHeight; ++y) {
        for (int x = 0; x < viewWidth; ++x) {
            auto tile = w.at(Point(x + camera.x, y + camera.y));

            if (tile.actor) {
                terminal_color(tile.actor->def.colour);
                terminal_put(x, y, tile.actor->def.glyph);
            } else if (tile.item) {
                terminal_color(tile.item->def.colour);
                terminal_put(x, y, tile.item->def.glyph);
            } else {
                terminal_color(w.getTileDef(tile.terrain).colour);
                terminal_put(x, y, w.getTileDef(tile.terrain).glyph);
            }
        }
    }

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
    for (int i = 0; i < logHeight; ++i) {
        const LogMessage &msg = w.getLogMsg(i);
        terminal_print(logX, screenHeight - i - 1, msg.msg.c_str());
    }
}


void gameloop(World &w) {
    w.mode = w.selection = 0;
    w.wantsToQuit = false;
    Actor *player = w.getPlayer();
    if (player->def.ident != 1) {
        std::cerr << "gameloop: player is not of type player.\n";
        return;
    }
    actionCentrePan(w, player, Command{ CMD_NONE }, true);


    bool wantTick = false;
    while (!w.wantsToQuit) {
        redraw_main(w);
        terminal_refresh();
        int key = terminal_read();

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
                if (tile.item) {
                    s << ", " << tile.item->def.name;
                }
                if (tile.actor) {
                    s << ", " << tile.actor->def.name;
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




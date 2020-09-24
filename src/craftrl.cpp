#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <BearLibTerminal.h>

#include "lodepng.h"
#include "world.h"


Dir getDir(World &w, const std::string &reason);
void redraw_main(World &w);
void doCrafting(World &w, Actor *player);


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


void makeLootAt(World &w, const LootTable *table, const Point &where) {
    if (!table || table->mRows.empty()) return;

    std::stringstream s;
    for (const LootRow &row : table->mRows) {
        if (row.ident < 0 || row.chance < 0 || row.max < 0) continue;
        const ItemDef &def = w.getItemDef(row.ident);
        if (def.ident < 0) continue;
        int chance = w.getRandom().next32() % 100;
        int qty = w.getRandom().between(row.min, row.max);
        for (int i = 0; i < qty; ++i ) {
            if (chance < row.chance) {
                Point dest = w.findDropSpace(where);
                if (w.valid(dest)) {
                    s << "Dropped " << def.name << ". ";
                    Item *item = new Item(def);
                    if (!w.moveItem(item, dest)) delete item;
                }
            }
        }
    }
    w.addLogMsg(s.str());
}


void shiftCameraForMove(World &w, Actor *player) {
    const int screenWidth = 80;
    const int screenHeight = 25;
    const int sidebarWidth = 30;
    const int logHeight = 3;
    const int viewWidth = screenWidth - sidebarWidth - 1;
    const int viewHeight = screenHeight - logHeight - 1;
    Point dest = player->pos;
    Point p = w.getCamera();
    if (dest.x < p.x + 10) p.x -= 10;
    if (dest.y < p.y + 5)  p.y -= 5;
    if (dest.x > p.x + viewWidth - 10) p.x += 10;
    if (dest.y > p.y + viewHeight - 5) p.y += 5;
    w.setCamera(p);

}

bool actionBreak(World &w, Actor *player, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Break");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    Point dest = player->pos.shift(dir);
    Tile &tile = w.at(dest);

    if (tile.actor) {
        if (tile.actor->def.faction == FAC_PLANT) {
            w.addLogMsg("Broke " + tile.actor->def.name + '.');
            Actor *actor = tile.actor;
            w.moveActor(actor, Point(-1, -1));
            makeLootAt(w, actor->def.loot, dest);
            delete actor;
            return true;
        } else {
            w.addLogMsg("Can't break that.");
        }
    }

    const TileDef &td = w.getTileDef(tile.terrain);
    if (td.breakTo >= 0) {
        tile.terrain = td.breakTo;
        w.addLogMsg("Broken.");
        makeLootAt(w, td.loot, dest);
    } else {
        w.addLogMsg("Nothing to break.");
    }
    return true;
}

bool actionContextMove(World &w, Actor *player, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Move");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    Point dest = player->pos.shift(dir);
    if (!w.valid(dest)) return false;

    const Tile &tile = w.at(dest);
    if (tile.actor) {
        std::stringstream s;
        switch (tile.actor->def.faction) {
            case FAC_ANIMAL:
                s << tile.actor->def.name << ": It has nothing to say.";
                w.addLogMsg(s.str());
                break;
            case FAC_VILLAGER:
                s << tile.actor->def.name << ": \"Hello!\"";
                w.addLogMsg(s.str());
                break;
            default:
                return false;
        }
        return true;
    } else {
        if (w.getTileDef(tile.terrain).solid) return false;
        if (w.moveActor(player, dest)) {
            if (tile.item) {
                w.addLogMsg("Item here: " + tile.item->def.name);
            }
            shiftCameraForMove(w, player);
            return true;
        }
        return false;
    }
}

bool actionCraft(World &w, Actor *player, Dir dir) {
    doCrafting(w, player);
    return false;
}

bool actionMove(World &w, Actor *player, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Move");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    Point dest = player->pos.shift(dir);
    const Tile &tile = w.at(dest);
    if (!w.valid(dest) || tile.actor || w.getTileDef(tile.terrain).solid) {
        w.addLogMsg("Blocked.");
        return false;
    }
    if (w.moveActor(player, dest)) {
        if (tile.item) {
            w.addLogMsg("Item here: " + tile.item->def.name);
        }
        shiftCameraForMove(w, player);
        return true;
    }
    return false;
}

bool actionDrop(World &w, Actor *player, Dir dir) {
    if (w.selection < 0 || w.selection >= player->inventory.size()) {
        w.addLogMsg("Nothing to drop.");
        return false;
    }

    Point dropAt = w.findDropSpace(player->pos);
    if (!w.valid(dropAt) || w.at(dropAt).item) {
        w.addLogMsg("No space to drop item.");
        return false;
    }

    const ItemDef *def = player->inventory.mContents[w.selection].def;
    Item *item = new Item(*def);
    if (w.moveItem(item, dropAt)) {
        player->inventory.remove(def);
        w.addLogMsg("Dropped " + def->name + ".");
        if (w.selection > 0 && w.selection >= player->inventory.size()) {
            w.selection = player->inventory.size() - 1;
        }
    } else {
        delete item;
    }

    return true;
}

bool actionWait(World &w, Actor *player, Dir dir) {
    return true;
}

bool actionUse(World &w, Actor *player, Dir dir) {
    if (w.selection < 0 || w.selection >= player->inventory.size()) {
        w.addLogMsg("Nothing to drop.");
        return false;
    }

    const ItemDef *def = player->inventory.mContents[w.selection].def;
    if (def->seedFor >= 0) {
        Dir d = getDir(w, "Plant");
        if (dir == Dir::None) {
            w.addLogMsg("Canceled.");
            return false;
        }
        Point dest = player->pos.shift(d);
        const Tile &t = w.at(dest);
        const TileDef &td = w.getTileDef(t.terrain);
        if (td.solid || t.actor) {
            w.addLogMsg("The space isn't clear.");
            return false;
        }
        const ActorDef &actorDef = w.getActorDef(def->seedFor);
        player->inventory.remove(def);
        if (actorDef.ident < 0) return false;
        Actor *actor = new Actor(actorDef);
        w.moveActor(actor, dest);
        return true;
    } else {
        w.addLogMsg("That's not something you can use.");
        return false;
    }
}

bool actionTake(World &w, Actor *player, Dir dir) {
    Item *item = w.at(player->pos).item;
    if (!item) {
        w.addLogMsg("Nothing to take.");
        return false;
    }

    if (player->inventory.add(&item->def)) {
        w.moveItem(item, Point(-1, -1));
        w.addLogMsg("Took " + item->def.name + ".");
        delete item;
        return true;
    }
    return false;
}

bool actionPan(World &w, Actor *player, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Pan");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    Point dest = w.getCamera().shift(dir, 5);
    w.setCamera(dest);
    return false;
}

bool actionCentrePan(World &w, Actor *player, Dir dir) {
    const int viewWidth = 50;
    const int viewHeight = 25;

    Point dest(player->pos.x - viewWidth / 2, player->pos.y - viewHeight / 2);
    w.setCamera(dest);
    return false;
}


bool actionDumpMap(World &w, Actor *player, Dir dir) {
    std::vector<unsigned char> image;
    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            auto tile = w.at(Point(x, y));
            auto td = w.getTileDef(tile.terrain);
            image.push_back((td.colour & 0x00FF0000) >> 16);
            image.push_back((td.colour & 0x0000FF00) >> 8);
            image.push_back((td.colour & 0x000000FF));
            image.push_back(255);
        }
    }

    unsigned error = lodepng::encode("map.png", image, w.width(), w.height());
    if (error) {
        w.addLogMsg("encoder error ");// << error << ": "<< lodepng_error_text(error) << std::endl;
    }
    return false;
}


bool actionPrevSelect(World &w, Actor *player, Dir dir) {
    if (w.selection > 0) --w.selection;
    return false;
}

bool actionNextSelect(World &w, Actor *player, Dir dir) {
    if (w.selection < player->inventory.size() - 1) ++w.selection;
    return false;
}

bool actionQuit(World &w, Actor *player, Dir dir) {
    w.wantsToQuit = true;
    return false;
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

    terminal_color(0xFFFFFFFF);
    terminal_printf(0, logY - 1, " HP: %d/%d ", player->health, player->def.health);
    terminal_printf(15, logY - 1, " POS:%d,%d ", player->pos.x, player->pos.y);
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
    actionCentrePan(w, player, Dir::None);

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
            switch(command.command) {
                case CMD_DUMPMAP:       wantTick = actionDumpMap(w, player, command.dir);  break;
                case CMD_TAKE:          wantTick = actionTake(w, player, command.dir);  break;
                case CMD_BREAK:         wantTick = actionBreak(w, player, command.dir);  break;
                case CMD_MOVE:          wantTick = actionMove(w, player, command.dir);  break;
                case CMD_PAN:           wantTick = actionPan(w, player, command.dir);  break;
                case CMD_RESETVIEW:     wantTick = actionCentrePan(w, player, command.dir);  break;
                case CMD_DROP:          wantTick = actionDrop(w, player, command.dir);  break;
                case CMD_USE:           wantTick = actionUse(w, player, command.dir);  break;
                case CMD_WAIT:          wantTick = actionWait(w, player, command.dir);  break;
                case CMD_CONTEXTMOVE:   wantTick = actionContextMove(w, player, command.dir);  break;
                case CMD_QUIT:          wantTick = actionQuit(w, player, command.dir);  break;
                case CMD_NEXT_SELECT:   wantTick = actionNextSelect(w, player, command.dir); break;
                case CMD_PREV_SELECT:   wantTick = actionPrevSelect(w, player, command.dir); break;
                case CMD_CRAFT:         wantTick = actionCraft(w, player, command.dir); break;

            }
        }

        if (wantTick) {
            w.tick();
            wantTick = false;
        }
    }
}



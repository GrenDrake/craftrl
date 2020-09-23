#include <map>
#include <string>
#include <vector>
#include <BearLibTerminal.h>

#include "lodepng.h"
#include "world.h"



Dir getDir(World &w, const std::string &reason);
void redraw_main(World &w);


Dir keyToDir(int key) {
    switch (key) {
        case TK_UP:     return Dir::North;
        case TK_LEFT:   return Dir::West;
        case TK_DOWN:   return Dir::South;
        case TK_RIGHT:  return Dir::East;

        case TK_KP_1:   return Dir::Southwest;
        case TK_KP_2:   return Dir::South;
        case TK_KP_3:   return Dir::Southeast;
        case TK_KP_4:   return Dir::West;
        case TK_KP_6:   return Dir::East;
        case TK_KP_7:   return Dir::Northwest;
        case TK_KP_8:   return Dir::North;
        case TK_KP_9:   return Dir::Northeast;
        default:        return Dir::None;
    }
}

Dir getDir(World &w, const std::string &reason) {
    w.addLogMsg(reason + ". Which way?");
    redraw_main(w);
    terminal_refresh();
    while (1) {
        int key = terminal_read();
        if (key == TK_CLOSE)    return Dir::RequestQuit;
        if (key == TK_Z)        return Dir::None;
        Dir d = keyToDir(key);
        if (d != Dir::None) return d;
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

bool actionBreak(World &w, Actor *player, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Break");
    }
    if (dir == Dir::None || dir == Dir::RequestQuit) return false;

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
    if (dir == Dir::None || dir == Dir::RequestQuit) return false;

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
            return true;
        }
        return false;
    }
}

bool actionMove(World &w, Actor *player, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Move");
    }
    if (dir == Dir::None || dir == Dir::RequestQuit) return false;

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
        return true;
    }
    return false;
}

bool actionDrop(World &w, Actor *player, Dir dir) {
    if (w.selection < 0 || w.selection >= player->inventory.mContents.size()) {
        w.addLogMsg("Nothing to drop.");
        return false;
    }

    Point dropAt = w.findDropSpace(player->pos);
    if (!w.valid(dropAt) || w.at(dropAt).item) {
        w.addLogMsg("No space to drop item.");
        return false;
    }

    const ItemDef *def = player->inventory.mContents[0].def;
    Item *item = new Item(*def);
    if (w.moveItem(item, dropAt)) {
        player->inventory.remove(def);
        w.addLogMsg("Dropped " + def->name + ".");
    } else {
        delete item;
    }

    return true;
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

bool actionPan(World &w, Dir dir) {
    if (dir == Dir::None) {
        dir = getDir(w, "Pan");
    }
    if (dir == Dir::None || dir == Dir::RequestQuit) return false;

    Point dest = w.getCamera().shift(dir, 5);
    w.setCamera(dest);
    return false;
}

bool actionCentrePan(World &w, const Actor *player) {
    const int viewWidth = 50;
    const int viewHeight = 25;

    Point dest(player->pos.x - viewWidth / 2, player->pos.y - viewHeight / 2);
    w.setCamera(dest);
    return false;
}


bool actionDumpMap(World &w) {
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




void redraw_main(World &w) {
    const Actor *player = w.getPlayer();
    Point camera = w.getCamera();
    const int screenWidth = 80;
    const int screenHeight = 25;
    const int logWidth = 30;
    const int logHeight = 12;
    const int logX = screenWidth - logWidth;
    const int logY = screenHeight - logHeight;
    const int viewWidth = screenWidth - logWidth - 1;
    const int viewHeight = screenHeight;

    terminal_clear();
    terminal_color(0xFFFFFFFF);
    for (int i = 0; i < screenHeight; ++i) {
        terminal_put(logX - 1, i, '|');
    }
    for (int i = logX; i < screenWidth; ++i) {
        terminal_put(i, logY - 1, '-');
    }

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

    terminal_printf(logX + 2, logY - 1, " HP: %d/%d ", player->health, player->def.health);

    int iy = 0;
    for (auto iter : w.getPlayer()->inventory.mContents) {
        terminal_print(logX, iy, (iter.def->name + "  x" + std::to_string(iter.qty)).c_str());
        ++iy;
    }

    for (int i = 0; i < logHeight; ++i) {
        const LogMessage &msg = w.getLogMsg(i);
        terminal_print(logX, screenHeight - i - 1, "                                        ");
        terminal_print(logX, screenHeight - i - 1, msg.msg.c_str());
    }
}


void gameloop(World &w) {
    int j = 0;

    w.allocMap(80, 80);
    buildmap(w);
    Actor *player = new Actor(w.getActorDef(1));
    Point starting = w.findOpenTile(false, true);
    player->reset();
    w.moveActor(player, starting);
    actionCentrePan(w, player);
    w.mode = w.selection = 0;

    bool wantsToQuit = false;
    bool wantTick = false;
    while (!wantsToQuit) {
        redraw_main(w);
        terminal_refresh();
        int key = terminal_read();
        switch (key) {
            case TK_CLOSE:
            case TK_Q:
                wantsToQuit = true;
                break;
            case TK_A: {
                ++j;
                std::string m = "Test Me " + std::to_string(j);
                w.addLogMsg(LogMessage{m});
                break; }
            case TK_MOUSE_RIGHT: {
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
                break; }

            case TK_F5:     wantTick = actionDumpMap(w);                    break;
            case TK_G:      wantTick = actionTake(w, player, Dir::None);    break;
            case TK_B:      wantTick = actionBreak(w, player, Dir::None);   break;
            case TK_M:      wantTick = actionMove (w, player, Dir::None);   break;
            case TK_P:      wantTick = actionPan  (w, Dir::None);           break;
            case TK_R:      wantTick = actionCentrePan(w, player);          break;
            case TK_D:      wantTick = actionDrop(w, player, Dir::None);    break;
            case TK_KP_5:
            case TK_SPACE:  wantTick = true;                                break;

            default: {
                Dir d = keyToDir(key);
                if (d != Dir::None) {
                    // int ctrl = terminal_check(TK_CTRL);
                    // int shift = terminal_check(TK_SHIFT);
                    // int alt = terminal_check(TK_ALT);
                    wantTick = actionContextMove(w, player, d);
                }
                break; }
        }

        if (wantTick) {
            w.tick();
            wantTick = false;
        }
    }
}



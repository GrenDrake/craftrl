#include <sstream>
#include "lodepng.h"
#include "world.h"

Dir getDir(World &w, const std::string &reason);
void doCrafting(World &w, Actor *player);
bool actionMove(World &w, Actor *player, const Command &command, bool silent);


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



bool actionAttack(World &w, Actor *player, const Command &command, bool silent) {
    Dir dir = command.dir;
    if (dir == Dir::None) {
        dir = getDir(w, "Attack");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    Point dest = player->pos.shift(dir);
    const Tile &tile = w.at(dest);

    if (tile.actor) {
        if (tile.actor->def.type == TYPE_PLANT) {
            w.addLogMsg("Broke " + tile.actor->def.name + '.');
            Actor *actor = tile.actor;
            w.moveActor(actor, nowhere);
            makeLootAt(w, actor->def.loot, dest);
            delete actor;
            return true;
        } else if (tile.actor->def.type == TYPE_ANIMAL) {
            w.addLogMsg("Killed " + tile.actor->def.name + '.');
            Actor *actor = tile.actor;
            w.moveActor(actor, nowhere);
            makeLootAt(w, actor->def.loot, dest);
            delete actor;
            return true;
        } else {
            w.addLogMsg("Can't attack that.");
            return false;
        }
    }

    const TileDef &td = w.getTileDef(tile.terrain);
    if (td.breakTo >= 0) {
        w.setTerrain(dest, td.breakTo);
        w.addLogMsg("Broken.");
        makeLootAt(w, td.loot, dest);
    } else {
        if (!silent) w.addLogMsg("Nothing to attack.");
        return false;
    }
    return true;
}


bool actionCentrePan(World &w, Actor *player, const Command &command, bool silent) {
    const int viewWidth = 50;
    const int viewHeight = 25;

    Point dest(player->pos.x - viewWidth / 2, player->pos.y - viewHeight / 2);
    w.setCamera(dest);
    return false;
}


bool actionContextMove(World &w, Actor *player, const Command &command, bool silent) {
    Dir dir = command.dir;
    if (dir == Dir::None) {
        dir = getDir(w, "Move");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }
    Command newCommand = { CMD_NONE, dir };

    Point dest = player->pos.shift(dir);
    if (!w.valid(dest)) return false;

    const Tile &tile = w.at(dest);
    if (tile.actor) return actionTalkActor(w, player, newCommand, true);

    const TileDef &tdef = w.getTileDef(tile.terrain);
    if (tdef.solid) return actionDo(w, player, newCommand, true);
    return actionMove(w, player, newCommand, true);
}


bool actionCraft(World &w, Actor *player, const Command &command, bool silent) {
    doCrafting(w, player);
    return false;
}


bool actionDo(World &w, Actor *player, const Command &command, bool silent) {
    Dir dir = command.dir;
    if (dir == Dir::None) {
        dir = getDir(w, "Do");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    Point dest = player->pos.shift(dir);
    const Tile &tile = w.at(dest);

    if (tile.actor) {
        w.addLogMsg(tile.actor->def.name + " is in the way.");
        return false;
    }

    const TileDef &td = w.getTileDef(tile.terrain);
    if (td.doorTo >= 0) {
        w.setTerrain(dest, td.doorTo);
        if (!silent) w.addLogMsg("Done.");
        return true;
    }
    if (!silent) w.addLogMsg("Nothing to do.");
    return false;
}


bool actionDrop(World &w, Actor *player, const Command &command, bool silent) {
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


bool actionDumpMap(World &w, Actor *player, const Command &command, bool silent) {
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


bool actionMove(World &w, Actor *player, const Command &command, bool silent) {
    Dir dir = command.dir;
    if (dir == Dir::None) {
        dir = getDir(w, "Move");
    }
    if (dir == Dir::None) {
        w.addLogMsg("Canceled.");
        return false;
    }

    if (!w.tryMoveActor(player, dir, false)) {
        w.addLogMsg("Blocked.");
        return false;
    } else {
        const Tile &tile = w.at(player->pos);
        if (tile.item) {
            actionTake(w, player, command, true);
        }
        shiftCameraForMove(w, player);
        return true;
    }
    return false;
}


bool actionNextSelect(World &w, Actor *player, const Command &command, bool silent) {
    if (w.selection < player->inventory.size() - 1) ++w.selection;
    return false;
}


bool actionPan(World &w, Actor *player, const Command &command, bool silent) {
    Dir dir = command.dir;
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


bool actionPrevSelect(World &w, Actor *player, const Command &command, bool silent) {
    if (w.selection > 0) --w.selection;
    return false;
}


bool actionQuit(World &w, Actor *player, const Command &command, bool silent) {
    w.wantsToQuit = true;
    actionSavegame(w, player, command, silent);
    return false;
}


bool actionSavegame(World &w, Actor *player, const Command &command, bool silent) {
    if (w.savegame("game.sav")) {
        w.addLogMsg("Game saved.");
    } else {
        w.addLogMsg("Save failed.");
    }
    return false;
}


bool actionTake(World &w, Actor *player, const Command &command, bool silent) {
    Item *item = w.at(player->pos).item;
    if (!item) {
        w.addLogMsg("Nothing to take.");
        return false;
    }

    if (player->inventory.add(&item->def)) {
        w.moveItem(item, nowhere);
        w.addLogMsg("Took " + item->def.name + ".");
        delete item;
        return true;
    }
    return false;
}


bool actionTalkActor(World &w, Actor *player, const Command &command, bool silent) {
    Dir dir = command.dir;
    if (dir == Dir::None) {
        dir = getDir(w, "Talk");
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
        switch (tile.actor->def.type) {
            case TYPE_VILLAGER:
                s << tile.actor->def.name << ": \"Hello!\"";
                w.addLogMsg(s.str());
                return false;
            default:
                w.addLogMsg("Can't talk to that!");
                return false;
        }
        return true;
    } else {
        if (!silent) w.addLogMsg("No one to talk to!");
        return false;
    }
}


bool actionUse(World &w, Actor *player, const Command &command, bool silent) {
    if (w.selection < 0 || w.selection >= player->inventory.size()) {
        w.addLogMsg("Nothing to drop.");
        return false;
    }

    const ItemDef *def = player->inventory.mContents[w.selection].def;
    if (def->seedFor >= 0) {
        Dir d = getDir(w, "Plant");
        if (d == Dir::None) {
            w.addLogMsg("Canceled.");
            return false;
        }
        Point dest = player->pos.shift(d);
        const Tile &t = w.at(dest);
        const TileDef &td = w.getTileDef(t.terrain);
        if (!td.ground || t.actor || t.item) {
            w.addLogMsg("The space isn't clear.");
            return false;
        }
        const ActorDef &actorDef = w.getActorDef(def->seedFor);
        player->inventory.remove(def);
        if (actorDef.ident < 0) return false;
        Actor *actor = new Actor(actorDef);
        w.moveActor(actor, dest);
        return true;
    } else if (def->constructs >= 0) {
        Dir d = getDir(w, "Construct");
        if (d == Dir::None) {
            w.addLogMsg("Canceled.");
            return false;
        }
        Point dest = player->pos.shift(d);
        const Tile &t = w.at(dest);
        const TileDef &td = w.getTileDef(t.terrain);
        if (!td.ground || t.actor || t.item) {
            w.addLogMsg("The space isn't clear.");
            return false;
        }
        w.setTerrain(dest, def->constructs);
        player->inventory.remove(def);
        return true;
    } else {
        if (!silent) w.addLogMsg("That's not something you can use.");
        return false;
    }
}


bool actionWait(World &w, Actor *player, const Command &command, bool silent) {
    return true;
}



bool actionSelectEnd(World &w, Actor *player, const Command &command, bool silent) {
    if (player->inventory.size() > 0) {
        w.selection = player->inventory.size() - 1;
    } else {
        w.selection = 0;
    }
    return false;
}

bool actionSelectHome(World &w, Actor *player, const Command &command, bool silent) {
    w.selection = 0;
    return false;
}

bool actionSelectPagedown(World &w, Actor *player, const Command &command, bool silent) {
    if (player->inventory.size() == 0) {
        w.selection = 0;
        return false;
    }

    w.selection += 10;
    if (w.selection >= player->inventory.size()) {
        w.selection = player->inventory.size() - 1;
    }
    return false;
}

bool actionSelectPageup(World &w, Actor *player, const Command &command, bool silent) {
    w.selection -= 10;
    if (w.selection < 0) w.selection = 0;
    return false;
}

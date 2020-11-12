#include <sstream>
#include "world.h"

Dir getDir(World &w, const std::string &reason);
void doCrafting(World &w, Actor *player, unsigned craftingStation);
void doTrading(World &w, Actor *left, Actor *right);
bool actionMove(World &w, Actor *player, const Command &command, bool silent);
void viewLog(World &w);


void makeLootAt(World &w, const LootTable *table, const Point &where, bool showMessages) {
    if (!table || table->mRows.empty()) return;

    Inventory inv;
    for (const LootRow &row : table->mRows) {
        if (row.ident < 0 || row.chance < 0 || row.max < 0) continue;
        const ItemDef &def = w.getItemDef(row.ident);
        if (def.ident < 0) continue;
        int chance = w.getRandom().next32() % 100;
        int qty = w.getRandom().between(row.min, row.max);
        if (chance < row.chance) inv.add(&def, qty);
    }

    std::stringstream s;
    for (InventoryRow &row : inv.mContents) {
        int realDropped = 0;
        for (int i = 0; i < row.qty; ++i ) {
            Point dest = w.findDropSpace(where);
            if (w.valid(dest)) {
                Item *item = new Item(*row.def);
                if (!w.moveItem(item, dest)) {
                    delete item;
                } else {
                    ++realDropped;
                }
            }
        }
        row.qty = realDropped;
    }

    inv.cleanup();
    if (inv.mContents.empty()) return;
    s << " Dropped";
    const unsigned invSize = inv.mContents.size();
    for (unsigned i = 0; i < invSize; ++i) {
        if (i != 0 && invSize > 2) s << ",";
        if (i == invSize - 1 && invSize > 1) s << " and";
        const InventoryRow &row = inv.mContents[i];
        if (row.qty > 1) {
            s << ' ' << row.qty << ' ' << row.def->plural;
        } else {
            s << " a " << row.def->name;
        }
    }
    s << '.';
    if (showMessages) w.appendLogMsg(s.str());
}



void shiftCameraForMove(World &w, Actor *player) {
    const int screenWidth = 80;
    const int screenHeight = 25;
    const int sidebarWidth = 30;
    const int logHeight = 3;
    const int viewWidth = (screenWidth - sidebarWidth - 1) / 2;
    const int viewHeight = screenHeight - logHeight - 1;
    Point dest = player->pos;
    Point p = w.getCamera();
    if (dest.x < p.x + 5) p.x -= 5;
    if (dest.y < p.y + 5)  p.y -= 5;
    if (dest.x > p.x + viewWidth - 5) p.x += 5;
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
        if (tile.actor->def.type != TYPE_VILLAGER) {
            w.doDamage(player, tile.actor);
            return true;
        } else {
            w.addLogMsg("Can't attack that.");
            return false;
        }
    }

    if (tile.building > 0) {
        const TileDef &td = w.getTileDef(tile.building);
        if (td.breakTo >= 0) {
            w.setBuilding(dest, 0);
            w.addLogMsg("Broken.");
            makeLootAt(w, td.loot, dest, true);
            return true;
        }
    }
    if (!silent) w.addLogMsg("Nothing to attack.");
    return false;
}


bool actionCentrePan(World &w, Actor *player, const Command &command, bool silent) {
    const int viewWidth = 25;
    const int viewHeight = 25;

    Point dest(player->pos.x - viewWidth / 2, player->pos.y - viewHeight / 2);
    w.setCamera(dest);
    return false;
}


bool actionClearRoom(World &w, Actor *player, const Command &command, bool silent) {
    if (!w.valid(player->pos) || !w.at(player->pos).room) {
        w.addLogMsg("Not in a room.");
        return false;
    }

    Room *room = w.at(player->pos).room;
    const RoomDef *def = room->def;
    w.removeRoom(room);
    delete room;
    if (!def)   w.addLogMsg("Cleared room (no RoomDef found).");
    else        w.addLogMsg("Cleared " + def->name + ".");

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

    const TileDef &bdef = w.getTileDef(tile.building);
    if (bdef.grantsCrafting) {
        doCrafting(w, player, bdef.grantsCrafting);
        return false;
    }
    if (bdef.solid) return actionDo(w, player, newCommand, true);

    const TileDef &tdef = w.getTileDef(tile.terrain);
    if (tdef.solid) return false;
    return actionMove(w, player, newCommand, true);
}


bool actionCraft(World &w, Actor *player, const Command &command, bool silent) {
    doCrafting(w, player, 0);
    return false;
}


bool actionDebug(World &w, Actor *player, const Command &command, bool silent) {
    doDebug(w, player);
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
        w.addLogMsg(tile.actor->getName() + " is in the way.");
        return false;
    }

    if(tile.building > 0) {
        const TileDef &td = w.getTileDef(tile.building);
        if (td.doorTo >= 0) {
            w.setBuilding(dest, td.doorTo);
            if (!silent) w.addLogMsg("Done.");
            return true;
        }
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


bool actionMakeRoom(World &w, Actor *player, const Command &command, bool silent) {
    if (w.at(player->pos).room) {
        w.addLogMsg("Already in a room.");
        return false;
    }
    if (!w.valid(player->pos) || w.getTileDef(w.at(player->pos).terrain).solid) {
        w.addLogMsg("Invalid room position.");
        return false;
    }

    if (!w.createRoom(player->pos)) {
        w.addLogMsg("Cannot make room here.");
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
    ui_MessageBox_Instant("Saving the game...");
    actionSavegame(w, player, command, silent);
    return false;
}


bool actionSavegame(World &w, Actor *player, const Command &command, bool silent) {
    ui_MessageBox_Instant("Saving the game...");
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
        w.removeItem(item);
        w.addLogMsg("Took " + item->getName(false) + ".");
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
                s << upperFirst(tile.actor->getName()) << ": \"Hello!\"";
                w.addLogMsg(s.str());
                return false;
            case TYPE_INVENTORY:
                doTrading(w, player, tile.actor);
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
        if (!td.ground || t.building > 0 || t.actor || t.item) {
            w.addLogMsg("The space isn't clear.");
            return false;
        }
        const ActorDef &actorDef = w.getActorDef(def->seedFor);
        player->inventory.remove(def);
        if (actorDef.ident < 0) return false;
        Actor *actor = new Actor(actorDef);
        actor->reset();
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
        if (def->makeFloor) {
            if (td.ident == def->constructs) {
                w.addLogMsg("That's already there.");
                return false;
            }
            w.setTerrain(dest, def->constructs);
        } else if (t.building == 0) {
            w.setBuilding(dest, def->constructs);
        } else {
            w.addLogMsg("There's already something there.");
            return false;
        }
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

bool actionSortInvByName(World &w, Actor *player, const Command &command, bool silent) {
    player->inventory.sort(SORT_NAME);
    return false;
}

bool actionViewLog(World &w, Actor *player, const Command &command, bool silent) {
    viewLog(w);
    return false;
}
#include <sstream>
#include <sstream>
#include <string>
#include <vector>
#include "world.h"

struct DebugCommand {
    std::string command;
    void (*handler)(World&, Actor*, const std::vector<std::string>&);
    unsigned arguments;
};

void debugDumpMap(World &w, Actor *player, const std::vector<std::string> &command);
void debugGive(World &w, Actor *player, const std::vector<std::string> &command);
void debugHelp(World &w, Actor *player, const std::vector<std::string> &command);
void debugInfo(World &w, Actor *player, const std::vector<std::string> &command);
void debugKill(World &w, Actor *player, const std::vector<std::string> &command);
void debugReset(World &w, Actor *player, const std::vector<std::string> &command);
void debugSpawn(World &w, Actor *player, const std::vector<std::string> &command);
void debugTeleport(World &w, Actor *player, const std::vector<std::string> &command);

Dir strToDir(const std::string &s) {
    if (s == "north")     return Dir::North;
    if (s == "south")     return Dir::South;
    if (s == "east")      return Dir::East;
    if (s == "west")      return Dir::West;
    if (s == "southeast") return Dir::Southeast;
    if (s == "southwest") return Dir::Southwest;
    if (s == "northeast") return Dir::Northeast;
    if (s == "northwest") return Dir::Northwest;

    if (s == "n")         return Dir::North;
    if (s == "s")         return Dir::South;
    if (s == "e")         return Dir::East;
    if (s == "w")         return Dir::West;
    if (s == "se")        return Dir::Southeast;
    if (s == "sw")        return Dir::Southwest;
    if (s == "ne")        return Dir::Northeast;
    if (s == "nw")        return Dir::Northwest;

    return Dir::None;
}

DebugCommand debugCommands[] = {
    {   "dumpmap",  debugDumpMap,   0  },
    {   "give",     debugGive,      2  },
    {   "help",     debugHelp,      0  },
    {   "info",     debugInfo,      1  },
    {   "kill",     debugKill,      1  },
    {   "reset",    debugReset,     1  },
    {   "spawn",    debugSpawn,     1  },
    {   "teleport", debugTeleport,  2  },
    {   "", nullptr }
};

void doDebug(World &w, Actor *player) {
    std::string fullCommand;
    if (!ui_prompt("Debug", "Enter Command", fullCommand)) {
        return;
    }

    std::vector<std::string> parts = explode(fullCommand);
    if (parts.empty()) return;

    for (const DebugCommand &cmd : debugCommands) {
        if (cmd.command == parts[0]) {
            if (cmd.arguments != parts.size() - 1) {
                w.addLogMsg("Wrong argument count for command " + parts[0] + ".");
                return;
            } else {
                cmd.handler(w, player, parts);
                return;
            }
        }
    }
    w.addLogMsg("Unknown debug command " + parts[0] + ". \"help\" to get list of commands.");
}

void dumpActorMap(World &w);
void dumpPlantMap(World &w);
void dumpTerrainMap(World &w);

void debugDumpMap(World &w, Actor *player, const std::vector<std::string> &command) {
    ui_MessageBox_Instant("Dumping map images...");
    dumpActorMap(w);
    dumpPlantMap(w);
    dumpTerrainMap(w);
    w.addLogMsg("Maps dumped to write directory.");
}

void debugGive(World &w, Actor *player, const std::vector<std::string> &command) {
    int qty = 0, ident = -1;
    if (!strToInt(command[1], qty)) {
        w.addLogMsg("quantity must be number.");
        return;
    }
    if (qty <= 0) {
        w.addLogMsg("quantity must greater than zero.");
        return;
    }
    if (!strToInt(command[2], ident)) {
        w.addLogMsg("ItemDef ident must be number.");
        return;
    }

    const ItemDef &def = w.getItemDef(ident);
    if (def.ident < 0) {
        w.addLogMsg("Invalid ItemDef ident.");
        return;
    }

    player->inventory.add(&def, qty);
    w.addLogMsg("Created " + std::to_string(qty) + " of " + def.name + ".");
}

void debugHelp(World &w, Actor *player, const std::vector<std::string> &command) {
    std::stringstream msg;
    msg << "Valid commands:";
    for (const DebugCommand &cmd : debugCommands) {
        msg << ' ' << cmd.command;
    }
    msg << '.';
    w.addLogMsg(msg.str());
}

void debugInfo(World &w, Actor *player, const std::vector<std::string> &command) {
    Dir d = strToDir(command[1]);
    if (d == Dir::None) {
        w.addLogMsg("Not a valid direction.");
        return;
    }
    Point dest = player->pos.shift(d);
    const Tile &tile = w.at(dest);
    if (!tile.actor) {
        w.addLogMsg("No actor present.");
        return;
    }

    Actor *a = tile.actor;
    std::stringstream s;
    s << a->getName() << ": H" << a->health << '/' << a->def.health << " F" << a->faction << " A" << a->age << " T" << a->type << " INV" << a->inventory.size();
    w.addLogMsg(s.str());
}

void debugKill(World &w, Actor *player, const std::vector<std::string> &command) {
    int radius = -1;
    if (!strToInt(command[1], radius)) {
        w.addLogMsg("Kill radius must be number.");
        return;
    }
    if (radius <= 0) {
        w.addLogMsg("kill radius must greater than zero.");
        return;
    }

    for (int y = player->pos.y - radius; y <= player->pos.y + radius; ++y) {
        for (int x = player->pos.x - radius; x <= player->pos.x + radius; ++x) {
            const Tile &tile = w.at(Point(x, y));
            if (tile.actor && tile.actor != player) {
                w.addLogMsg("Killed " + tile.actor->getName() + ".");
                tile.actor->health = 0;
                makeLootAt(w, tile.actor->def.loot, tile.actor->pos, true);
                w.moveActor(tile.actor, Point(0, 0));
            }
        }
    }
}

void debugReset(World &w, Actor *player, const std::vector<std::string> &command) {
    Dir d = strToDir(command[1]);
    if (d == Dir::None) {
        w.addLogMsg("Not a valid direction.");
        return;
    }
    Point dest = player->pos.shift(d);
    const Tile &tile = w.at(dest);
    if (!tile.actor) {
        w.addLogMsg("No actor present.");
        return;
    }

    tile.actor->reset();
    w.addLogMsg(tile.actor->getName() + " reset.");
}

void debugSpawn(World &w, Actor *player, const std::vector<std::string> &command) {
    Point p = player->pos.shift(Dir::North);
    if (!w.valid(p) || w.at(p).actor) {
        w.addLogMsg("Invalid spawn point.");
        return;
    }

    int ident = -1;
    if (!strToInt(command[1], ident)) {
        w.addLogMsg("ActorDef ident must be number.");
        return;
    }
    const ActorDef &def = w.getActorDef(ident);
    if (def.ident < 0) {
        w.addLogMsg("Invalid ActorDef ident.");
        return;
    }

    Actor *actor = new Actor(def);
    actor->reset();
    w.moveActor(actor, p);
    w.addLogMsg("Spawned " + def.name + " at " + p.toString() + ".");
}

void debugTeleport(World &w, Actor *player, const std::vector<std::string> &command) {
    int x = -1, y = -1;
    if (!strToInt(command[1], x)) {
        w.addLogMsg("X coord radius must be number.");
        return;
    }
    if (!strToInt(command[2], y)) {
        w.addLogMsg("y coord radius must be number.");
        return;
    }
    Point dest(x, y);

    if (!w.valid(dest)) {
        w.addLogMsg("Not a valid map position.");
        return;
    }
    const Tile &tile = w.at(dest);
    if (tile.actor) {
        w.addLogMsg("Position already occupied.");
        return;
    }
    if (w.getTileDef(tile.terrain).solid) {
        w.addLogMsg("Position not passable.");
        return;
    }

    w.moveActor(player, dest);
    actionCentrePan(w, player, Command{}, true);
}

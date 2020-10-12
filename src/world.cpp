#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <physfs.h>
#include "world.h"


const LogMessage World::BAD_LOGMESSAGE;
const Tile World::BAD_TILE(-1);
const ActorDef World::BAD_ACTORDEF = { -1 };
const ItemDef World::BAD_ITEMDEF = { -1 };
const TileDef World::BAD_TILEDEF = { -1 };
const RecipeDef World::BAD_RECIPEDEF = { -1 };
const RoomDef World::BAD_ROOMDEF = { -1 };


bool Inventory::add(const ItemDef *def, int qty) {
    for (unsigned i = 0; i < mContents.size(); ++i) {
        if (mContents[i].def == def) {
            mContents[i].qty += qty;
            return true;
        }
    }

    mContents.push_back(InventoryRow{qty, def});
    return true;
}

int Inventory::qty(const ItemDef *def) const {
    for (unsigned i = 0; i < mContents.size(); ++i) {
        if (mContents[i].def == def) {
            return mContents[i].qty;
        }
    }
    return 0;
}

bool Inventory::remove(const ItemDef *def, int qty) {
    auto iter = mContents.begin();
    while (iter != mContents.end()) {
        if (iter->def == def) {
            if (iter->qty > qty) {
                iter->qty -= qty;
                return true;
            } else if (iter->qty == qty) {
                mContents.erase(iter);
                return true;
            } else {
                return false;
            }
        }
        ++iter;
    }

    return false;
}

void Inventory::cleanup() {
    auto iter = mContents.begin();
    while (iter != mContents.end()) {
        if (iter->qty <= 0) {
            iter = mContents.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool sortByName(const InventoryRow &lhs, const InventoryRow &rhs) {
    return lhs.def->name < rhs.def->name;
}
void Inventory::sort(int sortType) {
    std::sort(mContents.begin(), mContents.end(), sortByName);
}


std::string Actor::getName() const {
    std::string name = "the " + def.name;
    if (health < def.health && health > 0) {
        int percent = health * 100 / def.health;
        name += " (" + std::to_string(percent) + "%)";
    }
    return name;
}

void Actor::reset() {
    health = def.health;
}

std::string Item::getName(bool forPural) const {
    if (forPural) return def.plural;
    return "a " + def.name;
}


World::World()
: inProgress(false), mTiles(nullptr), mPlayer(nullptr), turn(0), day(1), hour(12), minute(0) {
}

World::~World() {
    deallocMap();
    for (ActorDef &def : mActorDefs) {
        if (def.loot) delete def.loot;
    }
    for (TileDef &def : mTileDefs) {
        if (def.loot) delete def.loot;
    }

}

void World::allocMap(int width, int height) {
    deallocMap();
    mWidth = width;
    mHeight = height;
    mTiles = new Tile[width * height];
    turn = 0;
}

void World::deallocMap() {
    if (!mTiles) return;

    for (int i = 0; i < mWidth * mHeight; ++i) {
        delete mTiles->item;
    }
    delete[] mTiles;
    mTiles = nullptr;

    for (Actor *actor : mActors) {
        if (!actor) logger_log("deallocMap: Found null actor in actor list.");
        else        delete actor;
    }
    for (Room *room : mRooms) {
        if (!room)  logger_log("deallocMap: Found null room in room list.");
        else        delete room;
    }
    mActors.clear();
    mLog.clear();
    mPlayer = nullptr;
}

bool World::valid(const Point &p) const {
    if (p.x < 0 || p.y < 0) return false;
    if (p.x >= mWidth || p.y >= mHeight) return false;
    return true;
}



const Point& World::getCamera() const {
    return mCamera;
}

void World::setCamera(const Point &to) {
    mCamera = to;
}



Point World::findDropSpace(const Point &near) const {
    if (at(near).item == nullptr) return near;
    for (int i = 0; i < 8; ++i) {
        Dir d = static_cast<Dir>(i);
        Point p = near.shift(d);
        const Tile &t = at(p);
        if (getTileDef(t.terrain).solid) continue;
        if (t.item) continue;
        if (t.actor && t.actor->def.type == TYPE_PLANT) continue;
        return p;
    }
    return nowhere;
}


const Tile& World::at(const Point &p) const {
    if (!valid(p)) return BAD_TILE;
    int c = p.x + p.y * mWidth;
    return mTiles[c];
}


void World::setActor(const Point &pos, Actor *toActor) {
    if (!valid(pos)) return;
    int c = pos.x + pos.y * mWidth;
    mTiles[c].actor = toActor;
}

void World::setItem(const Point &pos, Item *toItem) {
    if (!valid(pos)) return;
    int c = pos.x + pos.y * mWidth;
    mTiles[c].item = toItem;
}

void World::setTerrain(const Point &pos, int toTile) {
    if (!valid(pos)) return;
    int c = pos.x + pos.y * mWidth;
    mTiles[c].terrain = toTile;

    // check for neccesary room updates
    if (mTiles[c].room) {
        updateRoom(mTiles[c].room);
    }
}

bool World::moveActor(Actor *actor, const Point &to) {
    if (!actor || (valid(to) && at(to).actor)) return false; // space already occupied

    if (to.x == 0 && to.y == 0) {
        setActor(actor->pos, nullptr);
        actor->pos = to;
        return true;
    }

    if (valid(actor->pos) && at(actor->pos).actor == actor) {
        if (!valid(to)) {
            logger_log("moveActor: Moving actor (" + actor->def.name + ") at " + actor->pos.toString() + " to invalid position.");
            removeActor(actor);
            return true;
        } else {
            setActor(actor->pos, nullptr);
        }
    } else {
        mActors.push_back(actor);
    }

    if (valid(to)) {
        setActor(to, actor);
        if (actor->type == 1) mPlayer = actor;
    }
    actor->pos = to;
    return true;
}

const Actor* World::getPlayer() const {
    return mPlayer;
}

Actor* World::getPlayer() {
    return mPlayer;
}

bool World::moveItem(Item *item, const Point &to) {
    if (!item || (valid(to) && at(to).item)) return false; // space already occupied

    if (valid(item->pos) && at(item->pos).item == item) {
        if (!valid(to)) {
            logger_log("moveItem: Moving item (" + item->def.name + ") at " + item->pos.toString() + " to invalid position.");
            removeItem(item);
        } else {
            setItem(item->pos, nullptr);
        }
    }

    if (valid(to)) {
        setItem(to, item);
    }
    item->pos = to;
    return true;
}

void World::removeActor(Actor *actor) {
    if (at(actor->pos).actor != actor) return;
    setActor(actor->pos, nullptr);
    auto iter = mActors.begin();
    while (iter != mActors.end()) {
        if (*iter == actor) {
            iter = mActors.erase(iter);
        } else {
            ++iter;
        }
    }
}

void World::removeItem(Item *item) {
    if (at(item->pos).item != item) return;
    setItem(item->pos, nullptr);
}

std::vector<Point> World::findRoomExtents(const Point &pos) const {
    std::vector<Point> result;
    std::vector<Point> todo;
    todo.push_back(pos);
    while (!todo.empty()) {
        Point pos = todo.back();
        todo.pop_back();
        bool alreadyDone = false;
        for (const Point &p : result) {
            if (p == pos) {
                alreadyDone = true;
                break;
            }
        }
        if (alreadyDone) continue;
        result.push_back(pos);

        Dir d = Dir::North;
        do {
            Point dest = pos.shift(d);
            if (valid(dest) && !getTileDef(at(dest).terrain).isWall) {
                todo.push_back(dest);
            }
            d = rotate45(d);
        } while (d != Dir::North);

        if (result.size() > 100) {
            return std::vector<Point>{};
        }
    }
    return result;
}

void World::addRoom(Room *room) {
    if (!room) return;
    mRooms.push_back(room);
    for (const Point &pos : room->points) {
        if (!valid(pos)) continue;
        int c = pos.x + pos.y * mWidth;
        mTiles[c].room = room;
    }
}

void World::removeRoom(Room *room) {
    for (const Point &pos : room->points) {
        int c = pos.x + pos.y * mWidth;
        mTiles[c].room = nullptr;
    }

    auto iter = mRooms.begin();
    while (iter != mRooms.end()) {
        if (*iter == room) {
            iter = mRooms.erase(iter);
        } else {
            ++iter;
        }
    }
}

void World::updateRoom(Room *room) {
    int score = 0;
    const RoomDef *theDef = nullptr;

    for (const RoomDef &def : mRoomDefs) {
        if (def.value <= score) continue;
        bool isMatch = true;
        for (int tile : def.requirements) {
            bool met = false;
            for (const Point &p : room->points) {
                if (at(p).terrain == tile) {
                    met = true;
                    break;
                }
            }
            if (!met) {
                isMatch = false;
                break;
            }
        }

        if (isMatch) {
            score = def.value;
            theDef = &def;
        }
    }

    if (theDef == nullptr) {
        theDef = &getRoomDef(0);
    }
    if (room->def == nullptr)   addLogMsg("Created " + theDef->name + ".");
    else                        addLogMsg("The " + room->def->name + " becomes a " + theDef->name + ".");
    room->def = theDef;
    room->type = theDef->ident;
}


Point World::findItemNearest(const Point &to, int itemIdent, int radius) const {
    Point result = nowhere;
    double distance = 999999.0;
    for (int y = to.y - radius; y <= to.y + radius; ++y) {
        for (int x = to.x - radius; x <= to.x + radius; ++x) {
            Point here(x, y);
            const Tile &tile = at(here);
            if (tile.item && tile.item->def.ident == itemIdent) {
                double myDist = here.distance(to);
                if (myDist < distance) {
                    result = here;
                    distance = myDist;
                }
            }
        }
    }
    return result;
}

Point World::findActorNearest(const Point &to, int notOfFaction, int radius) const {
    Point result = nowhere;
    double distance = 999999.0;
    for (int y = to.y - radius; y <= to.y + radius; ++y) {
        for (int x = to.x - radius; x <= to.x + radius; ++x) {
            Point here(x, y);
            const Tile &tile = at(here);
            if (!tile.actor || tile.actor->def.type == TYPE_PLANT) continue;
            if (notOfFaction < 0 || tile.actor->faction != notOfFaction) {
                double myDist = here.distance(to);
                if (myDist < distance) {
                    result = here;
                    distance = myDist;
                }
            }
        }
    }
    return result;
}

void World::doDamage(Actor *attacker, Actor *victim) {
    if (!attacker || !victim) {
        logger_log("doDamage: found null actor.");
        return;
    }

    bool showMsgs = mPlayer->pos.distance(victim->pos) < 10;
    int damage = attacker->def.baseDamage;
    victim->health -= damage;

    std::stringstream msg;
    msg << upperFirst(attacker->getName()) << " does " << damage << " damage to " << victim->getName() << ".";
    if (showMsgs) addLogMsg(msg.str());

    if (victim->health <= 0) {
        std::stringstream deathMsg;
        deathMsg << ' ' << upperFirst(victim->getName());
        if (victim->def.type == TYPE_PLANT) deathMsg << " breaks.";
        else                                deathMsg << " dies.";
        if (showMsgs) appendLogMsg(deathMsg.str());
        Point oldPos = victim->pos;
        moveActor(victim, Point(0, 0));
        if (victim->def.loot) {
            makeLootAt(*this, victim->def.loot, oldPos, showMsgs);
        }
    }

}

void World::addLogMsg(const std::string &msg) {
    mLog.push_back(LogMessage{msg});
}

void World::appendLogMsg(const std::string &msg) {
    if (mLog.empty()) addLogMsg(msg);
    mLog.back().msg += msg;
}

int World::getLogCount() const {
    return mLog.size();
}

const LogMessage& World::getLogMsg(int index) const {
    if (index < 0 || index >= static_cast<int>(mLog.size())) return BAD_LOGMESSAGE;
    return mLog[mLog.size() - index - 1];
}




void World::addActorDef(const ActorDef &ad) {
    mActorDefs.push_back(ad);
}

const ActorDef& World::getActorDef(int ident) const {
    for (const ActorDef &ad : mActorDefs) {
        if (ad.ident == ident) return ad;
    }
    return BAD_ACTORDEF;
}

void World::addItemDef(const ItemDef &td) {
    mItemDefs.push_back(td);
}

const ItemDef& World::getItemDef(int ident) const {
    for (const ItemDef &td : mItemDefs) {
        if (td.ident == ident) return td;
    }
    return BAD_ITEMDEF;
}

void World::addTileDef(const TileDef &td) {
    mTileDefs.push_back(td);
}

const TileDef& World::getTileDef(int ident) const {
    for (const TileDef &td : mTileDefs) {
        if (td.ident == ident) return td;
    }
    return BAD_TILEDEF;
}

void World::addRecipeDef(const RecipeDef &td) {
    mRecipeDefs.push_back(td);
}

std::vector<const RecipeDef*> World::getRecipeList(unsigned stations) const {
    std::vector<const RecipeDef*> list;
    for (const RecipeDef &td : mRecipeDefs) {
        if ((td.craftingStation & stations) == td.craftingStation) {
            list.push_back(&td);
        }
    }
    return list;
}

void World::addRoomDef(const RoomDef &rd) {
    mRoomDefs.push_back(rd);
}

const RoomDef& World::getRoomDef(int ident) const {
    for (const RoomDef &td : mRoomDefs) {
        if (td.ident == ident) return td;
    }
    return BAD_ROOMDEF;
}

bool World::tryMoveActor(Actor *actor, Dir baseDir, bool allowSidestep) {
    Point dest = actor->pos.shift(baseDir);

    const Tile &tile = at(dest);
    if (valid(dest) && !tile.actor && !getTileDef(tile.terrain).solid) {
        moveActor(actor, dest);
        return true;
    }

    if (!allowSidestep) return false;
    if (!tryMoveActor(actor, rotate45(baseDir), false)) {
        return tryMoveActor(actor, unrotate45(baseDir), false);
    }

    return false;
}

void World::tick() {
    ++turn;
    minute += 3;
    while (minute >= 60) {
        minute -= 60;
        ++hour;
    }
    while (hour >= 24) {
        hour -= 24;
        ++day;
    }

    for (unsigned i = 0; i < mActors.size(); ++i) {
        Actor *actor = mActors[i];
        if (mRandom.next32() % 1000 >= static_cast<unsigned>(actor->def.moveChance)) continue;

        ++actor->age;

        if (actor->def.type == TYPE_VILLAGER) {
            Dir dir = static_cast<Dir>(mRandom.next32() % 8);
            tryMoveActor(actor, dir);

        } else if (actor->def.type == TYPE_MONSTER) {
            Point victimPos = findActorNearest(actor->pos, actor->faction, 8);
            if (valid(victimPos)) {
                if (victimPos.distance(actor->pos) < 2) {
                    const Tile &tile = at(victimPos);
                    doDamage(actor, tile.actor);
                } else {
                    tryMoveActor(actor, actor->pos.directionTo(victimPos));
                }
            } else {
                Dir dir = static_cast<Dir>(mRandom.next32() % 8);
                tryMoveActor(actor, dir);
            }

        } else if (actor->def.type == TYPE_ANIMAL) {
            if (actor->def.foodItem >= 0) {
                Point foodPos = findItemNearest(actor->pos, actor->def.foodItem, 8);
                if (valid(foodPos)) {
                    Dir d = actor->pos.directionTo(foodPos);
                    if (d == Dir::None) {
                        // on food item, eat it
                        const Tile &tile = at(foodPos);
                        Item *item = tile.item;
                        if (item) {
                            removeItem(item);
                            delete item;
                        }
                    } else {
                        // move towards food
                        tryMoveActor(actor, d);
                    }
                    continue;
                }
            }

            Dir dir = static_cast<Dir>(mRandom.next32() % 8);
            tryMoveActor(actor, dir);

        } else if (actor->def.type == TYPE_PLANT) {
            if (actor->def.growTo >= 0 && actor->age >= actor->def.growTime) {
                const ActorDef &def = getActorDef(actor->def.growTo);
                if (def.ident == -1) {
                    actor->age = -9999;
                    logger_log(actor->def.name + " at " + actor->pos.toString() + " has invalid next growth state.");
                } else {
                    Actor *newActor = new Actor(def);
                    newActor->reset();
                    mActors[i] = newActor;
                    newActor->pos = actor->pos;
                    setActor(actor->pos, newActor);
                    delete actor;
                }
            }
        }
    }

    auto iter = mActors.begin();
    while (iter != mActors.end()) {
        if ((*iter)->pos.x == 0 && (*iter)->pos.y == 0) {
            Actor *actor = *iter;
            if (actor->def.type == TYPE_PLAYER) {
                actor->reset();
                Point p;
                do {
                    p.x = mRandom.next32() % mWidth;
                    p.y = mRandom.next32() % mHeight;
                } while (getTileDef(at(p).terrain).solid || at(p).actor);
                moveActor(actor, p);
                addLogMsg("You have died! Respawning...");
                logger_log("tick (info): respawning player at " + p.toString() + ".");
                actionCentrePan(*this, actor, Command{}, true);
                ++iter;
            } else {
                iter = mActors.erase(iter);
                delete actor;
            }
        } else {
            ++iter;
        }
    }

    if (selection >= mPlayer->inventory.size()) {
        selection = mPlayer->inventory.size() - 1;
    }
    if (selection < 0) selection = 0;

}

void World::getTime(int *day, int *hour, int *minute) const {
    *day    = this->day;
    *hour   = this->hour;
    *minute = this->minute;
}

void writeString(PHYSFS_file *out, const std::string &s) {
    for (char c : s) {
        PHYSFS_writeBytes(out, &c, 1);
    }
    char zero  =0;
    PHYSFS_writeBytes(out, &zero, 1);
}

bool World::savegame(const std::string &filename) const {
    logger_log("savegame (info): saving game.");
    PHYSFS_file *out = PHYSFS_openWrite(filename.c_str());
    if (!out) {
        logger_log("savegame: Failed to open save file.");
        return false;
    }

    const unsigned versionNumber = (VER_MAJOR << 16) | VER_MINOR;
    PHYSFS_writeULE32(out, 0x4C5243); // magic number
    PHYSFS_writeULE32(out, versionNumber);
    PHYSFS_writeULE32(out, mWidth);
    PHYSFS_writeULE32(out, mHeight);

    PHYSFS_writeULE32(out, turn);
    PHYSFS_writeULE32(out, day);
    PHYSFS_writeULE32(out, hour);
    PHYSFS_writeULE32(out, minute);


    int itemCount = 0;
    // write tiles
    PHYSFS_writeULE32(out, 0x454C4954);
    for (int i = 0; i < mWidth * mHeight; ++i) {
        PHYSFS_writeULE32(out, mTiles[i].terrain);
        if (mTiles[i].item) ++itemCount;
    }
    // write items on ground
    PHYSFS_writeULE32(out, 0x4D455449);
    PHYSFS_writeULE32(out, itemCount);
    for (int i = 0; i < mWidth * mHeight; ++i) {
        if (!mTiles[i].item) continue;
        const Item *item = mTiles[i].item;
        PHYSFS_writeULE32(out, item->def.ident);
        PHYSFS_writeULE32(out, item->pos.x);
        PHYSFS_writeULE32(out, item->pos.y);
    }
    // write actors
    PHYSFS_writeULE32(out, 0x52544341);
    PHYSFS_writeULE32(out, mActors.size());
    for (const Actor *actor : mActors) {
        PHYSFS_writeULE32(out, actor->def.ident);
        PHYSFS_writeULE32(out, actor->pos.x);
        PHYSFS_writeULE32(out, actor->pos.y);
        PHYSFS_writeULE32(out, actor->age);
        PHYSFS_writeULE32(out, actor->health);
        PHYSFS_writeULE32(out, actor->inventory.size());
        for (const InventoryRow &row : actor->inventory.mContents) {
            PHYSFS_writeULE32(out, row.qty);
            PHYSFS_writeULE32(out, row.def->ident);
        }
    }
    // write rooms
    PHYSFS_writeULE32(out, 0x4D4F4F52);
    PHYSFS_writeULE32(out, mRooms.size());
    for (const Room *room : mRooms) {
        PHYSFS_writeULE32(out, room->type);
        unsigned char size = room->points.size();
        PHYSFS_writeBytes(out, &size, 1);
        for (const Point &p : room->points) {
            PHYSFS_writeULE32(out, p.x);
            PHYSFS_writeULE32(out, p.y);
        }
    }
    // write log
    PHYSFS_writeULE32(out, 0x00474F4C);
    PHYSFS_writeULE32(out, mLog.size());
    for (const LogMessage &msg : mLog) {
        writeString(out, msg.msg);
    }

    PHYSFS_close(out);
    return true;
}

char read8(PHYSFS_File *file) {
    char v;
    if (PHYSFS_readBytes(file, &v, 1) != 1) {
        return -1;
    }
    return v;
}
int read32(PHYSFS_File *file) {
    unsigned v;
    if (!PHYSFS_readULE32(file, &v)) {
        return -1;
    }
    return v;
}
std::string readString(PHYSFS_File *file) {
    std::string s;
    char c = read8(file);
    while (c != 0) {
        s += c;
        c = read8(file);
    }
    return s;
}

bool World::loadgame(const std::string &filename) {
    logger_log("loadgame (info): loading game.");
    PHYSFS_file *inf = PHYSFS_openRead(("/save/" + filename).c_str());
    if (!inf) {
        logger_log("loadgame: Failed to open save file.");
        return false;
    }

    if (read32(inf) != 0x4C5243) {
        logger_log("loadgame: bad magic number.");
        PHYSFS_close(inf);
        return false;
    }

    const unsigned versionNumber = (VER_MAJOR << 16) | VER_MINOR;
    if (read32(inf) != versionNumber) {
        logger_log("loadgame: incompatable save version.");
        PHYSFS_close(inf);
        return false;
    }
    int width = read32(inf);
    int height = read32(inf);
    allocMap(width, height);

    turn    = read32(inf);
    day     = read32(inf);
    hour    = read32(inf);
    minute  = read32(inf);

    // read tiles
    if (read32(inf) != 0x454C4954) {
        logger_log("loadgame: expected start of tile data.");
        PHYSFS_close(inf);
        return false;
    }
    for (int i = 0; i < mWidth * mHeight; ++i) {
        int t = read32(inf);
        mTiles[i].terrain = t;
    }

    // read items on ground
    if (read32(inf) != 0x4D455449) {
        logger_log("loadgame: expected start of item data.");
        PHYSFS_close(inf);
        return false;
    }
    int itemCount = read32(inf);
    for (int i = 0; i < itemCount; ++i) {
        int ident = read32(inf);
        Item *item = new Item(getItemDef(ident));
        int x = read32(inf);
        int y = read32(inf);
        Point pos(x, y);
        moveItem(item, pos);
    }

    // read actors
    if (read32(inf) != 0x52544341) {
        logger_log("loadgame: expected start of actor data.");
        PHYSFS_close(inf);
        return false;
    }
    int actorCount = read32(inf);
    for (int i = 0; i < actorCount; ++i) {
        int ident = read32(inf);
        Actor *actor = new Actor(getActorDef(ident));
        int x = read32(inf);
        int y = read32(inf);
        Point pos(x, y);
        moveActor(actor, pos);
        actor->age = read32(inf);
        actor->health = read32(inf);
        int invCount = read32(inf);
        for (int j = 0; j < invCount; ++j) {
            int qty = read32(inf);
            int itemIdent = read32(inf);
            const ItemDef &idef = getItemDef(itemIdent);
            actor->inventory.add(&idef, qty);
        }
    }
    // read rooms
    if (read32(inf) != 0x4D4F4F52) {
        logger_log("loadgame: expected start of room data.");
        return false;
    }
    int roomCount = read32(inf);
    for (int i = 0; i < roomCount; ++i) {
        Room *room = new Room;
        room->type = read32(inf);
        int pointCount = read8(inf);
        for (int j = 0; j < pointCount; ++j) {
            Point p;
            p.x = read32(inf);
            p.y = read32(inf);
            room->points.push_back(p);
        }
        addRoom(room);
        updateRoom(room);
    }
    // read log
    if (read32(inf) != 0x00474F4C) {
        logger_log("loadgame: expected start of log data.");
        PHYSFS_close(inf);
        return false;
    }
    int logCount = read32(inf);
    for (int i = 0; i < logCount; ++i) {
        std::string msg = readString(inf);
        addLogMsg(msg);
    }

    PHYSFS_close(inf);
    return true;
}


std::ostream& operator<<(std::ostream &out, const Point &p) {
    std::stringstream s;
    s << '(' << p.x << ',' << p.y << ')';
    out << s.str();
    return out;
}

std::string directionName(Dir d) {
    switch(d) {
        case Dir::North:        return "north";     break;
        case Dir::Northeast:    return "northeast"; break;
        case Dir::East:         return "east";      break;
        case Dir::Southeast:    return "southeast"; break;
        case Dir::South:        return "south";     break;
        case Dir::Southwest:    return "southwest"; break;
        case Dir::West:         return "west";      break;
        case Dir::Northwest:    return "northwest"; break;
        case Dir::None:         return "none";      break;
        default: {
            std::stringstream s;
            s << "(bad dir " << static_cast<int>(d) << ')';
            return s.str(); }
    }
}

std::ostream& operator<<(std::ostream &out, const Dir &d) {
    out << directionName(d);
    return out;
}


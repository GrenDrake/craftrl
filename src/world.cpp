#include <fstream>
#include <iostream>
#include <sstream>
#include "world.h"


LogMessage World::BAD_LOGMESSAGE;
Tile World::BAD_TILE(-1);
const ActorDef World::BAD_ACTORDEF = { -1 };
const ItemDef World::BAD_ITEMDEF = { -1 };
const TileDef World::BAD_TILEDEF = { -1 };
const RecipeDef World::BAD_RECIPEDEF = { -1 };


Point Point::shift(Dir dir, int amnt) const {
    switch (dir) {
        case Dir::North:        return Point(x,         y - amnt);
        case Dir::Northeast:    return Point(x + amnt,  y - amnt);
        case Dir::East:         return Point(x + amnt,  y       );
        case Dir::Southeast:    return Point(x + amnt,  y + amnt);
        case Dir::South:        return Point(x,         y + amnt);
        case Dir::Southwest:    return Point(x - amnt,  y + amnt);
        case Dir::West:         return Point(x - amnt,  y       );
        case Dir::Northwest:    return Point(x - amnt,  y - amnt);
        default:                return *this;
    }
}


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


void Actor::reset() {
    health = def.health;
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
        if (!actor) std::cerr << "deallocMap: Found null actor in actor list.\n";
        else        delete actor;
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
        if (at(p).item) continue;
        return p;
    }
    return Point(-1, -1);
}


const Tile& World::at(const Point &p) const {
    if (!valid(p)) {
        BAD_TILE.terrain = -1;
        return BAD_TILE;
    }
    int c = p.x + p.y * mWidth;
    return mTiles[c];
}

Tile& World::at(const Point &p) {
    if (!valid(p)) {
        BAD_TILE.terrain = -1;
        return BAD_TILE;
    }
    int c = p.x + p.y * mWidth;
    return mTiles[c];
}

bool World::moveActor(Actor *actor, const Point &to) {
    if (!actor || (valid(to) && at(to).actor)) return false; // space already occupied

    if (valid(actor->pos) && at(actor->pos).actor == actor) {
        at(actor->pos).actor = nullptr;
        if (!valid(to)) {
            auto iter = mActors.begin();
            while (iter != mActors.cend()) {
                if (*iter == actor) {
                    iter = mActors.erase(iter);
                } else {
                    ++iter;
                }
            }
            return true;
        }
    } else {
        mActors.push_back(actor);
    }

    if (valid(to)) {
        at(to).actor = actor;
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
        at(item->pos).item = nullptr;
    }

    if (valid(to)) {
        at(to).item = item;
    }
    item->pos = to;
    return true;
}


void World::addLogMsg(const LogMessage &msg) {
    mLog.push_back(msg);
}

void World::addLogMsg(const std::string &msg) {
    mLog.push_back(LogMessage{msg});
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

std::vector<const RecipeDef*> World::getRecipeList() const {
    std::vector<const RecipeDef*> list;
    for (const RecipeDef &td : mRecipeDefs) {
        list.push_back(&td);
    }
    return list;
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

        ++actor->age;

        if (actor->def.aiType == AI_WANDER) {
            Dir dir = static_cast<Dir>(mRandom.next32() % 8);

            Point dest = actor->pos.shift(dir);
            const Tile tile = at(dest);
            if (valid(dest) && !tile.actor && !getTileDef(tile.terrain).solid) {
                moveActor(actor, dest);
            }

        } else if (actor->def.faction == FAC_PLANT) {
            if (actor->def.growTo >= 0 && actor->age >= actor->def.growTime) {
                const ActorDef &def = getActorDef(actor->def.growTo);
                if (def.ident == -1) {
                    actor->age = -9999;
                    std::cerr << actor->def.name << " at " << actor->pos << " has invalid next growth state.\n";
                } else {
                    std::cerr << actor->def.name << " at " << actor->pos << " has grown into " << def.name << ".\n";
                    Actor *newActor = new Actor(def);
                    mActors[i] = newActor;
                    newActor->pos = actor->pos;
                    at(actor->pos).actor = newActor;
                    delete actor;
                }
            }
        }
    }
}

void World::getTime(int *day, int *hour, int *minute) const {
    *day    = this->day;
    *hour   = this->hour;
    *minute = this->minute;
}

void write8(std::ostream &out, uint8_t value) {
    out.write(reinterpret_cast<char*>(&value), 1);
}
void write32(std::ostream &out, uint32_t value) {
    out.write(reinterpret_cast<char*>(&value), 4);
}
void writeString(std::ostream &out, const std::string &s) {
    for (char c : s) {
        write8(out, c);
    }
    write8(out, 0);
}

bool World::savegame(const std::string &filename) const {
    std::cerr << "savegame (info): saving game.\n";
    std::ofstream out(filename, std::ios_base::binary);
    if (!out) {
        std::cerr << "savegame: Failed to open save file.\n";
        return false;
    }

    write32(out, 0x4C5243); // magic number
    write32(out, 0);        // version #
    write32(out, mWidth);
    write32(out, mHeight);

    write32(out, turn);
    write32(out, day);
    write32(out, hour);
    write32(out, minute);


    int itemCount = 0;
    // write tiles
    write32(out, 0x454C4954);
    for (int i = 0; i < mWidth * mHeight; ++i) {
        write32(out, mTiles[i].terrain);
        if (mTiles[i].item) ++itemCount;
    }
    // write items on ground
    write32(out, 0x4D455449);
    write32(out, itemCount);
    for (int i = 0; i < mWidth * mHeight; ++i) {
        if (!mTiles[i].item) continue;
        const Item *item = mTiles[i].item;
        write32(out, item->def.ident);
        write32(out, item->pos.x);
        write32(out, item->pos.y);
    }
    // write actors
    write32(out, 0x52544341);
    write32(out, mActors.size());
    for (const Actor *actor : mActors) {
        write32(out, actor->def.ident);
        write32(out, actor->pos.x);
        write32(out, actor->pos.y);
        write32(out, actor->age);
        write32(out, actor->health);
        write32(out, actor->inventory.size());
        for (const InventoryRow &row : actor->inventory.mContents) {
            write32(out, row.qty);
            write32(out, row.def->ident);
        }
    }
    // write log
    write32(out, 0x00474F4C);
    write32(out, mLog.size());
    for (const LogMessage &msg : mLog) {
        writeString(out, msg.msg);
    }

    return true;
}

uint8_t read8(std::istream &inf) {
    uint8_t value = 0;
    inf.read(reinterpret_cast<char*>(&value), 1);
    return value;
}
uint32_t read32(std::istream &inf) {
    uint32_t value = 0;
    inf.read(reinterpret_cast<char*>(&value), 4);
    return value;
}
std::string readString(std::istream &inf) {
    std::string s;
    char c = read8(inf);
    while (c != 0) {
        s += c;
        c = read8(inf);
    }
    return s;
}

bool World::loadgame(const std::string &filename) {
    std::cerr << "loadgame (info): loading game.\n";
    std::ifstream inf(filename, std::ios_base::binary);
    if (!inf) {
        std::cerr << "loadgame: Failed to open save file.\n";
        return false;
    }

    if (read32(inf) != 0x4C5243) {
        std::cerr << "loadgame: bad magic number.\n";
        return false;
    }
    if (read32(inf) != 0) {
        std::cerr << "loadgame: incompatable save version.\n";
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
        std::cerr << "loadgame: expected start of tile data.\n";
        return false;
    }
    for (int i = 0; i < mWidth * mHeight; ++i) {
        int t = read32(inf);
        mTiles[i].terrain = t;
    }

    // read items on ground
    if (read32(inf) != 0x4D455449) {
        std::cerr << "loadgame: expected start of item data.\n";
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
        std::cerr << "loadgame: expected start of actor data.\n";
        return false;
    }
    int actorCount = read32(inf);
    for (int i = 0; i < actorCount; ++i) {
        if (inf.eof()) {
            std::cerr << "loadgame: unexpected end of file (actors).\n";
            return false;
        }
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
    // read log
    if (read32(inf) != 0x00474F4C) {
        std::cerr << "loadgame: expected start of log data.\n";
        return false;
    }
    int logCount = read32(inf);
    for (int i = 0; i < logCount; ++i) {
        std::string msg = readString(inf);
        addLogMsg(msg);
    }

    return true;
}


std::ostream& operator<<(std::ostream &out, const Point &p) {
    std::stringstream s;
    s << '(' << p.x << ',' << p.y << ')';
    out << s.str();
    return out;
}

std::ostream& operator<<(std::ostream &out, const Dir &d) {
    switch(d) {
        case Dir::North:        out << "north";     break;
        case Dir::Northeast:    out << "northeast"; break;
        case Dir::East:         out << "east";      break;
        case Dir::Southeast:    out << "southeast"; break;
        case Dir::South:        out << "south";     break;
        case Dir::Southwest:    out << "southwest"; break;
        case Dir::West:         out << "west";      break;
        case Dir::Northwest:    out << "northwest"; break;
        case Dir::None:         out << "none";      break;
        default:
            out << "(bad dir " << static_cast<int>(d) << ')';
    }
    return out;
}


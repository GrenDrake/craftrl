#include <ostream>
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
: inProgress(false), mTiles(nullptr), mPlayer(nullptr), turn(0) {
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

    for (unsigned i = 0; i < mWidth * mHeight; ++i) {
        delete mTiles->item;
    }
    for (Actor *actor : mActors) {
        delete actor;
    }
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
    if (!valid(p)) return BAD_TILE;
    int c = p.x + p.y * mWidth;
    return mTiles[c];
}

Tile& World::at(const Point &p) {
    if (!valid(p)) return BAD_TILE;
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
    for (Actor *actor : mActors) {
        if (actor->def.aiType == AI_WANDER) {
            Dir dir = static_cast<Dir>(mRandom.next32() % 8);

            Point dest = actor->pos.shift(dir);
            const Tile tile = at(dest);
            if (valid(dest) && !tile.actor && !getTileDef(tile.terrain).solid) {
                moveActor(actor, dest);
            }

        }
    }
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


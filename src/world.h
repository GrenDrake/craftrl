#ifndef WORLD_H
#define WORLD_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "random.h"

const int AI_NONE = 0;
const int AI_WANDER = 1;

const int FAC_PLAYER    = 0;
const int FAC_VILLAGER  = 1;
const int FAC_PLANT     = 2;
const int FAC_ANIMAL    = 3;
const int FAC_MONSTER   = 4;

enum class Dir {
    North, Northeast, East, Southeast, South, Southwest, West, Northwest,
    None, RequestQuit
};

struct Point {
    Point() : x(0), y(0) {};
    Point(int x, int y) : x(x), y(y) {};

    Point shift(Dir dir, int amnt = 1) const;

    int x, y;
};

struct LootRow {
    int ident;
    int min, max;
    int chance;
};
struct LootTable {
    std::vector<LootRow> mRows;
};

struct ActorDef {
    int ident;
    int glyph;
    unsigned colour;
    std::string name;
    int aiType;
    int faction;
    LootTable *loot;

    int health;
};

struct ItemDef {
    int ident;
    int glyph;
    unsigned colour;
    std::string name;
};

struct TileDef {
    int ident;
    int glyph;
    unsigned colour;
    std::string name;
    int breakTo;
    LootTable *loot;
    bool opaque;
    bool solid;
};

struct InventoryRow {
    int qty;
    const ItemDef *def;
};
struct Inventory {
    bool add(const ItemDef*, int qty = 1);
    int qty(const ItemDef*) const;
    bool remove(const ItemDef*, int qty = 1);

    std::vector<InventoryRow> mContents;
};

struct Actor {
    Actor(const ActorDef &def) : type(def.ident), def(def) { }
    void reset();

    int type;
    const ActorDef &def;
    Point pos;
    Inventory inventory;

    int health;
};

struct Item {
    Item(const ItemDef &def) : type(def.ident), def(def) { }
    int type;
    const ItemDef &def;
    Point pos;
};

struct Tile {
    Tile() : terrain(0), construct(0), actor(nullptr), item(0) { }
    Tile(int tile) : terrain(tile), construct(0), actor(nullptr), item(0) { }

    int terrain;
    int construct;
    Actor *actor;
    Item *item;
};

struct LogMessage {
    std::string msg;
};


class World {
public:

    World();
    void allocMap(int width, int height);
    int width()  const { return mWidth; }
    int height() const { return mHeight; }
    bool valid(const Point &p) const;
    Random& getRandom() { return mRandom; }

    const Point& getCamera() const;
    void setCamera(const Point &to);

    Point findDropSpace(const Point &near) const;
    const Tile& at(const Point &p) const;
    Tile& at(const Point &p);

    bool moveActor(Actor *actor, const Point &to);
    const Actor* getPlayer() const;
    Actor* getPlayer();
    bool moveItem(Item *item, const Point &to);

    void addLogMsg(const LogMessage &msg);
    void addLogMsg(const std::string &msg);
    int getLogCount() const;
    const LogMessage& getLogMsg(int index) const;

    void addActorDef(const ActorDef &ad);
    const ActorDef& getActorDef(int ident) const;
    int actorDefCount() const { return mActorDefs.size(); }
    void addItemDef(const ItemDef &td);
    const ItemDef& getItemDef(int ident) const;
    int itemDefCount() const { return mItemDefs.size(); }
    void addTileDef(const TileDef &td);
    const TileDef& getTileDef(int ident) const;
    int tileDefCount() const { return mTileDefs.size(); }

    void tick();

    bool inProgress;
    int mode, selection;

private:
    static LogMessage BAD_LOGMESSAGE;
    static Tile BAD_TILE;
    static const ActorDef BAD_ACTORDEF;
    static const ItemDef BAD_ITEMDEF;
    static const TileDef BAD_TILEDEF;

    std::vector<ActorDef> mActorDefs;
    std::vector<ItemDef> mItemDefs;
    std::vector<TileDef> mTileDefs;

    Point mCamera;
    std::vector<LogMessage> mLog;

    int mWidth, mHeight;
    Tile *mTiles;
    std::vector<Actor*> mActors;
    std::vector<Item*> mItems;
    Actor *mPlayer;

    mutable Random mRandom;
};


std::ostream& operator<<(std::ostream &out, const Point &p);
std::ostream& operator<<(std::ostream &out, const Dir &d);

#endif
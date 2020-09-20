#ifndef WORLD_H
#define WORLD_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "random.h"

const int AI_NONE = 0;
const int AI_WANDER = 1;

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


struct ActorDef {
    int ident;
    int glyph;
    unsigned colour;
    std::string name;
    int aiType;
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
    bool opaque;
    bool solid;
};

struct Inventory {
    bool add(const ItemDef*, int qty = 1);
    int qty(const ItemDef*) const;
    bool remove(const ItemDef*, int qty = 1);

    std::map<const ItemDef*, int> mContents;
};

struct Actor {
    Actor(const ActorDef &def) : type(def.ident), def(def) { }
    int type;
    const ActorDef &def;
    Point pos;
    Inventory inventory;
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

    Point findOpenTile(bool allowActor, bool allowItem) const;
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

bool buildmap(World &w);


#endif
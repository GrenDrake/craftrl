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

const int CMD_NONE              = -1;
const int CMD_DUMPMAP           = 0;
const int CMD_TAKE              = 1;
const int CMD_BREAK             = 2;
const int CMD_MOVE              = 3;
const int CMD_PAN               = 4;
const int CMD_RESETVIEW         = 5;
const int CMD_DROP              = 6;
const int CMD_USE               = 7;
const int CMD_WAIT              = 8;
const int CMD_CONTEXTMOVE       = 9;
const int CMD_QUIT              = 10;
const int CMD_NEXT_SELECT       = 11;
const int CMD_PREV_SELECT       = 12;
const int CMD_CANCEL            = 13;
const int CMD_CRAFT             = 14;

enum class Dir {
    North, Northeast, East, Southeast, South, Southwest, West, Northwest,
    None
};

struct Point {
    Point() : x(0), y(0) {};
    Point(int x, int y) : x(x), y(y) {};

    Point shift(Dir dir, int amnt = 1) const;

    int x, y;
};

struct Command {
    int key;
    bool ctrl, alt, shift;
    int command;
    Dir dir;
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
    int growTo;
    int growTime;
    int health;
};

struct ItemDef {
    int ident;
    int glyph;
    unsigned colour;
    std::string name;
    int seedFor;
    int constructs;
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

struct RecipeRow {
    int qty;
    int ident;
};
struct RecipeDef {
    int makeIdent;
    int makeQty;
    std::vector<RecipeRow> mRows;
};

struct InventoryRow {
    int qty;
    const ItemDef *def;
};
struct Inventory {
    bool add(const ItemDef*, int qty = 1);
    int qty(const ItemDef*) const;
    bool remove(const ItemDef*, int qty = 1);
    int size() const { return mContents.size(); }

    std::vector<InventoryRow> mContents;
};

struct Actor {
    Actor(const ActorDef &def) : type(def.ident), def(def), age(0) { }
    void reset();

    int type;
    const ActorDef &def;
    Point pos;
    Inventory inventory;

    int health;
    int age;
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
    void deallocMap();
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
    void addRecipeDef(const RecipeDef &td);
    const RecipeDef& getRecipeDef(int ident) const;
    int recipeDefCount() const { return mRecipeDefs.size(); }
    std::vector<const RecipeDef*> getRecipeList() const;

    void tick();
    unsigned getTurn() const { return turn; }

    bool inProgress, wantsToQuit;
    int mode, selection;

private:
    static LogMessage BAD_LOGMESSAGE;
    static Tile BAD_TILE;
    static const ActorDef BAD_ACTORDEF;
    static const ItemDef BAD_ITEMDEF;
    static const TileDef BAD_TILEDEF;
    static const RecipeDef BAD_RECIPEDEF;

    std::vector<ActorDef> mActorDefs;
    std::vector<ItemDef> mItemDefs;
    std::vector<TileDef> mTileDefs;
    std::vector<RecipeDef> mRecipeDefs;

    Point mCamera;
    std::vector<LogMessage> mLog;

    int mWidth, mHeight;
    Tile *mTiles;
    std::vector<Actor*> mActors;
    Actor *mPlayer;

    unsigned turn;

    mutable Random mRandom;
};


extern const Command gameCommands[];
const Command& findCommand(int key, const Command *commandList);


std::ostream& operator<<(std::ostream &out, const Point &p);
std::ostream& operator<<(std::ostream &out, const Dir &d);

#endif
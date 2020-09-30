#ifndef WORLD_H
#define WORLD_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "random.h"

const int INPUT_KEY_COUNT = 3;

const int AI_NONE = 0;
const int AI_WANDER = 1;

const int TYPE_PLAYER    = 0;
const int TYPE_VILLAGER  = 1;
const int TYPE_PLANT     = 2;
const int TYPE_ANIMAL    = 3;
const int TYPE_MONSTER   = 4;

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
const int CMD_SAVE              = 15;
const int CMD_DO                = 16;
const int CMD_TALK              = 17;
const int CMD_SELECT_PGDN       = 18;
const int CMD_SELECT_PGUP       = 19;
const int CMD_SELECT_HOME       = 20;
const int CMD_SELECT_END        = 21;

enum class Dir {
    North, Northeast, East, Southeast, South, Southwest, West, Northwest,
    None
};

struct Point {
    Point() : x(0), y(0) {};
    Point(int x, int y) : x(x), y(y) {};

    Point shift(Dir dir, int amnt = 1) const;
    Dir directionTo(const Point &rhs) const;
    double distance(const Point &rhs) const;

    int x, y;
};


struct InputKey {
    int key;
    bool ctrl, alt, shift;
};
struct Command {
    int command;
    Dir dir;
    InputKey key[INPUT_KEY_COUNT];
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
    int type;
    LootTable *loot;
    int growTo;
    int growTime;
    int health;
    int foodItem;
    int moveChance;
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
    int doorTo;
    LootTable *loot;
    bool opaque;
    bool solid;
    bool ground;
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
    ~World();
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
    // Tile& at(const Point &p);
    void setTerrain(const Point &pos, int toTile);
    void setActor(const Point &pos, Actor *toActor);
    void setItem(const Point &pos, Item *toItem);

    bool moveActor(Actor *actor, const Point &to);
    bool tryMoveActor(Actor *actor, Dir baseDir, bool allowSidestep = true);
    const Actor* getPlayer() const;
    Actor* getPlayer();
    bool moveItem(Item *item, const Point &to);

    Point findItemNearest(const Point &to, int itemIdent, int radius) const;

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
    void getTime(int *day, int *hour, int *minute) const;

    bool savegame(const std::string &filename) const;
    bool loadgame(const std::string &filename);

    bool inProgress, wantsToQuit;
    int mode, selection;

private:
    const static LogMessage BAD_LOGMESSAGE;
    static const Tile BAD_TILE;
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
    unsigned day, hour, minute;

    mutable Random mRandom;
};

typedef bool (*ActionHandler)(World&, Actor*, const Command&, bool);

bool actionAttack(World &w, Actor *player, const Command &command, bool silent);
bool actionCentrePan(World &w, Actor *player, const Command &command, bool silent);
bool actionContextMove(World &w, Actor *player, const Command &command, bool silent);
bool actionCraft(World &w, Actor *player, const Command &command, bool silent);
bool actionDo(World &w, Actor *player, const Command &command, bool silent);
bool actionDrop(World &w, Actor *player, const Command &command, bool silent);
bool actionDumpMap(World &w, Actor *player, const Command &command, bool silent);
bool actionMove(World &w, Actor *player, const Command &command, bool silent);
bool actionNextSelect(World &w, Actor *player, const Command &command, bool silent);
bool actionPan(World &w, Actor *player, const Command &command, bool silent);
bool actionPrevSelect(World &w, Actor *player, const Command &command, bool silent);
bool actionQuit(World &w, Actor *player, const Command &command, bool silent);
bool actionSavegame(World &w, Actor *player, const Command &command, bool silent);
bool actionSelectEnd(World &w, Actor *player, const Command &command, bool silent);
bool actionSelectHome(World &w, Actor *player, const Command &command, bool silent);
bool actionSelectPagedown(World &w, Actor *player, const Command &command, bool silent);
bool actionSelectPageup(World &w, Actor *player, const Command &command, bool silent);
bool actionTake(World &w, Actor *player, const Command &command, bool silent);
bool actionTalkActor(World &w, Actor *player, const Command &command, bool silent);
bool actionUse(World &w, Actor *player, const Command &command, bool silent);
bool actionWait(World &w, Actor *player, const Command &command, bool silent);


extern const Command gameCommands[];
const Command& findCommand(int key, const Command *commandList);
std::string commandName(int command);
ActionHandler commandAction(int command);

void ui_MessageBox(const std::string &title, const std::string &message);

Dir rotate45(Dir d);
Dir unrotate45(Dir d);

std::ostream& operator<<(std::ostream &out, const Point &p);
std::string directionName(Dir d);
std::ostream& operator<<(std::ostream &out, const Dir &d);

extern const Point nowhere;

#endif
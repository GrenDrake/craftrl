#include <map>
#include <iostream>
#include "data.h"
#include "world.h"

LootTable* parseLootTable(World &w, TokenData &data);

bool parseAddfile(World &w, TokenData &data) {
    data.next(); // skip "addfile"

    while (!data.matches(TokenType::Semicolon)) {
        if (!data.require(TokenType::String)) return false;
        for (const std::string &s : data.fileList) {
            if (s == data.here().s) {
                logger_log(data.here().origin.toString() + "  File \"" + data.here().s + "\" already included.");
                return false;
            }
        }
        logger_log("Including data file " + data.here().s + ".");
        data.fileList.push_back(data.here().s);
        data.next();
   }

    data.next(); // skip ";"
    return true;
}


bool parseActor(World &w, TokenData &data) {
    const Origin &fullOrigin = data.here().origin;
    data.next(); // skip "tile"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default actordef data
    ActorDef actor;
    actor.ident = -1;
    actor.glyph = '?';
    actor.colour = 0xFFFFFFFF;
    actor.name = "unnamed actor";
    actor.aiType = AI_NONE;
    actor.health = 1;
    actor.type = 99;
    actor.growTo = -1;
    actor.growTime = 100;
    actor.loot = nullptr;
    actor.foodItem = -1;
    actor.moveChance = 1000;
    actor.baseDamage = 1;

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "ident") {
            if (!data.asInt(actor.ident)) return false;
            data.next();
        } else if (name == "glyph") {
            if (!data.asInt(actor.glyph)) return false;
            data.next();
        } else if (name == "colour") {
            int value = 0;
            if (!data.asInt(value)) return false;
            actor.colour = static_cast<unsigned>(value) | 0xFF000000;
            data.next();
        } else if (name == "name") {
            if (!data.require(TokenType::String)) return false;
            actor.name = data.here().s;
            data.next();
        } else if (name == "ai") {
            if (!data.asInt(actor.aiType)) return false;
            data.next();
        } else if (name == "type") {
            if (!data.asInt(actor.type)) return false;
            data.next();
        } else if (name == "health") {
            if (!data.asInt(actor.health)) return false;
            data.next();
        } else if (name == "defaultFaction") {
            if (!data.asInt(actor.defaultFaction)) return false;
            data.next();
        } else if (name == "baseDamage") {
            if (!data.asInt(actor.baseDamage)) return false;
            data.next();
        } else if (name == "growTo") {
            if (!data.asInt(actor.growTo)) return false;
            data.next();
        } else if (name == "growTime") {
            if (!data.asInt(actor.growTime)) return false;
            data.next();
        } else if (name == "foodItem") {
            if (!data.asInt(actor.foodItem)) return false;
            data.next();
        } else if (name == "moveChance") {
            if (!data.asInt(actor.moveChance)) return false;
            data.next();
        } else if (name == "loot") {
            LootTable *table = parseLootTable(w, data);
            if (actor.loot) {
            const Origin &origin = data.here().origin;
                logger_log(origin.toString() + "  multiple loot tables found.");
                delete table;
                return false;
            } else {
                actor.loot = table;
            }
        } else {
            const Origin &origin = data.here().origin;
            logger_log(origin.toString() + "  Unknown property " + name + ".");
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    if (w.getActorDef(actor.ident).ident >= 0) {
            logger_log(fullOrigin.toString() + "  actor ident " + std::to_string(actor.ident) + " already used.");
            return false;
    }
    w.addActorDef(actor);
    return true;
}

bool parseDefine(World &w, TokenData &data) {
    const Origin &origin = data.here().origin;
    data.next(); // skip "define"

    if (!data.require(TokenType::Identifier)) return false;
    const std::string &name = data.here().s;
    data.next();

    if (!data.require(TokenType::Integer)) return false;
    int value = data.here().i;
    data.next();

    if (!data.require(TokenType::Semicolon)) return false;
    data.next();

    auto iter = data.symbols.find(name);
    if (iter != data.symbols.end()) {
        logger_log(origin.toString() + "  Name " + name + " already defined.");
        return false;
    }
    data.symbols.insert(std::make_pair(name, value));
    return true;
}

bool parseItem(World &w, TokenData &data) {
    const Origin &fullOrigin = data.here().origin;
    data.next(); // skip "tile"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default itemdef data
    ItemDef item;
    item.ident = -1;
    item.seedFor = -1;
    item.constructs = -1;
    item.glyph = '?';
    item.makeFloor = false;
    item.colour = 0xFFFFFFFF;
    item.name = "unnamed item";

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "makeFloor") item.makeFloor = true;
        else if (name == "ident") {
            if (!data.asInt(item.ident)) return false;
            data.next();
        } else if (name == "glyph") {
            if (!data.asInt(item.glyph)) return false;
            data.next();
        } else if (name == "colour") {
            int value = 0;
            if (!data.asInt(value)) return false;
            item.colour = static_cast<unsigned>(value) | 0xFF000000;
            data.next();
        } else if (name == "seedFor") {
            if (!data.asInt(item.seedFor)) return false;
            data.next();
        } else if (name == "constructs") {
            if (!data.asInt(item.constructs)) return false;
            data.next();
        } else if (name == "name") {
            if (!data.require(TokenType::String)) return false;
            item.name = data.here().s;
            data.next();
        } else if (name == "plural") {
            if (!data.require(TokenType::String)) return false;
            item.plural = data.here().s;
            data.next();
        } else {
            const Origin &origin = data.here().origin;
            logger_log(origin.toString() + "  Unknown property " + name + ".");
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    if (w.getItemDef(item.ident).ident >= 0) {
            logger_log(fullOrigin.toString() + "  item ident " + std::to_string(item.ident) + " already used.");
            return false;
    }
    if (item.plural.empty()) item.plural = item.name + "s";
    w.addItemDef(item);
    return true;
}

LootTable* parseLootTable(World &w, TokenData &data) {
    if (!data.require(TokenType::OpenBrace)) return nullptr;
    data.next(); // skip "{"

    // set up default LootTable data
    LootTable *table = new LootTable;

    while (!data.matches(TokenType::CloseBrace)) {
        LootRow row;

        if (!data.asInt(row.ident)) return nullptr;
        data.next();

        if (!data.asInt(row.chance)) return nullptr;
        data.next();

        if (!data.asInt(row.min)) return nullptr;
        data.next();

        if (!data.asInt(row.max)) return nullptr;
        data.next();

        table->mRows.push_back(row);
    }

    data.next(); // skip "}"
    return table;
}


bool parseRecipe(World &w, TokenData &data) {
    data.next(); // skip "tile"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default recipedef data
    RecipeDef recipe;
    recipe.makeQty = 1;
    recipe.makeIdent = -1;
    recipe.craftingStation = 0;

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "makeQty") {
            if (!data.asInt(recipe.makeQty)) return false;
            data.next();
        } else if (name == "makeIdent") {
            if (!data.asInt(recipe.makeIdent)) return false;
            data.next();
        } else if (name == "craftingStation") {
            int value = 0;
            if (!data.asInt(value)) return false;
            recipe.craftingStation = value;
            data.next();
        } else if (name == "part") {
            RecipeRow row;
            if (!data.asInt(row.qty)) return false;
            data.next();
            if (!data.asInt(row.ident)) return false;
            data.next();
            recipe.mRows.push_back(row);
        } else {
            const Origin &origin = data.here().origin;
            logger_log(origin.toString() + "  Unknown property " + name + ".");
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    w.addRecipeDef(recipe);
    return true;
}


bool parseRoom(World &w, TokenData &data) {
    const Origin &fullOrigin = data.here().origin;
    data.next(); // skip "room"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default actordef data
    RoomDef room;
    room.ident = -1;
    room.name = "generic room";
    room.value = 0;
    room.colour = 0xFFFFFFFF;

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "ident") {
            if (!data.asInt(room.ident)) return false;
            data.next();
        } else if (name == "value") {
            if (!data.asInt(room.value)) return false;
            data.next();
        } else if (name == "colour") {
            int value = 0;
            if (!data.asInt(value)) return false;
            room.colour = static_cast<unsigned>(value) | 0xFF000000;
            data.next();
        } else if (name == "name") {
            if (!data.require(TokenType::String)) return false;
            room.name = data.here().s;
            data.next();
        } else if (name == "requires") {
            int value = 0;
            if (!data.asInt(value)) return false;
            room.requirements.push_back(value);
            data.next();
        } else {
            const Origin &origin = data.here().origin;
            logger_log(origin.toString() + "  Unknown property " + name + ".");
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    if (w.getRoomDef(room.ident).ident >= 0) {
        logger_log(fullOrigin.toString() + "  room ident " + std::to_string(room.ident) + " already used.");
        return false;
    }
    w.addRoomDef(room);
    return true;
}

bool parseTile(World &w, TokenData &data) {
    const Origin &fullOrigin = data.here().origin;
    data.next(); // skip "tile"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default tiledef data
    TileDef tile;
    tile.ident = -1;
    tile.glyph = '?';
    tile.colour = 0xFFFFFFFF;
    tile.name = "unnamed tile";
    tile.opaque = false;
    tile.solid = false;
    tile.ground = false;
    tile.breakTo = -1;
    tile.doorTo = -1;
    tile.loot = nullptr;
    tile.grantsCrafting = 0;
    tile.isWall = false;
    tile.wallGroup = -1;
    tile.connectingTile = false;

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "opaque") tile.opaque = true;
        else if (name == "solid") tile.solid = true;
        else if (name == "ground") tile.ground = true;
        else if (name == "isWall") tile.isWall = true;
        else if (name == "connectingTile") tile.connectingTile = true;
        else if (name == "ident") {
            if (!data.asInt(tile.ident)) return false;
            data.next();
        } else if (name == "glyph") {
            if (!data.asInt(tile.glyph)) return false;
            data.next();
        } else if (name == "colour") {
            int value = 0;
            if (!data.asInt(value)) return false;
            tile.colour = static_cast<unsigned>(value) | 0xFF000000;
            data.next();
        } else if (name == "name") {
            if (!data.require(TokenType::String)) return false;
            tile.name = data.here().s;
            data.next();
        } else if (name == "breakTo") {
            if (!data.asInt(tile.breakTo)) return false;
            data.next();
        } else if (name == "doorTo") {
            if (!data.asInt(tile.doorTo)) return false;
            data.next();
        } else if (name == "wallGroup") {
            if (!data.asInt(tile.wallGroup)) return false;
            data.next();
        } else if (name == "grantsCrafting") {
            int value = 0;
            if (!data.asInt(value)) return false;
            tile.grantsCrafting = value;
            data.next();
        } else if (name == "loot") {
            LootTable *table = parseLootTable(w, data);
            if (tile.loot) {
            const Origin &origin = data.here().origin;
                logger_log(origin.toString() + "  multiple loot tables found.");
                delete table;
                return false;
            } else {
                tile.loot = table;
            }
        } else {
            const Origin &origin = data.here().origin;
            logger_log(origin.toString() + "  Unknown property " + name + ".");
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    if (w.getTileDef(tile.ident).ident >= 0) {
            logger_log(fullOrigin.toString() + "  tile ident " + std::to_string(tile.ident) + " already used.");
            return false;
    }
    w.addTileDef(tile);
    return true;
}


bool loadGameData(World &w, const std::string &filename) {
    TokenData data;
    data.fileList.push_back(filename);
    int errorCount = 0;

    for (unsigned i = 0; i < data.fileList.size(); ++i) {
        errorCount += loadGameData_Core(w, data, data.fileList[i]);
    }

    logger_log("Loaded " + std::to_string(w.actorDefCount()) + " actors.");
    logger_log("Loaded "   + std::to_string(w.itemDefCount()) + " items.");
    logger_log("Loaded "   + std::to_string(w.tileDefCount()) + " tiles.");
    logger_log("Loaded "   + std::to_string(w.recipeDefCount()) + " recipes.");
    logger_log("Loaded "   + std::to_string(w.roomDefCount()) + " rooms.");
    if (errorCount > 0) {
        logger_log("Found " + std::to_string(errorCount) + " errors in data file.");
        return false;
    }
    return true;
}

int loadGameData_Core(World &w, TokenData &data, const std::string &filename) {
    auto tokens = parseFile(filename);
    if (tokens.empty()) {
        logger_log(filename + "  failed to lex file.");
        return 1;
    }
    data.tokens = &tokens;
    data.pos = 0;


    int errorCount = 0;
    while (!data.end()) {
        if (data.matches(TokenType::Semicolon)) {
            data.next();
            continue;
        }

        if (!data.require(TokenType::Identifier)) { data.skipTo(TokenType::Semicolon); ++errorCount; continue; }

        bool success = false;
        if (data.matches("tile"))           success = parseTile(w, data);
        else if (data.matches("item"))      success = parseItem(w, data);
        else if (data.matches("actor"))     success = parseActor(w, data);
        else if (data.matches("recipe"))    success = parseRecipe(w, data);
        else if (data.matches("define"))    success = parseDefine(w, data);
        else if (data.matches("addfile"))   success = parseAddfile(w, data);
        else if (data.matches("room"))      success = parseRoom(w, data);
        else {
            const Origin &origin = data.here().origin;
            logger_log(origin.toString() + "  Unknown data type " + data.here().s + ".");
            ++errorCount;
            success = false;
            data.next();
        }

        if (!success) {
            ++errorCount;
            data.skipTo(TokenType::Semicolon);
        }
    }
    return errorCount;
}

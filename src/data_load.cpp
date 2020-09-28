#include <iostream>
#include "data.h"
#include "world.h"

LootTable* parseLootTable(World &w, TokenData &data);


bool parseActor(World &w, TokenData &data) {
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
                std::cerr << origin.filename << ':' << origin.line << "  multiple loot tables found.\n";
                delete table;
                return false;
            } else {
                actor.loot = table;
            }
        } else {
            const Origin &origin = data.here().origin;
            std::cerr << origin.filename << ':' << origin.line << "  Unknown property " << name << ".\n";
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
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
        std::cerr << origin.filename << ':' << origin.line << "  Name " << name << " already defined.\n";
        return false;
    }
    data.symbols.insert(std::make_pair(name, value));
    return true;
}

bool parseItem(World &w, TokenData &data) {
    data.next(); // skip "tile"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default itemdef data
    ItemDef item;
    item.ident = -1;
    item.seedFor = -1;
    item.constructs = -1;
    item.glyph = '?';
    item.colour = 0xFFFFFFFF;
    item.name = "unnamed item";

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "ident") {
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
        } else {
            const Origin &origin = data.here().origin;
            std::cerr << origin.filename << ':' << origin.line << "  Unknown property " << name << ".\n";
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
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
        } else if (name == "part") {
            RecipeRow row;
            if (!data.asInt(row.qty)) return false;
            data.next();
            if (!data.asInt(row.ident)) return false;
            data.next();
            recipe.mRows.push_back(row);
        } else {
            const Origin &origin = data.here().origin;
            std::cerr << origin.filename << ':' << origin.line << "  Unknown property " << name << ".\n";
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    w.addRecipeDef(recipe);
    return true;
}

bool parseTile(World &w, TokenData &data) {
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

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "opaque") tile.opaque = true;
        else if (name == "solid") tile.solid = true;
        else if (name == "ground") tile.ground = true;
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
        } else if (name == "loot") {
            LootTable *table = parseLootTable(w, data);
            if (tile.loot) {
            const Origin &origin = data.here().origin;
                std::cerr << origin.filename << ':' << origin.line << "  multiple loot tables found.\n";
                delete table;
                return false;
            } else {
                tile.loot = table;
            }
        } else {
            const Origin &origin = data.here().origin;
            std::cerr << origin.filename << ':' << origin.line << "  Unknown property " << name << ".\n";
            return false;
        }
    }

    data.next(); // skip "}"
    if (!data.require(TokenType::Semicolon)) return false;
    data.next(); // skip ";"
    w.addTileDef(tile);
    return true;
}


bool loadGameData(World &w) {
    TokenData data = parseFile("game.dat");
    if (!data.valid) return false;


    int errorCount = 0;
    while (!data.end()) {
        if (data.matches(TokenType::Semicolon)) {
            data.next();
            continue;
        }

        if (!data.require(TokenType::Identifier)) { data.skipTo(TokenType::Semicolon); ++errorCount; continue; }

        bool success = false;
        if (data.matches("tile"))       success = parseTile(w, data);
        else if (data.matches("item"))  success = parseItem(w, data);
        else if (data.matches("actor")) success = parseActor(w, data);
        else if (data.matches("recipe"))success = parseRecipe(w, data);
        else if (data.matches("define"))success = parseDefine(w, data);
        else {
            const Origin &origin = data.here().origin;
            std::cerr << origin.filename << ':' << origin.line << "  Unknown data type " << data.here().type << ".\n";
            ++errorCount;
            success = false;
            data.next();
        }

        if (!success) {
            ++errorCount;
            data.skipTo(TokenType::Semicolon);
        }
    }

    std::cerr << "Loaded " << w.actorDefCount() << " actors.\n";
    std::cerr << "Loaded " << w.itemDefCount() << " items.\n";
    std::cerr << "Loaded " << w.tileDefCount() << " tiles.\n";
    std::cerr << "Loaded " << w.recipeDefCount() << " recipes.\n";
    if (errorCount > 0) {
        std::cerr << "Found " << errorCount << " errors in data file.\n";
        return false;
    }
    return true;
}







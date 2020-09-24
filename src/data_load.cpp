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
    actor.faction = 99;
    actor.loot = nullptr;

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "ident") {
            if (!data.require(TokenType::Integer)) return false;
            actor.ident = data.here().i;
            data.next();
        } else if (name == "glyph") {
            if (!data.require(TokenType::Integer)) return false;
            actor.glyph = data.here().i;
            data.next();
        } else if (name == "colour") {
            if (!data.require(TokenType::Integer)) return false;
            actor.colour = static_cast<unsigned>(data.here().i) | 0xFF000000;
            data.next();
        } else if (name == "name") {
            if (!data.require(TokenType::String)) return false;
            actor.name = data.here().s;
            data.next();
        } else if (name == "ai") {
            if (!data.require(TokenType::Integer)) return false;
            actor.aiType = data.here().i;
            data.next();
        } else if (name == "faction") {
            if (!data.require(TokenType::Integer)) return false;
            actor.faction = data.here().i;
            data.next();
        } else if (name == "health") {
            if (!data.require(TokenType::Integer)) return false;
            actor.health = data.here().i;
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

bool parseItem(World &w, TokenData &data) {
    data.next(); // skip "tile"

    if (!data.require(TokenType::OpenBrace)) return false;
    data.next(); // skip "{"

    // set up default itemdef data
    ItemDef item;
    item.ident = -1;
    item.seedFor = -1;
    item.glyph = '?';
    item.colour = 0xFFFFFFFF;
    item.name = "unnamed item";

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "ident") {
            if (!data.require(TokenType::Integer)) return false;
            item.ident = data.here().i;
            data.next();
        } else if (name == "glyph") {
            if (!data.require(TokenType::Integer)) return false;
            item.glyph = data.here().i;
            data.next();
        } else if (name == "colour") {
            if (!data.require(TokenType::Integer)) return false;
            item.colour = static_cast<unsigned>(data.here().i) | 0xFF000000;
            data.next();
        } else if (name == "seedFor") {
            if (!data.require(TokenType::Integer)) return false;
            item.seedFor = static_cast<unsigned>(data.here().i);
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

        if (!data.require(TokenType::Integer)) return nullptr;
        row.ident = data.here().i;
        data.next();

        if (!data.require(TokenType::Integer)) return nullptr;
        row.chance = data.here().i;
        data.next();

        if (!data.require(TokenType::Integer)) return nullptr;
        row.min = data.here().i;
        data.next();

        if (!data.require(TokenType::Integer)) return nullptr;
        row.max = data.here().i;
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
            if (!data.require(TokenType::Integer)) return false;
            recipe.makeQty = data.here().i;
            data.next();
        } else if (name == "makeIdent") {
            if (!data.require(TokenType::Integer)) return false;
            recipe.makeIdent = data.here().i;
            data.next();
        } else if (name == "part") {
            RecipeRow row;
            if (!data.require(TokenType::Integer)) return false;
            row.qty = data.here().i;
            data.next();
            if (!data.require(TokenType::Integer)) return false;
            row.ident = data.here().i;
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
    tile.breakTo = -1;
    tile.loot = nullptr;

    while (!data.matches(TokenType::CloseBrace)) {
        if (!data.require(TokenType::Identifier)) return false;
        const std::string &name = data.here().s;
        data.next();

        if (name == "opaque") tile.opaque = true;
        else if (name == "solid") tile.solid = true;
        else if (name == "ident") {
            if (!data.require(TokenType::Integer)) return false;
            tile.ident = data.here().i;
            data.next();
        } else if (name == "glyph") {
            if (!data.require(TokenType::Integer)) return false;
            tile.glyph = data.here().i;
            data.next();
        } else if (name == "colour") {
            if (!data.require(TokenType::Integer)) return false;
            tile.colour = static_cast<unsigned>(data.here().i) | 0xFF000000;
            data.next();
        } else if (name == "name") {
            if (!data.require(TokenType::String)) return false;
            tile.name = data.here().s;
            data.next();
        } else if (name == "breakTo") {
            if (!data.require(TokenType::Integer)) return false;
            tile.breakTo = data.here().i;
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







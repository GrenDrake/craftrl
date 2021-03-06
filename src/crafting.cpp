#include <BearLibTerminal.h>
#include "world.h"

const int MAX_CRAFT = 1000000;

bool canMakeRecipe(World &w, const RecipeDef *recipe, int qty, const Inventory &inventory) {
    if (!recipe) return false;

    for (const RecipeRow &row : recipe->mRows) {
        const ItemDef &partDef = w.getItemDef(row.ident);
        if (inventory.qty(&partDef) < row.qty * qty) {
            return false;
        }
    }
    return true;
}

int maxCraftable(World &w, const RecipeDef *recipe, const Inventory &inventory) {
    int maxCount = 0;
    for (int i = 1; i < 99999; ++i) {
        if (canMakeRecipe(w, recipe, i, inventory)) {
            maxCount = i;
        }
    }
    return maxCount;
}

void doCrafting(World &w, Actor *player, unsigned craftingStation) {
    const unsigned highlightBG  = 0xFF666666;
    const unsigned highlightFG  = 0xFFFFFFFF;
    const unsigned textBG       = 0xFF000000;
    const unsigned textFG       = 0xFFAAAAAA;
    const unsigned validFG      = 0xFFAAFFAA;
    const unsigned invalidFG    = 0xFFFFAAAA;
    const int screenHeight = 25;

    const auto list = w.getRecipeList(craftingStation);

    int selection = 0, count = 1;
    while (1) {
        const RecipeDef *current = nullptr;

        terminal_bkcolor(textBG);
        terminal_color(textFG);
        terminal_clear();
        for (int y = 0; y < screenHeight; ++y) {
            terminal_put(29, y, LD_VERTICAL);
        }

        int cy = 0;
        for (const auto *row : list) {
            if (!row) continue;
            const ItemDef &makeDef = w.getItemDef(row->makeIdent);
            if (cy == selection) {
                terminal_color(highlightFG);
                terminal_bkcolor(highlightBG);
                terminal_clear_area(0, cy, 29, 1);
                terminal_put(0, cy, '>');
                current = row;
            } else {
                terminal_color(textFG);
                terminal_bkcolor(textBG);
            }
            if (canMakeRecipe(w, row, count, player->inventory))  {
                terminal_color(0xFFAAFFAA);
            } else {
                terminal_color(0xFFFFAAAA);
            }
            if (makeDef.ident >= 0) {
                terminal_printf(4, cy, "%d %s", row->makeQty * count, makeDef.name.c_str());
                terminal_color(0xFFFFFFFF);
                terminal_put(2, cy, makeDef.glyph);
            } else {
                terminal_printf(4, cy, "Bad item ident: %s", row->makeIdent);
            }
            ++cy;
        }

        terminal_color(textFG);
        terminal_bkcolor(textBG);
        terminal_printf(35, 24, " Crafting: %d/%d ", count, maxCraftable(w, current, player->inventory));
        bool canMake = canMakeRecipe(w, current, count, player->inventory);
        if (current) {
            cy = 0;
            for (const RecipeRow &row : current->mRows) {
                const ItemDef &partDef = w.getItemDef(row.ident);
                int qtyHeld = player->inventory.qty(&partDef);
                if (qtyHeld < row.qty * count) {
                    terminal_color(invalidFG);
                } else {
                    terminal_color(validFG);
                }
                if (partDef.ident >= 0) {
                    terminal_printf(33, cy, "%d/%d %s", row.qty * count, qtyHeld, partDef.name.c_str());
                    terminal_color(0xFFFFFFFF);
                    terminal_put(31, cy, partDef.glyph);
                } else {
                    terminal_printf(32, cy, "Bad item ident: %s", row.ident);
                }
                ++cy;
            }
        }

        terminal_refresh();

        int key = terminal_read();
        switch(key) {
            case TK_ESCAPE:
            case TK_CLOSE:
            case TK_Z:
            case TK_Q:
                return;
            case TK_KP_2:
            case TK_DOWN:
                if (selection < static_cast<int>(list.size()) - 1) ++selection;
                break;
            case TK_KP_8:
            case TK_UP:
                if (selection > 0) --selection;
                break;
            case TK_END:
                selection = static_cast<int>(list.size()) - 1;
                break;
            case TK_HOME:
                selection = 0;
                break;
            case TK_PAGEUP:
                selection -= 10;
                if (selection < 0) selection = 0;
                break;
            case TK_PAGEDOWN:
                selection += 10;
                if (selection >= static_cast<int>(list.size())) selection = static_cast<int>(list.size()) - 1;
                break;
            case TK_BACKSPACE:
                count /= 10;
                if (count <= 0) count = 1;
                break;
            case TK_KP_PLUS:
            case TK_EQUALS:
            case TK_KP_6:
            case TK_RIGHT:
                if (count < MAX_CRAFT) ++count;
                break;
            case TK_MINUS:
            case TK_KP_MINUS:
            case TK_KP_4:
            case TK_LEFT:
                if (count > 1) --count;
                break;
            case TK_M: {
                int result = maxCraftable(w, current, player->inventory);
                if (result > 0) count = result;
                break; }
            case TK_1:
                count *= 10;
                count += 1;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_2:
                count *= 10;
                count += 2;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_3:
                count *= 10;
                count += 3;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_4:
                count *= 10;
                count += 4;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_5:
                count *= 10;
                count += 5;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_6:
                count *= 10;
                count += 6;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_7:
                count *= 10;
                count += 7;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_8:
                count *= 10;
                count += 8;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_9:
                count *= 10;
                count += 9;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_0:
                count *= 10;
                if (count > MAX_CRAFT) count = MAX_CRAFT;
                break;
            case TK_ENTER:
            case TK_KP_ENTER:
            case TK_C:
                if (current && canMake) {
                    for (const RecipeRow &row : current->mRows) {
                        const ItemDef &partDef = w.getItemDef(row.ident);
                        player->inventory.remove(&partDef, row.qty * count);
                    }
                    const ItemDef &makeDef = w.getItemDef(current->makeIdent);
                    player->inventory.add(&makeDef, current->makeQty * count);

                }
                break;
        }
    }


}

#include <BearLibTerminal.h>
#include "world.h"

bool canMakeRecipe(World &w, const RecipeDef *def, const Inventory &inventory) {
    if (!def) return false;

    for (const RecipeRow &row : def->mRows) {
        const ItemDef &partDef = w.getItemDef(row.ident);
        if (inventory.qty(&partDef) < row.qty) {
            return false;
        }
    }
    return true;
}

void doCrafting(World &w, Actor *player) {
    const unsigned highlightBG  = 0xFF666666;
    const unsigned highlightFG  = 0xFFFFFFFF;
    const unsigned textBG       = 0xFF000000;
    const unsigned textFG       = 0xFFAAAAAA;
    const unsigned validFG      = 0xFFAAFFAA;
    const unsigned invalidFG    = 0xFFFFAAAA;

    const int screenWidth = 80;
    const int screenHeight = 24;
    const int inventoryHeight = 11;
    const int inventoryY = screenHeight - inventoryHeight;

    const auto list = w.getRecipeList();

    int selection = 0;
    while (1) {
        const RecipeDef *current = nullptr;

        terminal_bkcolor(textBG);
        terminal_color(textFG);
        terminal_clear();
        for (int y = 0; y < screenHeight; ++y) {
            terminal_put(29, y, '|');
        }
        for (int x = 30; x < screenWidth; ++x) {
            terminal_put(x, inventoryY - 1, '-');
        }
        terminal_put(29, inventoryY - 1, '+');

        int cy = 0;
        for (const auto *row : list) {
            if (!row) continue;
            const ItemDef &makeDef = w.getItemDef(row->makeIdent);
            if (cy == selection) {
                terminal_color(highlightFG);
                terminal_bkcolor(highlightBG);
                terminal_clear_area(0, cy, 29, 1);
                terminal_put(2, cy, '>');
                current = row;
            } else {
                terminal_color(textFG);
                terminal_bkcolor(textBG);
            }
            if (canMakeRecipe(w, row, player->inventory))  {
                terminal_color(0xFFAAFFAA);
                terminal_put(0, cy, '+');
            } else {
                terminal_color(0xFFFFAAAA);
            }
            if (makeDef.ident >= 0) {
                terminal_printf(4, cy, "%d %s", row->makeQty, makeDef.name.c_str());
            } else {
                terminal_printf(4, cy, "Bad item ident: %s", row->makeIdent);
            }
            ++cy;
        }

        terminal_color(textFG);
        terminal_bkcolor(textBG);
        int top = 0;
        for (unsigned i = 0; i < inventoryHeight; ++i) {
            unsigned index = i + top;
            if (index < 0 || index >= player->inventory.mContents.size()) continue;
            int iqty            = player->inventory.mContents[i].qty;
            const ItemDef *idef = player->inventory.mContents[i].def;
            if (idef == nullptr) {
                terminal_printf(32, inventoryY + i, "    (bad item ident)");
            } else {
                terminal_printf(32, inventoryY + i, "%3d %s", iqty, idef->name.c_str());
            }
        }

        bool canMake = canMakeRecipe(w, current, player->inventory);
        if (current) {
            cy = 0;
            for (const RecipeRow &row : current->mRows) {
                const ItemDef &partDef = w.getItemDef(row.ident);
                if (player->inventory.qty(&partDef) < row.qty) {
                    terminal_color(invalidFG);
                } else {
                    terminal_color(validFG);
                    terminal_put(30, cy, '+');
                }
                if (partDef.ident >= 0) {
                    terminal_printf(32, cy, "%d %s", row.qty, partDef.name.c_str());
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
            case TK_DOWN:
                if (selection < static_cast<int>(list.size()) - 1) ++selection;
                break;
            case TK_UP:
                if (selection > 0) --selection;
                break;
            case TK_ENTER:
            case TK_KP_ENTER:
            case TK_C:
                if (current && canMake) {
                    for (const RecipeRow &row : current->mRows) {
                        const ItemDef &partDef = w.getItemDef(row.ident);
                        player->inventory.remove(&partDef, row.qty);
                    }
                    const ItemDef &makeDef = w.getItemDef(current->makeIdent);
                    player->inventory.add(&makeDef, current->makeQty);

                }
                break;
        }
    }


}

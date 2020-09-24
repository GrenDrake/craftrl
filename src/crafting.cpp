#include <BearLibTerminal.h>
#include "world.h"


void doCrafting(World &w, Actor *player) {

    const auto list = w.getRecipeList();

    int selection = 0;
    while (1) {
        const RecipeDef *current = nullptr;

        terminal_clear();
        int cy = 0;
        for (const auto *row : list) {
            if (!row) continue;
            const ItemDef &makeDef = w.getItemDef(row->makeIdent);
            if (cy == selection) {
                terminal_print(0, cy, "-->");
                terminal_color(0xFFFFFFFF);
                current = row;
            } else {
                terminal_color(0xFFAAAAAA);
            }
            if (makeDef.ident >= 0) {
                terminal_printf(4, cy, "%d %s", row->makeQty, makeDef.name.c_str());
            } else {
                terminal_printf(4, cy, "Bad item ident: %s", row->makeIdent);
            }
            ++cy;
        }

        terminal_color(0xFFFFFFFF);
        bool canMake = true;
        if (current) {
            cy = 0;
            for (const RecipeRow &row : current->mRows) {
                const ItemDef &partDef = w.getItemDef(row.ident);
                if (player->inventory.qty(&partDef) < row.qty) {
                    canMake = false;
                    terminal_color(0xFFFFAAAA);
                } else {
                    terminal_color(0xFFAAFFAA);
                }
                if (partDef.ident >= 0) {
                    terminal_printf(30, cy, "%d %s", row.qty, partDef.name.c_str());
                } else {
                    terminal_printf(30, cy, "Bad item ident: %s", row.ident);
                }
                ++cy;
            }
        }

        terminal_color(0xFFFFFFFF);
        terminal_printf(30, cy + 2, "Can make: %s", canMake ? "YES" : "NO");
        terminal_refresh();

        int key = terminal_read();
        switch(key) {
            case TK_Z:
                return;
            case TK_DOWN:
                ++selection;
                break;
            case TK_UP:
                --selection;
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

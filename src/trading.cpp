#include <BearLibTerminal.h>
#include "world.h"


void doTrading(World &w, Actor *left, Actor *right) {
    if (!left || !right) return;
    const unsigned highlightBG  = 0xFF666666;
    const unsigned highlightFG  = 0xFFFFFFFF;
    const unsigned textBG       = 0xFF000000;
    const unsigned textFG       = 0xFFAAAAAA;

    const int screenHeight = 24;

    int selection = 0;
    int side = 0;
    while (1) {

        terminal_bkcolor(textBG);
        terminal_color(textFG);
        terminal_clear();
        for (int y = 0; y < screenHeight; ++y) {
            terminal_put(39, y, LD_VERTICAL);
        }

        if (side) {
            terminal_color(textFG);
            terminal_bkcolor(textBG);
        } else {
            terminal_color(textBG);
            terminal_bkcolor(textFG);
        }
        terminal_printf(4, 24, left->getName().c_str());
        if (!side) {
            terminal_color(textFG);
            terminal_bkcolor(textBG);
        } else {
            terminal_color(textBG);
            terminal_bkcolor(textFG);
        }
        terminal_printf(44, 24, right->getName().c_str());

        int cy = 0;
        for (const auto &row : left->inventory.mContents) {
            if (cy == selection && !side) {
                terminal_color(highlightFG);
                terminal_bkcolor(highlightBG);
                terminal_clear_area(0, cy, 29, 1);
                terminal_put(0, cy, '>');
            } else {
                terminal_color(textFG);
                terminal_bkcolor(textBG);
            }
            terminal_put(2, cy, row.def->glyph);
            terminal_printf(4, cy, "%d %s", row.qty, row.def->name.c_str());
            ++cy;
        }

        cy = 0;
        for (const auto &row : right->inventory.mContents) {
            if (cy == selection && side) {
                terminal_color(highlightFG);
                terminal_bkcolor(highlightBG);
                terminal_clear_area(40, cy, 29, 1);
                terminal_put(40, cy, '>');
            } else {
                terminal_color(textFG);
                terminal_bkcolor(textBG);
            }
            terminal_put(42, cy, row.def->glyph);
            terminal_printf(44, cy, "%d %s", row.qty, row.def->name.c_str());
            ++cy;
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
                ++selection;
                break;
            case TK_KP_8:
            case TK_UP:
                --selection;
                break;
            case TK_END:
                if (side)   selection = right->inventory.size() - 1;
                else        selection = left->inventory.size() - 1;
                break;
            case TK_HOME:
                selection = 0;
                break;
            case TK_PAGEUP:
                selection -= 10;
                break;
            case TK_PAGEDOWN:
                selection += 10;
                break;
            case TK_TAB:
            case TK_KP_6:
            case TK_RIGHT:
            case TK_KP_4:
            case TK_LEFT:
                side = !side;
                break;
            case TK_R: {
                Inventory *l = &left->inventory;
                Inventory *r = &right->inventory;
                if (side) {
                    l = &right->inventory;
                    r = &left->inventory;
                }

                while (l->size() > 0) {
                    r->add(l->mContents[0].def, l->mContents[0].qty);
                    l->remove(l->mContents[0].def, l->mContents[0].qty);
                }
                break; }
            case TK_ENTER:
            case TK_KP_ENTER:
                if (selection < 0) break;
                if (!side) {
                    int qty = 1;
                    if (terminal_state(TK_SHIFT)) qty = left->inventory.mContents[selection].qty;
                    const ItemDef *idef = left->inventory.mContents[selection].def;
                    left->inventory.remove(idef, qty);
                    right->inventory.add(idef, qty);
                } else {
                    int qty = 1;
                    if (terminal_state(TK_SHIFT)) qty = right->inventory.mContents[selection].qty;
                    const ItemDef *idef = right->inventory.mContents[selection].def;
                    right->inventory.remove(idef, qty);
                    left->inventory.add(idef, qty);
                }
                break;
        }

        if (selection < 0) selection = 0;
        else if (!side && selection >= left->inventory.size()) selection = left->inventory.size() - 1;
        else if (side && selection >= right->inventory.size()) selection = right->inventory.size() - 1;

    }
}

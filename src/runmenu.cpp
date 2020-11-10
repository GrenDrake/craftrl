#include <BearLibTerminal.h>
#include "runmenu.h"

void setColourIfSelected(int selection, int position, bool enabled) {
    if (!enabled) {
        terminal_bkcolor(0xFF000000);
        terminal_color(0xFF777777);
    } else if (selection == position)  {
        terminal_color(0xFFFFFFFF);
        terminal_bkcolor(0xFF000000);
        terminal_print(3, 6 + position, ">");
        terminal_print(23, 6 + position, "<");
        terminal_bkcolor(0xFF777777);
        terminal_color(0xFFFFFFFF);
    } else {
        terminal_bkcolor(0xFF000000);
        terminal_color(0xFFAAAAAA);
    }
    terminal_clear_area(5, 6 + position, 17, 1);
}


int runMenu(Menu &menu) {
    while (1) {
        terminal_bkcolor(0xFF000000);
        terminal_color(0xFFFFFFFF);
        terminal_clear();
        terminal_print(5, 3, menu.titleLeft.c_str());
        terminal_print(75 - menu.titleRight.size(), 3, menu.titleRight.c_str());

        for (unsigned i = 0; i < menu.items.size(); ++i) {
            const MenuItem &item = menu.items[i];
            if (item.type == MENU_SPACE) continue;
            setColourIfSelected(menu.selection, i, !item.disabled);
            terminal_print(5, 6 + i, item.text.c_str());
            if (item.type == MENU_TEXT) {
                terminal_print(25, 6 + i, "______________________________");
                terminal_print(25, 6 + i, item.strValue.c_str());
            } else if (item.type == MENU_INT) {
                terminal_printf(25, 6 + i, "%d", item.intValue);
            }
        }
        terminal_refresh();

        MenuItem &currentItem = menu.items[menu.selection];
        int key = terminal_read();
        switch (key) {
            case TK_CLOSE:
            case TK_ESCAPE:
                return MENU_CANCELED;
            case TK_ENTER:
            case TK_KP_ENTER:
                if (currentItem.type == MENU_SELECT) {
                    return menu.items[menu.selection].retCode;
                }
            case TK_SPACE:
                if (currentItem.type == MENU_TEXT) {
                    currentItem.strValue += ' ';
                } else if (currentItem.type == MENU_SELECT) {
                    return menu.items[menu.selection].retCode;
                }
                break;
            case TK_DOWN: {
                int newItem = menu.selection + 1;
                while (menu.items[newItem].disabled && newItem < static_cast<int>(menu.items.size()) - 1) {
                    ++newItem;
                }
                if (!menu.items[newItem].disabled && newItem < static_cast<int>(menu.items.size())) menu.selection = newItem;
                break; }
            case TK_UP: {
                if (menu.selection <= 0) break;
                int newItem = menu.selection - 1;
                while (newItem > 0 && menu.items[newItem].disabled) {
                    --newItem;
                }
                if (!menu.items[newItem].disabled) menu.selection = newItem;
                break; }

            case TK_LEFT:
                if (currentItem.type == MENU_INT) {
                    if (currentItem.intValue > currentItem.minValue) --currentItem.intValue;
                }
                break;
            case TK_RIGHT:
                if (currentItem.type == MENU_INT) {
                    if (currentItem.intValue < currentItem.maxValue) ++currentItem.intValue;
                }
                break;

            case TK_BACKSPACE:
                if (currentItem.type == MENU_TEXT) {
                    if (currentItem.strValue.size() > 0) currentItem.strValue.erase(currentItem.strValue.end() - 1);
                } else if (currentItem.type == MENU_INT) {
                    if (currentItem.intValue != 0) currentItem.intValue /= 10;
                    if (currentItem.intValue < currentItem.minValue) currentItem.intValue = currentItem.minValue;
                }
                break;
            default:
                if (currentItem.type == MENU_TEXT && currentItem.strValue.size() < 30) {
                    int ch = terminal_state(TK_CHAR);
                    if (ch >= 32 && ch <= 127) {
                        currentItem.strValue += static_cast<char>(ch);
                    }
                } else if (currentItem.type == MENU_INT) {
                    int ch = terminal_state(TK_CHAR);
                    if (ch >= '0' && ch <= '9') {
                        ch -= '0';
                        currentItem.intValue *= 10;
                        currentItem.intValue += ch;
                        if (currentItem.intValue > currentItem.maxValue) currentItem.intValue = currentItem.maxValue;
                    }
                }
        }
    }

    return MENU_CANCELED;
}
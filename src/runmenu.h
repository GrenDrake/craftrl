#ifndef RUNMENU_H
#define RUNMENU_H

#include <string>
#include <vector>

const int MENU_CANCELED     = -1;
const int MENU_SELECT       = 0;
const int MENU_TEXT         = 1;
const int MENU_SPACE        = 3;

struct MenuItem {
    std::string text;
    int retCode;
    bool disabled;
    int type;
    std::string strValue;
    int intValue;
};

struct Menu {
    std::string titleLeft;
    std::string titleRight;
    std::vector<MenuItem> items;
    int selection;
};

int runMenu(Menu &menu);

#endif

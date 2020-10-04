#include <sstream>
#include <BearLibTerminal.h>
#include "world.h"


const Command gameCommands[] = {
    {   CMD_QUIT,           Dir::None,      { { TK_Q,        }, { TK_ESCAPE } } },
    {   CMD_CANCEL,         Dir::None,      { { TK_Z,        },  } },
    {   CMD_DUMPMAP,        Dir::None,      { { TK_F5,       },  } },
    {   CMD_TAKE,           Dir::None,      { { TK_G,        },  } },
    {   CMD_BREAK,          Dir::None,      { { TK_A,        },  } },
    {   CMD_DO,             Dir::None,      { { TK_E,        },  } },
    {   CMD_MOVE,           Dir::None,      { { TK_M,        },  } },
    {   CMD_PAN,            Dir::None,      { { TK_P,        },  } },
    {   CMD_RESETVIEW,      Dir::None,      { { TK_R,        },  } },
    {   CMD_DROP,           Dir::None,      { { TK_D,        },  } },
    {   CMD_TALK,           Dir::None,      { { TK_T,        },  } },
    {   CMD_USE,            Dir::None,      { { TK_ENTER,    }, { TK_KP_ENTER} } },
    {   CMD_WAIT,           Dir::None,      { { TK_SPACE     }, { TK_KP_5 } } },
    {   CMD_CRAFT,          Dir::None,      { { TK_C,        },  } },
    {   CMD_SAVE,           Dir::None,      { { TK_F2,       },  } },
    {   CMD_PREV_SELECT,    Dir::None,      { { TK_LBRACKET, }, { TK_KP_MINUS } } },
    {   CMD_NEXT_SELECT,    Dir::None,      { { TK_RBRACKET, }, { TK_KP_PLUS  } } },
    {   CMD_SELECT_PGDN,    Dir::None,      { { TK_PAGEDOWN  } } },
    {   CMD_SELECT_PGUP,    Dir::None,      { { TK_PAGEUP    } } },
    {   CMD_SELECT_HOME,    Dir::None,      { { TK_HOME      } } },
    {   CMD_SELECT_END,     Dir::None,      { { TK_END       } } },
    {   CMD_SORT_INV_NAME,  Dir::None,      { { TK_O         } } },
    {   CMD_DEBUG,          Dir::None,      { { TK_F7        } } },

    {   CMD_CONTEXTMOVE,    Dir::North,     { { TK_UP,       }, { TK_K }, { TK_KP_8 } } },
    {   CMD_CONTEXTMOVE,    Dir::East,      { { TK_RIGHT,    }, { TK_L }, { TK_KP_6 } } },
    {   CMD_CONTEXTMOVE,    Dir::South,     { { TK_DOWN,     }, { TK_J }, { TK_KP_2 } } },
    {   CMD_CONTEXTMOVE,    Dir::West,      { { TK_LEFT,     }, { TK_H }, { TK_KP_4 } } },
    {   CMD_CONTEXTMOVE,    Dir::Southwest, { { },              { TK_B }, { TK_KP_1 } } },
    {   CMD_CONTEXTMOVE,    Dir::Southeast, { { },              { TK_N }, { TK_KP_3 } } },
    {   CMD_CONTEXTMOVE,    Dir::Northeast, { { },              { TK_U }, { TK_KP_9 } } },
    {   CMD_CONTEXTMOVE,    Dir::Northwest, { { },              { TK_Y }, { TK_KP_7 } } },

    {   CMD_NONE,           Dir::None,      { { 0,           },  } },
};

const Command BAD_COMMAND  = { CMD_NONE };
const Command QUIT_COMMAND = { CMD_QUIT };
const Command& findCommand(int key, const Command *commandList) {
    if (key == TK_CLOSE) return QUIT_COMMAND;

    // int ctrl = terminal_check(TK_CTRL);
    // int shift = terminal_check(TK_SHIFT);
    // int alt = terminal_check(TK_ALT);

    for (unsigned int i = 0; commandList[i].command >= 0; ++i) {
        for (int j = 0; j < INPUT_KEY_COUNT; ++j) {
            if (commandList[i].key[j].key == key) {
                return commandList[i];
            }
        }
    }
    return BAD_COMMAND;
}



std::string commandName(int command) {
    switch(command) {
        case CMD_NONE:          return "None";
        case CMD_DUMPMAP:       return "Dump map to file";
        case CMD_TAKE:          return "Take";
        case CMD_BREAK:         return "Break";
        case CMD_MOVE:          return "Move";
        case CMD_PAN:           return "Pan";
        case CMD_RESETVIEW:     return "Centre View";
        case CMD_DROP:          return "Drop";
        case CMD_USE:           return "Use";
        case CMD_WAIT:          return "Wait";
        case CMD_TALK:          return "Talk";
        case CMD_CONTEXTMOVE:   return "Context-Sensitive Move";
        case CMD_QUIT:          return "Quit";
        case CMD_NEXT_SELECT:   return "Inventory Down";
        case CMD_PREV_SELECT:   return "Inventory Up";
        case CMD_CANCEL:        return "Cancel";
        case CMD_CRAFT:         return "Craft";
        case CMD_SAVE:          return "Save Game";
        case CMD_DO:            return "Do";
        case CMD_SELECT_PGDN:   return "Pagedown Inventory";
        case CMD_SELECT_PGUP:   return "Pageup Inventory";
        case CMD_SELECT_HOME:   return "Inventory First";
        case CMD_SELECT_END:    return "Inventory Last";
        case CMD_SORT_INV_NAME: return "Sort Inventory by Name";
        case CMD_DEBUG:         return "Debug Command";
        default: {
            std::stringstream s;
            s << "(Unknown Command " << command << ')';
            return s.str(); }
    }
}

ActionHandler commandAction(int command) {
    switch (command) {
        case CMD_DUMPMAP:       return actionDumpMap;
        case CMD_TAKE:          return actionTake;
        case CMD_BREAK:         return actionAttack;
        case CMD_MOVE:          return actionMove;
        case CMD_PAN:           return actionPan;
        case CMD_RESETVIEW:     return actionCentrePan;
        case CMD_DROP:          return actionDrop;
        case CMD_USE:           return actionUse;
        case CMD_WAIT:          return actionWait;
        case CMD_TALK:          return actionTalkActor;
        case CMD_CONTEXTMOVE:   return actionContextMove;
        case CMD_QUIT:          return actionQuit;
        case CMD_NEXT_SELECT:   return actionNextSelect;
        case CMD_PREV_SELECT:   return actionPrevSelect;
        case CMD_CRAFT:         return actionCraft;
        case CMD_DO:            return actionDo;
        case CMD_SAVE:          return actionSavegame;
        case CMD_SELECT_PGDN:   return actionSelectPagedown;
        case CMD_SELECT_PGUP:   return actionSelectPageup;
        case CMD_SELECT_HOME:   return actionSelectHome;
        case CMD_SELECT_END:    return actionSelectEnd;
        case CMD_SORT_INV_NAME: return actionSortInvByName;
        case CMD_DEBUG:         return actionDebug;
        default:                return nullptr;
    }
}
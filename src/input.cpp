#include <sstream>
#include <BearLibTerminal.h>
#include "world.h"

const Command gameCommands[] = {
    {   CMD_QUIT,           Dir::None,      { { TK_Q,    }, { TK_ESCAPE }, { TK_CLOSE } } },
    {   CMD_CANCEL,         Dir::None,      { { TK_Z,        },  } },
    {   CMD_DUMPMAP,        Dir::None,      { { TK_F5,       },  } },
    {   CMD_TAKE,           Dir::None,      { { TK_G,        },  } },
    {   CMD_BREAK,          Dir::None,      { { TK_A,        },  } },
    {   CMD_DO,             Dir::None,      { { TK_E,        },  } },
    {   CMD_MOVE,           Dir::None,      { { TK_M,        },  } },
    {   CMD_PAN,            Dir::None,      { { TK_P,        },  } },
    {   CMD_RESETVIEW,      Dir::None,      { { TK_R,        },  } },
    {   CMD_DROP,           Dir::None,      { { TK_D,        },  } },
    {   CMD_USE,            Dir::None,      { { TK_ENTER,    }, { TK_KP_ENTER} } },
    {   CMD_WAIT,           Dir::None,      { { TK_SPACE     }, { TK_KP_5 } } },
    {   CMD_CRAFT,          Dir::None,      { { TK_C,        },  } },
    {   CMD_PREV_SELECT,    Dir::None,      { { TK_LBRACKET, }, { TK_KP_MINUS } } },
    {   CMD_NEXT_SELECT,    Dir::None,      { { TK_RBRACKET, }, { TK_KP_PLUS  } } },
    {   CMD_SAVE,           Dir::None,      { { TK_F2,       },  } },

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

const Command BAD_COMMAND = {   CMD_NONE, Dir::None, { { TK_INPUT_NONE, false, false, false } } };
const Command& findCommand(int key, const Command *commandList) {
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
        case CMD_CONTEXTMOVE:   return "Context-Sensitive Move";
        case CMD_QUIT:          return "Quit";
        case CMD_NEXT_SELECT:   return "Next Inventory";
        case CMD_PREV_SELECT:   return "Previous Inventory";
        case CMD_CANCEL:        return "Cancel";
        case CMD_CRAFT:         return "Craft";
        case CMD_SAVE:          return "Save Game";
        case CMD_DO:            return "Do";
        default: {
            std::stringstream s;
            s << "(Unknown Command " << command << ')';
            return s.str(); }
    }
}

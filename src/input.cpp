#include <BearLibTerminal.h>
#include "world.h"

const Command gameCommands[] = {
    {   TK_CLOSE,       false,  false,  false,  CMD_QUIT,           Dir::None },
    {   TK_ESCAPE,      false,  false,  false,  CMD_QUIT,           Dir::None },
    {   TK_Q,           false,  false,  false,  CMD_QUIT,           Dir::None },
    {   TK_Z,           false,  false,  false,  CMD_CANCEL,         Dir::None },

    {   TK_F5,          false,  false,  false,  CMD_DUMPMAP,        Dir::None },
    {   TK_G,           false,  false,  false,  CMD_TAKE,           Dir::None },
    {   TK_A,           false,  false,  false,  CMD_BREAK,          Dir::None },
    {   TK_M,           false,  false,  false,  CMD_MOVE,           Dir::None },
    {   TK_P,           false,  false,  false,  CMD_PAN,            Dir::None },
    {   TK_R,           false,  false,  false,  CMD_RESETVIEW,      Dir::None },
    {   TK_D,           false,  false,  false,  CMD_DROP,           Dir::None },
    {   TK_ENTER,       false,  false,  false,  CMD_USE,            Dir::None },
    {   TK_KP_ENTER,    false,  false,  false,  CMD_USE,            Dir::None },
    {   TK_KP_5,        false,  false,  false,  CMD_WAIT,           Dir::None },
    {   TK_SPACE,       false,  false,  false,  CMD_WAIT,           Dir::None },
    {   TK_LBRACKET,    false,  false,  false,  CMD_PREV_SELECT,    Dir::None },
    {   TK_KP_MINUS,    false,  false,  false,  CMD_PREV_SELECT,    Dir::None },
    {   TK_RBRACKET,    false,  false,  false,  CMD_NEXT_SELECT,    Dir::None },
    {   TK_KP_PLUS,     false,  false,  false,  CMD_NEXT_SELECT,    Dir::None },
    {   TK_C,           false,  false,  false,  CMD_CRAFT,          Dir::None },

    {   TK_UP,          false,  false,  false,  CMD_CONTEXTMOVE,    Dir::North },
    {   TK_RIGHT,       false,  false,  false,  CMD_CONTEXTMOVE,    Dir::East },
    {   TK_DOWN,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::South },
    {   TK_LEFT,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::West },
    {   TK_K,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::North },
    {   TK_L,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::East },
    {   TK_J,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::South },
    {   TK_H,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::West },
    {   TK_B,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Southwest },
    {   TK_N,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Southeast },
    {   TK_Y,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Northeast },
    {   TK_U,           false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Northwest },

    {   TK_KP_1,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Southwest },
    {   TK_KP_2,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::South },
    {   TK_KP_3,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Southeast },
    {   TK_KP_6,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::East },
    {   TK_KP_9,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Northeast },
    {   TK_KP_8,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::North },
    {   TK_KP_7,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::Northwest },
    {   TK_KP_4,        false,  false,  false,  CMD_CONTEXTMOVE,    Dir::West },

    {   TK_F2,          false,  false,  false,  CMD_SAVE,           Dir::None },

    {   0,              false,  false,  false,  CMD_NONE,           Dir::None }
};

const Command BAD_COMMAND = {   TK_INPUT_NONE, false, false, false, CMD_NONE, Dir::None };
const Command& findCommand(int key, const Command *commandList) {
    // int ctrl = terminal_check(TK_CTRL);
    // int shift = terminal_check(TK_SHIFT);
    // int alt = terminal_check(TK_ALT);

    for (unsigned int i = 0; commandList[i].command >= 0; ++i) {
        if (commandList[i].key == key) {
            return commandList[i];
        }
    }
    return BAD_COMMAND;
}

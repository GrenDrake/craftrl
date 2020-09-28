#include <BearLibTerminal.h>



void ui_MessageBox(const std::string &title, const std::string &message) {
    const int screenWidth = 80;
    const int screenHeight = 24;
    const int msgLength = message.size();
    const int titleLength = title.size();
    const int maxLength = titleLength > msgLength ? titleLength : msgLength;
    const int boxWidth = maxLength + 4;
    const int boxHeight = 5;
    const int titleX = (screenWidth - titleLength) / 2;
    const int boxX = (screenWidth - boxWidth) / 2;
    const int boxY = (screenHeight - boxHeight) / 2;

    terminal_bkcolor(0xFF333333);
    terminal_color(0xFFFFFFFF);
    terminal_clear_area(boxX, boxY, boxWidth, boxHeight);

    for (int y = boxY + 1; y < boxY + boxHeight; ++y) {
        terminal_put(boxX, y, '|');
        terminal_put(boxX + boxWidth - 1, y, '|');
    }
    for (int x = boxX + 1; x < boxX + boxWidth; ++x) {
        terminal_put(x, boxY, '-');
        terminal_put(x, boxY + boxHeight - 1, '-');
    }
    terminal_put(boxX, boxY, '+');
    terminal_put(boxX, boxY + boxHeight - 1, '+');
    terminal_put(boxX + boxWidth - 1, boxY, '+');
    terminal_put(boxX + boxWidth - 1, boxY + boxHeight - 1, '+');

    terminal_print(titleX, boxY, (" " + title + " ").c_str());
    terminal_print(boxX + 2, boxY + 2, message.c_str());

    terminal_bkcolor(0xFFFFFFFF);
    terminal_color(0xFF333333);
    terminal_print(boxX + boxWidth - 8, boxY + boxHeight - 1, " OKAY ");
    terminal_refresh();
    while (1) {
        int key = terminal_read();
        if (key == TK_MOUSE_LEFT) {
            int mx = terminal_state(TK_MOUSE_X);
            int my = terminal_state(TK_MOUSE_Y);
            if (my != boxY + boxHeight - 1) continue;
            if (mx < boxX + boxWidth - 8 || mx > boxX + boxWidth - 3) continue;
            break;
        }
        if (key == TK_MOUSE_RIGHT || key == TK_MOUSE_MIDDLE) continue;
        break;
    }
}
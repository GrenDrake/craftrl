#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "world.h"

ConfigData configRead(const std::string &filename) {
    std::stringstream filetext(readFile(filename));

    ConfigData data = {
        80, 25 // screen width, height
    };

    unsigned lineNumber = 0;
    std::string line;
    while (std::getline(filetext, line)) {
        ++lineNumber;
        auto pos = line.find_first_of('=');
        if (pos == std::string::npos) {
            logger_log(filename + ":" + std::to_string(lineNumber) + " invalid config line.");
            continue;
        }

        std::string field = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        if (field.empty()) {
            logger_log(filename + ":" + std::to_string(lineNumber) + " Empty config value name.");
        } else if (field == "screenWidth") {
            if (!strToInt(value, data.screenWidth)) logger_log(filename + ":" + std::to_string(lineNumber) + " Screen width is not valid number.");
            if (data.screenWidth < 80) {
                data.screenWidth = 80;
                logger_log(filename + ":" + std::to_string(lineNumber) + " Screen width must be at least 80.");
            }
        } else if (field == "screenHeight") {
            if (!strToInt(value, data.screenHeight)) logger_log(filename + ":" + std::to_string(lineNumber) + " Screen height is not valid number.");
            if (data.screenHeight < 24) {
                data.screenHeight = 24;
                logger_log(filename + ":" + std::to_string(lineNumber) + " Screen height must be at least 24.");
            }
        } else {
            logger_log(filename + ":" + std::to_string(lineNumber) + " Unknown config value " + field + ".");
        }
    }

    return data;
}


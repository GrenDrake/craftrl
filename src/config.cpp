#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <physfs.h>

#include "world.h"
#include "logger.h"

std::string trim(std::string s);

std::string readFile(const std::string &filename) {
    PHYSFS_File *fp = PHYSFS_openRead(filename.c_str());
    if (!fp) {
        logger_log("Failed to open file " + filename + ".");
        return "";
    }
    auto filesize = PHYSFS_fileLength(fp);
    char *buffer = new char[filesize];
    buffer[0] = 0;
    auto bytesRead = PHYSFS_readBytes(fp, buffer, filesize);
    if (bytesRead < filesize) {
        logger_log("Incomplete read of file " + filename + ".");
    }
    std::string text = buffer;
    delete[] buffer;
    return text;
}

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


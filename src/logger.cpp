#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <physfs.h>

void logger_setFile(const std::string &filename);
void logger_close();
void logger_log(const std::string &msg);

static PHYSFS_file *logFile = nullptr;

void logger_setFile(const std::string &filename) {
    if (logFile) {
        PHYSFS_close(logFile);
        logFile = nullptr;
    }
    logFile = PHYSFS_openWrite(filename.c_str());
    if (!logFile) {
        logger_log("Failed to allocate log file.");
        return;
    }
    logger_log("Opened log file.");
}

void logger_close() {
    logger_log("Closing log file.");
    PHYSFS_close(logFile);
    logFile = nullptr;
}

void logger_log(const std::string &msg) {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y-%m-%d %H:%M  ", timeinfo);

    if (logFile) {
        PHYSFS_writeBytes(logFile, buffer, strlen(buffer));
        PHYSFS_writeBytes(logFile, msg.c_str(), msg.size());
        PHYSFS_writeBytes(logFile, "\n", 1);
    } else {
        std::cerr  << buffer << "  " << msg << '\n';
    }
}

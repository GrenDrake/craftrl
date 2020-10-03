#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

void logger_setFile(const std::string &filename);
void logger_close();
void logger_log(const std::string &msg);

static std::ofstream *logFile = nullptr;

void logger_setFile(const std::string &filename) {
    if (logFile) delete logFile;
    logFile = new std::ofstream;
    if (!logFile) {
        logger_log("Failed to allocate log file.");
        return;
    }
    logFile->rdbuf()->pubsetbuf(nullptr, 0); // disable output buffering
    logFile->open(filename);
    if (!logFile->is_open()) {
        delete logFile;
        logger_log("Failed to open log file.");
    } else {
        logger_log("Opened log file.");
    }
}

void logger_close() {
    logger_log("Closing log file.");
    if (logFile) delete logFile;
}

void logger_log(const std::string &msg) {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y-%m-%d %H:%M.", timeinfo);

    if (logFile)    (*logFile) << buffer << "  " << msg << '\n';
    else            std::cerr  << buffer << "  " << msg << '\n';
}

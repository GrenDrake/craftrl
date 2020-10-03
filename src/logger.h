#ifndef LOGGER_H
#define LOGGER_H

void logger_setFile(const std::string &filename);
void logger_close();
void logger_log(const std::string &msg);

#endif

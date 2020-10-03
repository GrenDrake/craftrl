#ifndef TEST_H
#define TEST_H

#include <string>

bool requireString(const std::string &testname, const std::string &left, const std::string &right);
bool requireUnsignedLongLong(const std::string &testname, unsigned long long left, unsigned long long right);

#endif

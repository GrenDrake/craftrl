#include <iostream>
#include <string>
#include "test.h"


bool requireInt(const std::string &testname, int left, int right) {
    if (left == right) return true;
    std::cout << "Test " << testname << " failed; expected " << right << ", but found ";
    std::cout << left << ".\n";
    return false;
}

bool requireString(const std::string &testname, const std::string &left, const std::string &right) {
    if (left == right) return true;
    std::cout << "Test " << testname << " failed; expected \"" << right << "\", but found \"";
    std::cout << left << "\".\n";
    return false;
}

bool requireUnsignedLongLong(const std::string &testname, unsigned long long left, unsigned long long right) {
    if (left == right) return true;
    std::cout << "Test " << testname << " failed; expected " << right << ", but found ";
    std::cout << left << ".\n";
    return false;
}

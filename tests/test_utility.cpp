#include <iostream>
#include <string>
#include "test.h"


unsigned long long hashString(const std::string &str);
std::string upperFirst(std::string text);



bool testUpperFirst() {
    std::cout << "Testing upperFirst.\n";

    if (!requireString("empty string", upperFirst(""), "" )) return false;
    if (!requireString("all lowercase", upperFirst("the world"), "The world" )) return false;
    if (!requireString("mixed case", upperFirst("The World"), "The World")) return false;
    if (!requireString("uppercase", upperFirst("THE WORLD"), "THE WORLD")) return false;
    return true;
}


bool testStringHash() {
    std::cout << "Testing stringHash.\n";

    if (!requireUnsignedLongLong("empty string", hashString(""), 14695981039346656037ull)) return false;
    if (!requireUnsignedLongLong("string \"hello\"", hashString("hello"), 11831194018420276491ull)) return false;
    return true;
}



int main() {

    if (!testUpperFirst()) return 1;
    if (!testStringHash()) return 1;
    std::cout << "All tests passed.\n";

    return 0;
}

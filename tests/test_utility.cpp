#include <iostream>
#include <string>
#include <vector>
#include "test.h"


unsigned long long hashString(const std::string &str);
std::vector<std::string> explode(const std::string &text);
bool strToInt(const std::string &str, int &number);
std::string trim(std::string s);
std::string upperFirst(std::string text);



bool testExplode() {
    std::cout << "Testing explode.\n";

    if (!requireInt("empty string", explode("").size(), 0)) return false;

    auto t1 = explode("world");
    if (!requireInt("one word, result size", t1.size(), 1)) return false;
    if (!requireString("one word, word unchanged", t1[0], "world")) return false;

    auto t2 = explode("  world  ");
    if (!requireInt("one word w/ whitespace, result size", t2.size(), 1)) return false;
    if (!requireString("one word w/ whitespace, word unchanged", t2[0], "world")) return false;

    auto t3 = explode("the world");
    if (!requireInt("two words, result size", t3.size(), 2)) return false;
    if (!requireString("two words, first word", t3[0], "the")) return false;
    if (!requireString("two words, second word", t3[1], "world")) return false;

    auto t4 = explode("   the   world   ");
    if (!requireInt("two words, result size", t4.size(), 2)) return false;
    if (!requireString("two words w/ whitespace, first word", t4[0], "the")) return false;
    if (!requireString("two words w/ whitespace, second word", t4[1], "world")) return false;

    auto t5 = explode("the world a great");
    if (!requireInt("four words, result size", t5.size(), 4)) return false;
    if (!requireString("four words, first word", t5[0], "the")) return false;
    if (!requireString("four words, second word", t5[1], "world")) return false;
    if (!requireString("four words, third word", t5[2], "a")) return false;
    if (!requireString("four words, fourth word", t5[3], "great")) return false;

    auto t6 = explode("   the   world   is   great   ");
    if (!requireInt("four words, result size", t6.size(), 4)) return false;
    if (!requireString("four words w/ whitespace, first word", t6[0], "the")) return false;
    if (!requireString("four words w/ whitespace, second word", t6[1], "world")) return false;
    if (!requireString("four words w/ whitespace, third word", t6[2], "is")) return false;
    if (!requireString("four words w/ whitespace, fourth word", t6[3], "great")) return false;

    return true;
}

bool testStringHash() {
    std::cout << "Testing stringHash.\n";

    if (!requireUnsignedLongLong("empty string", hashString(""), 14695981039346656037ull)) return false;
    if (!requireUnsignedLongLong("string \"hello\"", hashString("hello"), 11831194018420276491ull)) return false;
    return true;
}

bool testStrToInt() {
    std::cout << "Testing strToInt.\n";
    int result = 0;

    if (!requireInt("empty string", strToInt("", result), 0)) return false;
    if (!requireInt("not a number", strToInt("abc", result), 0)) return false;
    if (!requireInt("mixed - number on left", strToInt("123abc", result), 0)) return false;
    if (!requireInt("mixed - number on right", strToInt("abc123", result), 0)) return false;

    if (!requireInt("\"5\" is strue", strToInt("5", result), 1)) return false;
    if (!requireInt("\"5\" returns right value", result, 5)) return false;
    if (!requireInt("\"  725  \" is true", strToInt("  725  ", result), 1)) return false;
    if (!requireInt("\"  725  \" returns right value", result, 725)) return false;
    if (!requireInt("\"-14\" is strue", strToInt("-14", result), 1)) return false;
    if (!requireInt("\"-14\" returns right value", result, -14)) return false;
    return true;
}

bool testTrim() {
    std::cout << "Testing trim.\n";

    if (!requireString("empty string", trim(""), "")) return false;
    if (!requireString("no whitespace", trim("the  world"), "the  world")) return false;
    if (!requireString("initial whitespace", trim("   the  world"), "the  world")) return false;
    if (!requireString("final whitespace", trim("the  world   "), "the  world")) return false;
    if (!requireString("dual whitespace", trim("   the  world   "), "the  world")) return false;
    return true;
}

bool testUpperFirst() {
    std::cout << "Testing upperFirst.\n";

    if (!requireString("empty string", upperFirst(""), "" )) return false;
    if (!requireString("all lowercase", upperFirst("the world"), "The world" )) return false;
    if (!requireString("mixed case", upperFirst("The World"), "The World")) return false;
    if (!requireString("uppercase", upperFirst("THE WORLD"), "THE WORLD")) return false;
    return true;
}



int main() {

    if (!testExplode())     return 1;
    if (!testStringHash())  return 1;
    if (!testStrToInt())    return 1;
    if (!testTrim())        return 1;
    if (!testUpperFirst())  return 1;
    std::cout << "All tests passed.\n";

    return 0;
}

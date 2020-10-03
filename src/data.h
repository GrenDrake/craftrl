#ifndef DATA_H
#define DATA_H

#include <map>
#include <iosfwd>
#include <string>
#include <vector>

class World;

enum class TokenType {
    Identifier, String, Integer, OpenBrace, CloseBrace, Semicolon,
    Invalid
};

struct Origin {
    std::string toString() const;
    std::string filename;
    int line;
};

struct Token {
    Origin origin;
    TokenType type;
    int i;
    std::string s;
};

struct TokenData {
    TokenData();

    bool end() const;
    const Token& here() const;
    const Token& next();

    void skipTo(TokenType type);
    bool require(TokenType type);
    bool matches(TokenType type) const;
    bool matches(const std::string &identifier) const;

    bool asInt(int &value) const;

    bool valid;
    unsigned pos = 0;
    std::vector<Token> *tokens;
    std::vector<std::string> fileList;
    std::map<std::string, int> symbols;
};

bool loadGameData(World &w, const std::string &filename);
int loadGameData_Core(World &w, TokenData &data, const std::string &filename);
std::vector<Token> parseFile(const std::string &filename);

std::ostream& operator<<(std::ostream &out, const TokenType &type);
std::ostream& operator<<(std::ostream &out, const Token &token);

#endif

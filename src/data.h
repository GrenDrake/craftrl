#ifndef DATA_H
#define DATA_H

#include <map>
#include <iosfwd>
#include <string>
#include <vector>

enum class TokenType {
    Identifier, String, Integer, OpenBrace, CloseBrace, Semicolon,
    Invalid
};

struct Origin {
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
    TokenData(const std::vector<Token> &tokens);

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
    std::vector<Token> tokens;
    std::map<std::string, int> symbols;
};

TokenData parseFile(const std::string &filename);

std::ostream& operator<<(std::ostream &out, const TokenType &type);
std::ostream& operator<<(std::ostream &out, const Token &token);



#endif

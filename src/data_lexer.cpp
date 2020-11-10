#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <physfs.h>

#include "data.h"
#include "logger.h"


bool is_space(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_digit(int c) {
    return c >= '0' && c <= '9';
}

bool is_alpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_alnum(int c) {
    return is_digit(c) || is_alpha(c);
}

bool is_identifier(int c) {
    return is_digit(c) || is_alpha(c) || c == '-' || c == '_';
}

std::string Origin::toString() const {
    std::stringstream s;
    s << filename;
    if (line > 0) s << ':' << line;
    return s.str();
}

std::ostream& operator<<(std::ostream &out, const TokenType &type) {
    switch(type) {
        case TokenType::Identifier: out << "Identifier";    break;
        case TokenType::String:     out << "String";        break;
        case TokenType::Integer:    out << "Integer";       break;
        case TokenType::OpenBrace:  out << "Open Brace";    break;
        case TokenType::CloseBrace: out << "Close Brace";   break;
        case TokenType::Invalid:    out << "Invalid";       break;
        case TokenType::Semicolon:  out << "Semicolon";       break;
        default:
            out << "(unknown token type " << static_cast<int>(type) << ')';
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const Token &token) {
    out << '[' << token.origin.filename << ':' << token.origin.line << "  " << token.type;
    if (token.type == TokenType::Integer)           out << "; " << token.i;
    else if (token.type == TokenType::Identifier)   out << "; " << token.s;
    else if (token.type == TokenType::String)       out << "; \"" << token.s << '"';
    out << ']';
    return out;
}


TokenData::TokenData()
: valid(true), pos(0), tokens(nullptr) {
}

bool TokenData::end() const {
    return pos >= tokens->size();
}

const Token BAD_TOKEN{ Origin{}, TokenType::Invalid };

const Token& TokenData::here() const {
    if (pos < tokens->size()) return (*tokens)[pos];
    return BAD_TOKEN;
}

const Token& TokenData::next() {
    if (pos < tokens->size()) ++pos;
    return here();
}

void TokenData::skipTo(TokenType type) {
    while (pos < tokens->size() && !matches(type)) ++pos;
}

bool TokenData::require(TokenType type) {
    if (matches(type)) {
        return true;
    }
    std::stringstream s;
    s << here().origin.toString() + "  Expected " << type << " but found " << here().type << ".";
    logger_log(s.str());
    return false;
}

bool TokenData::matches(TokenType type) const {
    const Token &t = here();
    return t.type == type;
}

bool TokenData::matches(const std::string &identifier) const {
    if (!matches(TokenType::Identifier) || here().s != identifier) return false;
    return true;
}

bool TokenData::asInt(int &value) const {
    if (matches(TokenType::Integer)) {
        value = here().i;
        return true;
    }

    if (matches(TokenType::Identifier)) {
        auto iter = symbols.find(here().s);
        if (iter == symbols.end()) {
            logger_log(here().origin.toString() + "  Undefined symbol " + here().s + ".");
            return false;
        }
        value = iter->second;
        return true;
    }

    std::stringstream s;
    s << here().origin.toString() + "  Expected Integer but found " << here().type << ".";
    logger_log(s.str());
    return false;
}

int readLine(PHYSFS_file *file, std::string &line) {
    if (!file || PHYSFS_eof(file)) return 0;
    line = "";
    while (1) {
        char byte;
        int len = PHYSFS_readBytes(file, &byte, 1);
        if (len != 1) return 1;
        if (byte == '\n') return 1;
        if (byte == '\r') continue;
        line += byte;
    }
}

std::vector<Token> parseFile(const std::string &filename) {
    std::vector<Token> tokens;
    std::string line;
    PHYSFS_File *inf = PHYSFS_openRead(("/data/" + filename).c_str());

    int errorCount = 0;
    int lineNo = 0;
    while (readLine(inf, line)) {
        ++lineNo;

        std::string::size_type pos = 0;
        while (pos < line.size()) {

            if (is_space(line[pos])) {
                ++pos;
                continue;
            } else if (line[pos] == '{') {
                tokens.push_back(Token{Origin{filename, lineNo}, TokenType::OpenBrace});
                ++pos;
            } else if (line[pos] == '}') {
                tokens.push_back(Token{Origin{filename, lineNo}, TokenType::CloseBrace});
                ++pos;
            } else if (line[pos] == ';') {
                tokens.push_back(Token{Origin{filename, lineNo}, TokenType::Semicolon});
                ++pos;
            } else if (line[pos] == '\'' || line[pos] == '"') {
                Origin origin{filename, lineNo};
                int quoteChar = line[pos];
                ++pos;
                auto start = pos;
                while (pos < line.size() && line[pos] != quoteChar) ++pos;
                std::string text = line.substr(start, pos - start);
                ++pos;

                if (quoteChar == '\'') {
                    if (text.size() != 1) {
                        logger_log(filename + ":" + std::to_string(lineNo) + "  Bad character literal length.");
                        ++errorCount;
                    } else {
                        tokens.push_back(Token{origin, TokenType::Integer, text[0]});
                    }
                } else {
                    tokens.push_back(Token{origin, TokenType::String, 0, text});
                }
            } else if (is_identifier(line[pos])) {
                Origin origin{filename, lineNo};
                auto start = pos;
                while (pos < line.size() && is_identifier(line[pos])) ++pos;
                std::string text = line.substr(start, pos - start);
                if (text.size() >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
                    char *endPtr = nullptr;
                    int result = strtol(text.substr(2).c_str(), &endPtr, 16);
                    if (*endPtr == '\x0') {
                        tokens.push_back(Token{origin, TokenType::Integer, result});
                    } else {
                        logger_log(filename + ":" + std::to_string(lineNo) + "  Invalid hex literal.");
                        ++errorCount;
                    }
                } else {
                    char *endPtr = nullptr;
                    int result = strtol(text.c_str(), &endPtr, 10);
                    if (*endPtr == '\x0') {
                        tokens.push_back(Token{origin, TokenType::Integer, result});
                    } else {
                        tokens.push_back(Token{origin, TokenType::Identifier, 0, text});
                    }
                }
            } else {
                logger_log(filename + ":" + std::to_string(lineNo) + "  Unexpected character " + line[pos] + ".");
                ++errorCount;
                ++pos;
            }
        }
    }

    PHYSFS_close(inf);
    if (errorCount) return std::vector<Token>{};
    return tokens;
}






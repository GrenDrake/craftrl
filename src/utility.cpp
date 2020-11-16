#include <string>
#include <vector>
#include <physfs.h>

unsigned long long hashString(const std::string &str);
std::vector<std::string> explode(const std::string &text);
bool strToInt(const std::string &str, int &number);
std::string trim(std::string s);
std::string upperFirst(std::string text);

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
unsigned long long hashString(const std::string &str) {
    const unsigned long long FNV_offset_basis = 14695981039346656037ul;
    const unsigned long long FNV_prime = 1099511628211ul;
    unsigned long long hash = FNV_offset_basis;

    for (char c : str) {
        hash ^= static_cast<unsigned long>(c);
        hash *= FNV_prime;
    }
    return hash;
}

std::vector<std::string> explode(const std::string &text) {
    std::vector<std::string> parts;
    auto pos = text.find_first_of(" \t");
    std::string::size_type lastpos = 0;
    while (pos != std::string::npos) {
        std::string part = text.substr(lastpos, pos - lastpos);
        lastpos = pos;
        pos = text.find_first_of(" \t", pos + 1);

        part = trim(part);
        if (!part.empty()) parts.push_back(part);
    }
    std::string part = text.substr(lastpos);
    part = trim(part);
    if (!part.empty()) parts.push_back(part);
    return parts;
}

std::string readFile(const std::string &filename) {
    PHYSFS_File *fp = PHYSFS_openRead(filename.c_str());
    if (!fp) {
#ifdef LOGGER_H
        logger_log("Failed to open file " + filename + ".");
#endif
        return "";
    }
    auto filesize = PHYSFS_fileLength(fp);
    char *buffer = new char[filesize];
    buffer[0] = 0;
    auto bytesRead = PHYSFS_readBytes(fp, buffer, filesize);
    if (bytesRead < filesize) {
#ifdef LOGGER_H
        logger_log("Incomplete read of file " + filename + ".");
#endif
    }
    std::string text = buffer;
    delete[] buffer;
    return text;
}

bool strToInt(const std::string &str, int &number) {
    std::string work = trim(str);
    if (work.empty()) return false;
    char *endPtr;
    int value = strtol(work.c_str(), &endPtr, 10);
    if (*endPtr == 0) {
        number = value;
        return true;
    } else {
        return false;
    }
}

std::string trim(std::string s) {
    auto start = s.find_first_not_of(" \t");
    if (start > 0) s.erase(0, start);
    while (!s.empty() && s.back() == ' ') s.erase(s.size() - 1);
    return s;
}

std::string upperFirst(std::string text) {
    if (!text.empty()) {
        if (text[0] >= 'a' && text[0] <= 'z') {
            text[0] -= 'a' - 'A';
        }
    }
    return text;
}


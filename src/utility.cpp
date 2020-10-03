#include <string>

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

std::string upperFirst(std::string text) {
    if (!text.empty()) {
        if (text[0] >= 'a' && text[0] <= 'z') {
            text[0] -= 'a' - 'A';
        }
    }
    return text;
}
#include "include/dsl/lexer.hpp"
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <sstream>

static const std::unordered_map<std::string, TokenType> action_keywords = {
    {"select", TokenType::Select},
    {"insert", TokenType::Insert},
    {"create", TokenType::Create},
    {"=", TokenType::Equal},
    {">", TokenType::Greater},
    {">=", TokenType::GreaterOrEqual},
    {"<", TokenType::Less},
    {"<=", TokenType::LessOrEqual},
    {"from", TokenType::From},
    {";", TokenType::Eos},
    {"and", TokenType::And},
    {"where", TokenType::Where}
};

std::string to_lower(const std::string &s) {
    std::string copy = s;
    std::transform(s.begin(), s.end(), copy.begin(), [](unsigned char c){ return std::tolower(c); });
    return copy;
}

std::string trim(const std::string& s) {
    auto start = std::find_if_not(s.begin(), s.end(),
        [](unsigned char c){ return std::isspace(c); });
    auto end = std::find_if_not(s.rbegin(), s.rend(),
        [](unsigned char c){ return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : "";
}

std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> result;
    std::string word;
    while (iss >> word) {
        result.push_back(word);
    }
    return result;
}

std::expected<std::vector<Token>, LexingError> lex(const std::string &query) {
    std::vector<Token> parsed_tokens;
    std::string buffer;

    auto flush_buffer = [&]() -> std::expected<void, LexingError> {
        if (buffer.empty()) return {};
        std::string lower = to_lower(buffer);
        if (auto it = action_keywords.find(lower); it != action_keywords.end()) {
            parsed_tokens.push_back(Token{it->second});
        } else {
            parsed_tokens.push_back(Token{TokenType::Identifier, buffer});
        }
        buffer.clear();
    };

    for (const char ch : query) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (std::isspace(c)) {
            flush_buffer();
        } else if (ch == ',') {
            flush_buffer();
            parsed_tokens.push_back(Token{
                .token_type = TokenType::Comma
            });
        } else if (ch == ';') {
            flush_buffer();
            parsed_tokens.push_back(Token{
                .token_type = TokenType::Eos
            });
        } else if (ch == '(') {
            flush_buffer();
            parsed_tokens.push_back(Token{
                .token_type = TokenType::Open_Paren
            });
        } else if (ch == ')') {
            flush_buffer();
            parsed_tokens.push_back(Token{
                .token_type = TokenType::Close_Paren
            });
        } else {
            buffer.push_back(ch);
        }
    }
    flush_buffer();
    return parsed_tokens;
}
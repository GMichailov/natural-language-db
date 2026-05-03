#pragma once

#include <string>
#include <vector>
#include <expected>

enum class TokenType {
    Select,
    Insert,
    Create,
    Equal,
    Greater,
    GreaterOrEqual,
    Less,
    LessOrEqual,
    Identifier,
    Comma,
    From,
    Eos,
    Unidentifiable,
    Open_Paren,
    Close_Paren,
    And
};

struct Token {
    TokenType token_type;
    std::string value;
};

struct LexingError {
    std::string message;
    size_t pos;
};

std::expected<std::vector<Token>, LexingError> lex(std::string &query);
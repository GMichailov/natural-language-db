#include "include/dsl/parser.hpp"

inline bool verify_select_tokens_structure(std::vector<Token> &tokens) {
    // Structure should be: Select, identifier(s) separated by commas, from, identifier. And then optional Where clause verified separately.
    if (tokens[0].token_type != TokenType::Select) return false;
    size_t idx = 1;
    bool star_seen = false;
    while (idx < tokens.size() && tokens[idx].token_type != TokenType::From) {
        if (idx % 2 == 0 && tokens[idx].token_type != TokenType::Comma) return false;
        else if (tokens[idx].token_type != TokenType::Identifier) return false;
        else if (star_seen) return false;
        else {
            if (tokens[idx].value == "*") star_seen = true;
        }
    }
    // Check if ended on a comma or ran out of tokens.
    if (idx == tokens.size() || idx % 2 == 1) return false;
    idx += 1;
    if (tokens[idx].token_type != TokenType::Identifier) return false;
    idx += 1;
    // Next token should immediately be an Eos or a Where, nothing else.
    if (tokens[idx].token_type == TokenType::Eos) return idx == tokens.size()-1;
    else if (tokens[idx].token_type != TokenType::Where) return false;
    idx += 1;
    // For simplicity, avoid parenthesis for now.
    // Accepted where statments are WHERE column op value AND? (More statements); 
    size_t offset = 0;
    size_t and_count = 0;
    while (idx + offset < tokens.size()) {
        // This has a bug right now. Easier to just check in batches of 3.
        if (((offset - and_count) % 3 == 0 || (offset - and_count) % 3 == 2) && tokens[idx + offset].token_type != TokenType::Identifier) return false;
        else if ((offset - and_count) % 3 == 1) {
            
        }
    }
}

inline Statement parse_tokens_into_select_statement(std::vector<Token> &tokens) {

}

inline Statement parse_tokens_into_insert_statement(std::vector<Token> &tokens) {
    
}

inline Statement parse_tokens_into_create_statement(std::vector<Token> &tokens) {
    
}

std::expected<Statement, ParsingError> parse_tokens_into_statement(std::vector<Token> &tokens) {
    switch (tokens[0].token_type) {
        case TokenType::Select:
            return parse_tokens_into_select_statement(tokens);
        case TokenType::Insert:
            return parse_tokens_into_insert_statement(tokens);
        case TokenType::Create:
            return parse_tokens_into_create_statement(tokens);
        default:
            return std::unexpected(ParsingError{
                .message = "Illegal Action Type, must be select, insert, or create"
            });
    }
}
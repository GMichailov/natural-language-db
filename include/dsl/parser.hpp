#include "include/dsl/lexer.hpp"

#include <variant>

using Value = std::variant<int, float, double, std::string>;

struct ComparisonOp {
    TokenType token_type;
};

struct WhereOp {
    std::string column_name;
    ComparisonOp op;
    Value value;
};

struct ParsingError {
    std::string message;
};

struct SelectStatement {
    std::string columns;
};

struct InsertStatement {

};

struct CreateStatement {

};

struct Statement {
    enum class StatementType { SELECT, INSERT, CREATE };
    std::variant<SelectStatement, InsertStatement, CreateStatement> statement;
    StatementType statement_type;
};

std::expected<Statement, ParsingError> parse_tokens_into_statement(std::vector<Token> &tokens);


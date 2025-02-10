
/* tokens.h */
#ifndef TOKENS_H
#define TOKENS_H

/* Token types that need to be recognized by the lexer
 * TODO: Add more token types as per requirements:
 * - Keywords or reserved words (if, repeat, until)
 * - Identifiers
 * - String literals
 * - More operators
 * - Delimiters
 */
typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,           // e.g. 123
    TOKEN_OPERATOR,         // e.g. + - * / % && ||
    TOKEN_ERROR,            // e.g. ERROR_INVALID_CHAR
    TOKEN_KEYWORD,          // e.g. func if until while for
    TOKEN_IDENTIFIER,
    TOKEN_STRING_LITERAL,   // e.g. "SeaPlus+"
    TOKEN_CHAR_LITERAL,     // e.g. 'c'
    TOKEN_DELIMITER,        // e.g. {} [] ()
    TOKEN_SPECIAL_CHARACTER // e.g. _ &
} TokenType;

/* Error types for lexical analysis
 * TODO: Add more error types as needed for your language - as much as you like !!
 */
typedef enum {
    ERROR_NONE,
    ERROR_INVALID_CHAR,
    ERROR_INVALID_NUMBER,
    ERROR_CONSECUTIVE_OPERATORS,
    ERROR_STRING_OVERFLOW,
    ERROR_UNTERMINATED_STRING,
    ERROR_INVALID_ESCAPE_CHARACTER,
    ERROR_UNTERMINATED_CHARACTER,
    ERROR_OPEN_DELIMITER
} ErrorType;

/* Token structure to store token information
 * TODO: Add more fields if needed for your implementation
 * Hint: You might want to consider adding line and column tracking if you want to debug your lexer properly.
 * Don't forget to update the token fields in lexer.c as well
 */
typedef struct {
    TokenType type;
    char lexeme[100];   // Actual text of the token
    int line;           // Line number in source file
    ErrorType error;    // Error type if any
} Token;

#endif /* TOKENS_H */

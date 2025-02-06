
/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../include/tokens.h"
#include "../../include/keywords.h"

// Line tracking
static int current_line = 1;
static char last_token_type = 'x'; // For checking consecutive operators

/* Print error messages for lexical errors */
void print_error(ErrorType error, int line, const char *lexeme) {
    printf("Lexical Error at line %d: ", line);
    switch (error) {
        case ERROR_INVALID_CHAR:
            printf("Invalid character '%s'\n", lexeme);
            break;
        case ERROR_INVALID_NUMBER:
            printf("Invalid number format\n");
            break;
        case ERROR_CONSECUTIVE_OPERATORS:
            printf("Consecutive operators not allowed\n");
            break;
        case ERROR_UNTERMINATED_STRING:
            printf("Unterminated string\n");
            break;
        case ERROR_OPEN_DELIMITER:
            printf("Unclosed brackets\n");
            break;
        default:
            printf("Unknown error\n");
    }
}

/* Print token information */
void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }

    printf("Token: ");
    switch (token.type) {
        case TOKEN_NUMBER:
            printf("NUMBER");
            break;
        case TOKEN_OPERATOR:
            printf("OPERATOR");
            break;
        case TOKEN_EOF:
            printf("EOF");
            break;
        case TOKEN_KEYWORD:
            printf("KEYWORD");
            break;
        case TOKEN_IDENTIFIER:
            printf("IDENTIFIER");
            break;
        case TOKEN_STRING_LITERAL:
            printf("STRING_LITERAL");
            break;
        case TOKEN_DELIMITER:
            printf("DELIMITER");
            break;
        case TOKEN_SPECIAL_CHARACTER:
            printf("SPECIAL_CHARACTER");
            break;
        default:
            printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n", token.lexeme, token.line);
}

/* Get next token from input */
Token get_next_token(const char *input, int *pos) {
    Token token = {TOKEN_ERROR, "", current_line, ERROR_NONE};
    char c;

    // Skip whitespace and track line numbers
    while ((c = input[*pos]) != '\0' && (c == ' ' || c == '\n' || c == '\t')) {
        if (c == '\n') {
            current_line++;
        }
        (*pos)++;
    }

    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    c = input[*pos];

    // TODO: Add comment handling here
    // Comment handler
    // Single line comment
    //can keep skipping until newline character is reached
    if (c == '#') {
        do{
            (*pos)++;
            c = input[*pos];
        } while(c != '\n');
        //skip newline character
        (*pos)++;
        c = input[*pos];

        while(c == ' ' || c == '\t' || c == '\n'  && c != '\0'){
            (*pos)++;
            c = input[*pos];
        }
    }

    // Multi line comment
    // should skip until */ is reached
    if (c == '/' && input[*pos + 1] == '*') {
        (*pos)++;
        c = input[*pos];
       do{
            (*pos)++;
            c = input[*pos];
       }while((c != '*') && (input[*pos + 1] != '/'));
       while(c == '*' || c == '/'){
        //skip comment closing
            (*pos)++;
            c = input[*pos];
       }
       //skip to start of next token
        while(c == ' ' || c == '\t' || c == '\n'  && c != '\0'){
            (*pos)++;
            c = input[*pos];
        }
    }

    // Number handler
    if (isdigit(c)) {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while (isdigit(c) && i < sizeof(token.lexeme) - 1);

        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        last_token_type = 'n';
        return token;
    }

    // Keyword and Identifier handler
    if(isalpha(c) || c == '_'){
        int i = 0;

        do{
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while((isalnum(c) || c == '_') && i < sizeof(token.lexeme) - 1); // numbers and _ are valid in identifiers
        // Terminate string
        token.lexeme[i] = '\0';

        if(iskeyword(token.lexeme)){
            token.type = TOKEN_KEYWORD;
            last_token_type = 'k';
        }
        else{
            token.type = TOKEN_IDENTIFIER;
            last_token_type = 'i';
        }
        return token;
    }

    // TODO: Add string literal handling here (escape characters too)
    // String literal handler
    if(c == '"'){
        int i = 0;
        int untermed = 0;
        do{
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            if(c = '\0'){
                //string has reached EOF
                untermed = 1;
                break;
            }
        } while(c != '"' && i < sizeof(token.lexeme) - 1);
        //terminate string
        //need to include space for the last closing quote
        if(untermed == 1){
            token.error = ERROR_UNTERMINATED_STRING;
        }
        else{
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            token.lexeme[i] = '\0';
        }
        token.type = TOKEN_STRING_LITERAL;
        last_token_type = 's';
        return token;
    }
    // TODO: Add all remaining operators and test them
    // Operator handler
    if (c == '+' || c == '-') {
        if (last_token_type == 'o') {
            // Check for consecutive operators
            token.error = ERROR_CONSECUTIVE_OPERATORS;
            token.lexeme[0] = c;
            token.lexeme[1] = '\0';
            (*pos)++;
            return token;
        }
        token.type = TOKEN_OPERATOR;
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        last_token_type = 'o';
        (*pos)++;
        return token;
    }

    // TODO: TEST THIS DELIMITER CODE
    // Delimiter handler
    // Bracket based Delimiters (must be closed)
    if (c == '(' || c == '{' || c == '[' ||
        c == ')' || c == '}' || c == ']') {
        // should maybe write code to check for closure, but not yet
        token.type = TOKEN_DELIMITER;
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        last_token_type = 'b'; //brackets (any type)
        // note: could have last token type of r (regular), c {curvy}, s [square]
        (*pos)++;
        return token;
    }

    // Generic Delimiters (don't need closure)
    if (c == ';' || c == ',') {
        token.type = TOKEN_DELIMITER;
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        last_token_type = 'd'; //delimiter
        (*pos)++;
        return token;
    }

    // Handle invalid characters
    token.error = ERROR_INVALID_CHAR;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    (*pos)++;
    return token;
}

// This is a basic lexer that handles numbers (e.g., "123", "456"), basic operators (+ and -), consecutive operator errors, whitespace and newlines, with simple line tracking for error reporting.

int main() {
    const char *input = "123 + 456 - 789\n1 ++ 2 \nint print /* this is a multi line \n comment */ myVar my_Var \n #one line comment\n \"String Literal\""; // Test with multi-line input
    int position = 0;
    Token token;

    printf("Analyzing input:\n%s\n\n", input);

    do {
        token = get_next_token(input, &position);
        print_token(token);
    } while (token.type != TOKEN_EOF);

    return 0;
}

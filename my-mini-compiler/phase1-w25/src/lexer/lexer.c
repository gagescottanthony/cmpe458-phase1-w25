
/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../include/tokens.h"
#include "../../include/keywords.h"

// Line tracking
static int current_line = 1;
static char last_token_type = 'y'; // For checking consecutive operators

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
        case ERROR_INVALID_ESCAPE_CHARACTER:
            printf("Unrecognized/invalid escape character\n");
            break;
        case ERROR_UNTERMINATED_CHARACTER:
            printf("Unterminated character\n");
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
        case TOKEN_CHAR_LITERAL:
            printf("CHAR_LITERAL");
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

    // Check for end of file
    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    // get current character
    c = input[*pos];

    // Comment handler
    // Single line comment
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
        (*pos)++; // skip /
        // c = input[*pos];
        do{
            (*pos)++; //move ahead (will also skip asterisk in /*)
            c = input[*pos];
            if (c == '\0') {
                printf("[WARN]: Unclosed comment\n");
                break;
            }
        }while((c != '*') && (input[*pos + 1] != '/'));
        (*pos)+=2; // move ahead of */
        c = input[*pos];
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
        last_token_type = 'n'; //number
        return token;
    }

    // Keyword and Identifier handler
    if(isalpha(c) || (c == '_' && isalnum(input[*pos + 1]))){
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
            last_token_type = 'k'; //keyword
        }
        else{
            token.type = TOKEN_IDENTIFIER;
            last_token_type = 'i'; //identifier
        }
        return token;
    }

    // Special character handler
    if((c == '&' && input[*pos + 1] != '&') || c == '_') {
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        token.type = TOKEN_SPECIAL_CHARACTER;
        (*pos)++;
        last_token_type = 'z'; //special character
        return token;
    }

    // String literal handler
    if(c == '"'){
        int i = 0;
        int unterminated = 0;
        do{
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            if(c == '\0'){ //string has reached EOF
                unterminated = 1;
                break;
            }
        } while(c != '"' && i < sizeof(token.lexeme) - 1);
        //terminate string
        //need to include space for the last closing quote
        if(unterminated == 1){
            token.error = ERROR_UNTERMINATED_STRING;
        }
        else{
            token.lexeme[i++] = c;
            (*pos)++;
            token.lexeme[i] = '\0';
        }
        token.type = TOKEN_STRING_LITERAL;
        last_token_type = 's'; //string
        return token;
    }

    // char literal handler
    if(c == '\''){
        // following character should be an escape character
        char c_char = input[*pos+1];
        if(c_char == '\\') {
            // check it gets closed, if not skip 4 characters and continue
            if (input[*pos+3] != '\'') {
                token.error = ERROR_UNTERMINATED_CHARACTER;
                token.lexeme[0] = c_char;
                token.lexeme[1] = '\0';
                last_token_type = 'e'; //error
                (*pos) += 4;
                return token;
            }

            // case block for all escape characters supported by the system
            char c_escape = input[*pos+2];
            switch (c_escape) {
                case '\\':
                case '\'':
                case '\"':
                case '\?':
                case 'n':
                case 'r':
                case 't':
                    // all valid escape chars \\ \' \" \? \r \r \t
                    token.lexeme[0] = c_char;
                    token.lexeme[1] = c_escape;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_CHAR_LITERAL;
                    last_token_type = 'x'; // escape char
                    (*pos) += 4;
                    return token;
                default:
                    // unrecognized escape character
                    token.error = ERROR_INVALID_ESCAPE_CHARACTER;
                    token.lexeme[0] = c_char;
                    token.lexeme[1] = '\0';
                    last_token_type = 'e'; // error
                    (*pos) += 4;
                    return token;
            }
        }
        // using an invalid character like ? (don't need to check \ because it would have been caught above)
        if (c_char == '\?' ) {
            token.error = ERROR_INVALID_CHAR;
            token.lexeme[0] = c_char;
            token.lexeme[1] = '\0';
            last_token_type = 'e'; // error
            (*pos) += 3;
        }
        else if (input[*pos+2] != '\'') { // unterminated character
            token.error = ERROR_UNTERMINATED_CHARACTER;
            last_token_type = 'e'; // error
            (*pos) += 3;
        }
        else {  // any valid character
            token.lexeme[0] = c_char;
            token.lexeme[1] = '\0';
            token.type = TOKEN_CHAR_LITERAL;
            *pos += 3;
            last_token_type = 'c'; // char
        }
        // the char literal handler can finally return
        return token;
    }

    // Operator handler
    /* List of Operators (Grouped by first character and behaviour):
    //RULE: Standalone
    $:  $ (factorial)

    //RULE: Can be trailed by one repetition or an equals sign
    +:  + (add), ++ (increment), += (add-assign)
    -:  - (sub), -- (decrement), -= (sub-assign)

    //RULE: Can be trailed only by an equals sign
    *:  * (multiply), *= (multiply-assign)      NOTE: ** not an op
    /:  / (divide), /= (divide-assign)        NOTE: // is meaningless
    %:  % (modulo), %= (mod-assign)
    =:  = (assignment), == (logic eq)
    !:  ! (bitwise not), != (logic not)

    //RULE: Can be trailed only by itself
    |:  | (bitwise or), || (logical or)
    ^:  ^ (bitwise xor), ^^ (power)

    //RULE: Can be trailed by itself or question mark and CANT standalone
    &:  && (logical and), &? (bitwise and)   NOTE: & is a special char
    
    //RULE: Can repeat 3 times or be trailed by an equals sign
    <:  < (less), <= (less or equal), << (shift left), <<< (rotate left)
    >:  > (greater), >= (greater or equal), >> (shift right), >>> (rotate right)
    */

    if (c == '$' || c == '+' || c == '-' || c == '*' || c == '/'
        || c == '%' || c == '=' || c == '!'  || c == '|'
        || c == '^' || c == '&' || c == '<' || c== '>') {
        // Check for consecutive operators
        if (last_token_type == 'o' && c != '!' && c != '$') {
            token.error = ERROR_CONSECUTIVE_OPERATORS;
            token.lexeme[0] = c;
            token.lexeme[1] = '\0';
            (*pos)++;
            return token;
        }

        //Determine first-char logic
        char c_next = input[*pos + 1]; // c_next should always be in array bound, if passed in string was completed with a null terminal.
        switch (c) {
            //Can be trailed by one repetition or an equals sign
            case '+':
            case '-':
                if (c_next == '=') {
                    // += and -= cases
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'q'; // equals
                } else if(c_next == c) {
                    // ++ and -- cases
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'o'; // operator
                } else {
                    // +, - case
                    token.lexeme[0] = c;
                    token.lexeme[1] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 1;
                    last_token_type = 'o'; // operator
                }
                break;

            //Can be trailed only by an equals sign
            case '*':
            case '/':
            case '%':
            case '=':
                if (c_next == '=') {
                    // *=, /=, %=, ==
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'q'; // equals
                } else {
                    // *, /, %, =
                    token.lexeme[0] = c;
                    token.lexeme[1] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 1;
                    last_token_type = 'o'; // operator
                }
                break;

            //Can be chained together as many times as you want !!!!true
            case '!':
                if (c_next == '=') {
                    //!= case
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'q'; // equals
                } else {
                    token.lexeme[0] = c;
                    token.lexeme[1] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 1;
                    last_token_type = 'u'; // repeatable operator (unary)
                }
                break;



            //Can be trailed only by itself
            case '|':
            case '^':
                if (c_next == c) {
                    // ||, ^^
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'o'; // operator
                } else {
                    // |, ^
                    token.lexeme[0] = c;
                    token.lexeme[1] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 1;
                    last_token_type = 'o'; // operator
                }
                break;

            //Can be trailed by itself or question mark and CANT standalone
            case '&':
                if (c_next == c || c_next == '?') {
                    // &&, &?
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'o'; // operator
                }
                break;

            //RULE: Can repeat 3 times or be trailed by an equals sign
            case '<':
            case '>':
                if (c_next == c) {
                    // <<, <<<, >>, >>>
                    //input of *pos+2 should be in bound because c_next was a regular character.
                    if (input[*pos + 2] == c) {
                        // <<<, >>>
                        token.lexeme[0] = c;
                        token.lexeme[1] = c;
                        token.lexeme[2] = c;
                        token.lexeme[3] = '\0';
                        token.type = TOKEN_OPERATOR;
                        *pos += 3;
                        last_token_type = 'o'; // operator
                    } else {
                        // <<, >>
                        token.lexeme[0] = c;
                        token.lexeme[1] = c;
                        token.lexeme[2] = '\0';
                        token.type = TOKEN_OPERATOR;
                        *pos += 2;
                        last_token_type = 'o'; // operator
                    }
                    break;
                }
                //must be separate to prevent <=< from being valid, for example.
                if (c_next == '=') {
                    // <=, >=
                    token.lexeme[0] = c;
                    token.lexeme[1] = c_next;
                    token.lexeme[2] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 2;
                    last_token_type = 'o'; // operator
                } else {
                    // <, >
                    token.lexeme[0] = c;
                    token.lexeme[1] = '\0';
                    token.type = TOKEN_OPERATOR;
                    *pos += 1;
                    last_token_type = 'o'; // operator
                }
                break;

            case '$':
                // $ is factorial
                token.lexeme[0] = c;
                token.lexeme[1] = '\0';
                token.type = TOKEN_OPERATOR;
                *pos += 1;
                last_token_type = 'u'; //technically infinitely repeatable $$5 so unary

            // If it somehow caught the operator but couldn't identify it, this catches it
            default:
                printf("[WARN]: Character %c was accepted by if statement but not assigned a case. Assuming standalone operator.\n", c);
        }
        // "finally the token can return to the main function. May he finally rest..."
        return token;
    }

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
    last_token_type = 'e'; //error
    (*pos)++;
    return token;
}

int main() {
    // get file
    FILE *file = fopen("../phase1-w25/test/input_valid.txt", "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // get buffer size based on file size for chars
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    // fill buffer with full file of chars in order
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    size_t b = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] != '\r') {
            buffer[b++] = buffer[i];
        }
    }
    buffer[b] = '\0';

    // start at beginning of buffer
    int position = 0;
    Token token;

    // perform tokenization
    printf("Analyzing Correct Input:\n%s\n\n", buffer);
    do {
        token = get_next_token(buffer, &position);
        print_token(token);
    } while (token.type != TOKEN_EOF);

    // free memory and close file
    free(buffer);
    fclose(file);

    // Repeat for Incorrect file
    current_line = 1;

    // get file
    file = fopen("../phase1-w25/test/input_invalid.txt", "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // get buffer size based on file size for chars
    buffer = malloc(file_size + 1);
    if (!buffer) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    // fill buffer with full file of chars in order
    bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    b = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] != '\r') {
            buffer[b++] = buffer[i];
        }
    }
    buffer[b] = '\0';

    // start at beginning of buffer
    position = 0;

    // perform tokenization
    printf("Analyzing Incorrect Input:\n%s\n\n", buffer);
    do {
        token = get_next_token(buffer, &position);
        print_token(token);
    } while (token.type != TOKEN_EOF);

    // free memory "he ain't deserve to be locked up"
    free(buffer);
    fclose(file);
    return 0;
}

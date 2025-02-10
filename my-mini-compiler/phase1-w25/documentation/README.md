# SeaPlus+ README

## Keywords
|Category|Keywords|
|--------|--------|
|**Logic/Conditionals:**|if, else, switch, case, default|
|**Logic:**|do, while, for, until, break|
|**IO:**|print, read|
|**Data Types:**|int, float, double, char, bool, string, void|
|**Functions:**|func|
|**Misc:**|null, true, false|

## Operators
||||||||
|---|---|---|---|---|---|---|
|+ (add)|- (sub)|* (mult)|/ (div)|% (mod)|\|\| (log or)|&& (log and)
|+= (add-assign)|-= (sub-assign)|*= (mult-assign)|/= (div-assign)|%= (mod-assign)|++(incre)|-- (decre)
|$ (factorial)|<= (less-eq)|>= (grt-eq)|= (assign)|== (log eq)|!=(log not)|! (not)
|^^ (power)|<< (shift-l)|>> (shift-r)|< (less)|> (grt)|\| (b.w. or)|&? (b.w. and)
|^ (b.w. xor)|<<< (rot-l)|>>> (rot-r)||||
## Delimiters and Punctuation
|||||||||
|---|---|---|---|---|---|---|---|
|,|;|{|}|(|)|[|]|

## String Literals
Strings can be up to 100 characters in length (max 99 other characters followed by \0).
Strings exceeding this length will return an ERROR_STRING_OVERFLOW type token.
Acceptable characters include all Alphanumeric symbols, punctation, and whitespace (though some of these require escape characters to parse correctly).

### Escape Characters

|||||||
|---|---|---|---|---|---|
|\\\\ |\\'|\\"|\\n|\\r|\\t|

Unrecognized/Invalid escape characters (not listed above) will return an ERROR_INVALID_ESCAPE_CHARACTER.
```
"Strings look like this"
```
Unclosed double-quotes will return an ERROR_UNTERMINATED_STRING.

## Char Literals
Char literals are only 1 character in length, and support the same character set as String Literals.
Tokens exceeding this length or missing with unclosed single-quotes will return an ERROR_UNTERMINATED_CHARACTER.
```
//Example Characters
'a'
'\t'
'2'
```

## Special Characters
||||
|---|---|---|
|\_|&|
## Comments
Single line:
```
#Single line comments look like this
```
Multi line:
```
/*Multi line comments look  
like this*/
```
## Parser Error Generation
|Error Type|
|---|
|ERROR_NONE|
|ERROR_INVALID_CHAR|
|ERROR_INVALID_NUMBER|
|ERROR_CONSECUTIVE_OPERATORS|
|ERROR_STRING_OVERFLOW|
|ERROR_UNTERMINATED_STRING|
|ERROR_INVALID_ESCAPE_CHARACTER|
|ERROR_UNTERMINATED_CHARACTER|
|ERROR_OPEN_DELIMITER|
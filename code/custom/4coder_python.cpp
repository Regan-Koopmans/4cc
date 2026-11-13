/*
4coder_python.cpp - Python lexer and per-buffer language selection.
*/

// TOP

#if !defined(FCODER_PYTHON_CPP)
#define FCODER_PYTHON_CPP

// NOTE: Python tokens reuse the C++ sub kinds so existing coloring,
// code index, and token helpers work without change.

struct Python_Keyword{
    String_Const_u8 text;
    Token_Base_Kind base_kind;
    i16 sub_kind;
};

global Python_Keyword python_keywords[] = {
    {string_u8_litexpr("False"),    TokenBaseKind_LiteralInteger, TokenCppKind_LiteralFalse},
    {string_u8_litexpr("True"),     TokenBaseKind_LiteralInteger, TokenCppKind_LiteralTrue},
    {string_u8_litexpr("None"),     TokenBaseKind_Keyword, TokenCppKind_NullPtr},
    {string_u8_litexpr("and"),      TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("as"),       TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("assert"),   TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("async"),    TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("await"),    TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("break"),    TokenBaseKind_Keyword, TokenCppKind_Break},
    {string_u8_litexpr("class"),    TokenBaseKind_Keyword, TokenCppKind_Class},
    {string_u8_litexpr("continue"), TokenBaseKind_Keyword, TokenCppKind_Continue},
    {string_u8_litexpr("def"),      TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("del"),      TokenBaseKind_Keyword, TokenCppKind_Delete},
    {string_u8_litexpr("elif"),     TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("else"),     TokenBaseKind_Keyword, TokenCppKind_Else},
    {string_u8_litexpr("except"),   TokenBaseKind_Keyword, TokenCppKind_Catch},
    {string_u8_litexpr("finally"),  TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("for"),      TokenBaseKind_Keyword, TokenCppKind_For},
    {string_u8_litexpr("from"),     TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("global"),   TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("if"),       TokenBaseKind_Keyword, TokenCppKind_If},
    {string_u8_litexpr("import"),   TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("in"),       TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("is"),       TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("lambda"),   TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("nonlocal"), TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("not"),      TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("or"),       TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("pass"),     TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("raise"),    TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("return"),   TokenBaseKind_Keyword, TokenCppKind_Return},
    {string_u8_litexpr("try"),      TokenBaseKind_Keyword, TokenCppKind_Try},
    {string_u8_litexpr("while"),    TokenBaseKind_Keyword, TokenCppKind_While},
    {string_u8_litexpr("with"),     TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("yield"),    TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
};

struct Python_Operator{
    char *text;
    Token_Base_Kind base_kind;
    i16 sub_kind;
};

// NOTE: Longest operators first so the first match is the longest.
global Python_Operator python_operators[] = {
    {"**=", TokenBaseKind_Operator, TokenCppKind_StarEq},
    {"//=", TokenBaseKind_Operator, TokenCppKind_DivEq},
    {">>=", TokenBaseKind_Operator, TokenCppKind_RightRightEq},
    {"<<=", TokenBaseKind_Operator, TokenCppKind_LeftLeftEq},
    {"...", TokenBaseKind_Operator, TokenCppKind_DotDotDot},
    {"->",  TokenBaseKind_Operator, TokenCppKind_Arrow},
    {":=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"**",  TokenBaseKind_Operator, TokenCppKind_Star},
    {"//",  TokenBaseKind_Operator, TokenCppKind_Div},
    {"<<",  TokenBaseKind_Operator, TokenCppKind_LeftLeft},
    {">>",  TokenBaseKind_Operator, TokenCppKind_RightRight},
    {"<=",  TokenBaseKind_Operator, TokenCppKind_LessEq},
    {">=",  TokenBaseKind_Operator, TokenCppKind_GrtrEq},
    {"==",  TokenBaseKind_Operator, TokenCppKind_EqEq},
    {"!=",  TokenBaseKind_Operator, TokenCppKind_NotEq},
    {"+=",  TokenBaseKind_Operator, TokenCppKind_PlusEq},
    {"-=",  TokenBaseKind_Operator, TokenCppKind_MinusEq},
    {"*=",  TokenBaseKind_Operator, TokenCppKind_StarEq},
    {"/=",  TokenBaseKind_Operator, TokenCppKind_DivEq},
    {"%=",  TokenBaseKind_Operator, TokenCppKind_ModEq},
    {"&=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"|=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"^=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"@=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"(",   TokenBaseKind_ParentheticalOpen,  TokenCppKind_ParenOp},
    {")",   TokenBaseKind_ParentheticalClose, TokenCppKind_ParenCl},
    {"[",   TokenBaseKind_ParentheticalOpen,  TokenCppKind_BrackOp},
    {"]",   TokenBaseKind_ParentheticalClose, TokenCppKind_BrackCl},
    {"{",   TokenBaseKind_ScopeOpen,  TokenCppKind_BraceOp},
    {"}",   TokenBaseKind_ScopeClose, TokenCppKind_BraceCl},
    {";",   TokenBaseKind_StatementClose, TokenCppKind_Semicolon},
    {",",   TokenBaseKind_StatementClose, TokenCppKind_Comma},
    {":",   TokenBaseKind_Operator, TokenCppKind_Colon},
    {".",   TokenBaseKind_Operator, TokenCppKind_Dot},
    {"+",   TokenBaseKind_Operator, TokenCppKind_Plus},
    {"-",   TokenBaseKind_Operator, TokenCppKind_Minus},
    {"*",   TokenBaseKind_Operator, TokenCppKind_Star},
    {"/",   TokenBaseKind_Operator, TokenCppKind_Div},
    {"%",   TokenBaseKind_Operator, TokenCppKind_Mod},
    {"&",   TokenBaseKind_Operator, TokenCppKind_And},
    {"|",   TokenBaseKind_Operator, TokenCppKind_Or},
    {"^",   TokenBaseKind_Operator, TokenCppKind_Xor},
    {"~",   TokenBaseKind_Operator, TokenCppKind_Tilde},
    {"<",   TokenBaseKind_Operator, TokenCppKind_Less},
    {">",   TokenBaseKind_Operator, TokenCppKind_Grtr},
    {"=",   TokenBaseKind_Operator, TokenCppKind_Eq},
    {"@",   TokenBaseKind_Operator, TokenCppKind_Star},
};

function b32
python_char_is_ident_start(u8 c){
    return(character_is_alpha(c) || c >= 0x80);
}

function b32
python_char_is_ident(u8 c){
    return(character_is_alpha_numeric(c) || c >= 0x80);
}

function b32
python_char_is_string_prefix(u8 c){
    switch (c){
        case 'r': case 'R':
        case 'b': case 'B':
        case 'u': case 'U':
        case 'f': case 'F':
        {
            return(true);
        }
    }
    return(false);
}

// NOTE: Returns the length of a string prefix (0, 1, or 2) if the bytes at i
// begin a string literal, otherwise -1.
function i64
python_string_prefix_length(u8 *str, i64 i, i64 size){
    i64 result = -1;
    if (i < size && (str[i] == '\'' || str[i] == '"')){
        result = 0;
    }
    else if (i + 1 < size && python_char_is_string_prefix(str[i]) &&
             (str[i + 1] == '\'' || str[i + 1] == '"')){
        result = 1;
    }
    else if (i + 2 < size && python_char_is_string_prefix(str[i]) &&
             python_char_is_string_prefix(str[i + 1]) &&
             (str[i + 2] == '\'' || str[i + 2] == '"')){
        result = 2;
    }
    return(result);
}

function i64
python_lex_string(u8 *str, i64 i, i64 size, b32 *is_triple_out){
    u8 quote = str[i];
    b32 triple = (i + 2 < size && str[i + 1] == quote && str[i + 2] == quote);
    *is_triple_out = triple;
    if (triple){
        i += 3;
        for (;i < size;){
            if (str[i] == '\\'){
                i += 2;
            }
            else if (str[i] == quote && i + 2 < size &&
                     str[i + 1] == quote && str[i + 2] == quote){
                i += 3;
                return(i);
            }
            else{
                i += 1;
            }
        }
    }
    else{
        i += 1;
        for (;i < size;){
            if (str[i] == '\\'){
                if (i + 1 < size && str[i + 1] == '\r' && i + 2 < size && str[i + 2] == '\n'){
                    i += 3;
                }
                else{
                    i += 2;
                }
            }
            else if (str[i] == quote){
                i += 1;
                return(i);
            }
            else if (str[i] == '\n'){
                // NOTE: Unterminated string ends at end of line.
                return(i);
            }
            else{
                i += 1;
            }
        }
    }
    return(clamp_top(i, size));
}

function i64
python_lex_number(u8 *str, i64 i, i64 size, b32 *is_float_out){
    b32 is_float = false;
    if (str[i] == '0' && i + 1 < size &&
        (str[i + 1] == 'x' || str[i + 1] == 'X' ||
         str[i + 1] == 'o' || str[i + 1] == 'O' ||
         str[i + 1] == 'b' || str[i + 1] == 'B')){
        i += 2;
        for (;i < size && (character_is_alpha_numeric(str[i]));){
            i += 1;
        }
    }
    else{
        for (;i < size && (character_is_base10(str[i]) || str[i] == '_');){
            i += 1;
        }
        if (i < size && str[i] == '.'){
            is_float = true;
            i += 1;
            for (;i < size && (character_is_base10(str[i]) || str[i] == '_');){
                i += 1;
            }
        }
        if (i < size && (str[i] == 'e' || str[i] == 'E')){
            i64 j = i + 1;
            if (j < size && (str[j] == '+' || str[j] == '-')){
                j += 1;
            }
            if (j < size && character_is_base10(str[j])){
                is_float = true;
                i = j;
                for (;i < size && (character_is_base10(str[i]) || str[i] == '_');){
                    i += 1;
                }
            }
        }
        if (i < size && (str[i] == 'j' || str[i] == 'J')){
            is_float = true;
            i += 1;
        }
    }
    *is_float_out = is_float;
    return(i);
}

function Token_List
lex_full_input_python(Arena *arena, String_Const_u8 input){
    Token_List list = {};
    u8 *str = input.str;
    i64 size = (i64)input.size;
    i64 i = 0;
    for (;i < size;){
        Token token = {};
        token.pos = i;
        u8 c = str[i];

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'){
            for (;i < size;){
                u8 w = str[i];
                if (!(w == ' ' || w == '\t' || w == '\n' || w == '\r' || w == '\f' || w == '\v')){
                    break;
                }
                i += 1;
            }
            token.kind = TokenBaseKind_Whitespace;
            token.sub_kind = TokenCppKind_Whitespace;
        }
        else if (c == '#'){
            for (;i < size && str[i] != '\n' && str[i] != '\r';){
                i += 1;
            }
            token.kind = TokenBaseKind_Comment;
            token.sub_kind = TokenCppKind_LineComment;
        }
        else if (c == '\\'){
            i += 1;
            token.kind = TokenBaseKind_Whitespace;
            token.sub_kind = TokenCppKind_Backslash;
        }
        else if (python_string_prefix_length(str, i, size) >= 0){
            i64 prefix = python_string_prefix_length(str, i, size);
            b32 triple = false;
            i = python_lex_string(str, i + prefix, size, &triple);
            token.kind = TokenBaseKind_LiteralString;
            token.sub_kind = triple?TokenCppKind_LiteralStringRaw:TokenCppKind_LiteralString;
        }
        else if (python_char_is_ident_start(c)){
            for (;i < size && python_char_is_ident(str[i]);){
                i += 1;
            }
            String_Const_u8 lexeme = SCu8(str + token.pos, i - token.pos);
            token.kind = TokenBaseKind_Identifier;
            token.sub_kind = TokenCppKind_Identifier;
            for (i32 k = 0; k < ArrayCount(python_keywords); k += 1){
                if (string_match(lexeme, python_keywords[k].text)){
                    token.kind = python_keywords[k].base_kind;
                    token.sub_kind = python_keywords[k].sub_kind;
                    break;
                }
            }
        }
        else if (character_is_base10(c) ||
                 (c == '.' && i + 1 < size && character_is_base10(str[i + 1]))){
            b32 is_float = false;
            if (c == '.'){
                // NOTE: ".5" style float.
                i += 1;
                for (;i < size && (character_is_base10(str[i]) || str[i] == '_');){
                    i += 1;
                }
                is_float = true;
                if (i < size && (str[i] == 'e' || str[i] == 'E' || str[i] == 'j' || str[i] == 'J')){
                    b32 ignore = false;
                    i = python_lex_number(str, i - 1, size, &ignore);
                }
            }
            else{
                i = python_lex_number(str, i, size, &is_float);
            }
            if (is_float){
                token.kind = TokenBaseKind_LiteralFloat;
                token.sub_kind = TokenCppKind_LiteralFloat64;
            }
            else{
                token.kind = TokenBaseKind_LiteralInteger;
                token.sub_kind = TokenCppKind_LiteralInteger;
            }
        }
        else if (c == '@' && i + 1 < size && python_char_is_ident_start(str[i + 1])){
            // NOTE: Decorator. Colored as preprocessor.
            i += 1;
            for (;i < size && (python_char_is_ident(str[i]) || str[i] == '.');){
                i += 1;
            }
            token.kind = TokenBaseKind_Preprocessor;
            token.sub_kind = TokenCppKind_PPUnknown;
        }
        else{
            b32 matched = false;
            for (i32 k = 0; k < ArrayCount(python_operators); k += 1){
                char *op = python_operators[k].text;
                i64 len = (i64)cstring_length(op);
                if (i + len <= size && block_match(str + i, op, len)){
                    i += len;
                    token.kind = python_operators[k].base_kind;
                    token.sub_kind = python_operators[k].sub_kind;
                    matched = true;
                    break;
                }
            }
            if (!matched){
                i += 1;
                token.kind = TokenBaseKind_LexError;
                token.sub_kind = TokenCppKind_LexError;
            }
        }

        token.size = i - token.pos;
        token_list_push(arena, &list, &token);
    }

    Token eof = {};
    eof.pos = size;
    eof.size = 0;
    eof.kind = TokenBaseKind_EOF;
    eof.sub_kind = TokenCppKind_EOF;
    token_list_push(arena, &list, &eof);

    return(list);
}

#endif

// BOTTOM

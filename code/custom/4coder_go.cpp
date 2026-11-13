/*
4coder_go.cpp - Go lexer.
*/

// TOP

#if !defined(FCODER_GO_CPP)
#define FCODER_GO_CPP

// NOTE: Go tokens reuse the C++ sub kinds so existing coloring, code index,
// and token helpers work without change. Go has no explicit statement
// terminators; the lexer applies Go's semicolon insertion rule and flags the
// last token of each statement with TokenBaseFlag_StatementEnd so the brace
// indenter knows a statement is complete.

struct Go_Keyword{
    String_Const_u8 text;
    Token_Base_Kind base_kind;
    i16 sub_kind;
};

global Go_Keyword go_keywords[] = {
    {string_u8_litexpr("break"),       TokenBaseKind_Keyword, TokenCppKind_Break},
    {string_u8_litexpr("case"),        TokenBaseKind_Keyword, TokenCppKind_Case},
    {string_u8_litexpr("chan"),        TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("const"),       TokenBaseKind_Keyword, TokenCppKind_Const},
    {string_u8_litexpr("continue"),    TokenBaseKind_Keyword, TokenCppKind_Continue},
    {string_u8_litexpr("default"),     TokenBaseKind_Keyword, TokenCppKind_Default},
    {string_u8_litexpr("defer"),       TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("else"),        TokenBaseKind_Keyword, TokenCppKind_Else},
    {string_u8_litexpr("fallthrough"), TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("for"),         TokenBaseKind_Keyword, TokenCppKind_For},
    {string_u8_litexpr("func"),        TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("go"),          TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("goto"),        TokenBaseKind_Keyword, TokenCppKind_Goto},
    {string_u8_litexpr("if"),          TokenBaseKind_Keyword, TokenCppKind_If},
    {string_u8_litexpr("import"),      TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("interface"),   TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("map"),         TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("package"),     TokenBaseKind_Keyword, TokenCppKind_Namespace},
    {string_u8_litexpr("range"),       TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("return"),      TokenBaseKind_Keyword, TokenCppKind_Return},
    {string_u8_litexpr("select"),      TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("struct"),      TokenBaseKind_Keyword, TokenCppKind_Struct},
    {string_u8_litexpr("switch"),      TokenBaseKind_Keyword, TokenCppKind_Switch},
    {string_u8_litexpr("type"),        TokenBaseKind_Keyword, TokenCppKind_Typedef},
    {string_u8_litexpr("var"),         TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
    {string_u8_litexpr("true"),        TokenBaseKind_LiteralInteger, TokenCppKind_LiteralTrue},
    {string_u8_litexpr("false"),       TokenBaseKind_LiteralInteger, TokenCppKind_LiteralFalse},
    {string_u8_litexpr("nil"),         TokenBaseKind_Keyword, TokenCppKind_NullPtr},
    {string_u8_litexpr("iota"),        TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric},
};

struct Go_Operator{
    char *text;
    Token_Base_Kind base_kind;
    i16 sub_kind;
};

// NOTE: Longest operators first so the first match is the longest.
global Go_Operator go_operators[] = {
    {"<<=", TokenBaseKind_Operator, TokenCppKind_LeftLeftEq},
    {">>=", TokenBaseKind_Operator, TokenCppKind_RightRightEq},
    {"&^=", TokenBaseKind_Operator, TokenCppKind_Eq},
    {"...", TokenBaseKind_Operator, TokenCppKind_DotDotDot},
    {"&&",  TokenBaseKind_Operator, TokenCppKind_AndAnd},
    {"||",  TokenBaseKind_Operator, TokenCppKind_OrOr},
    {"<-",  TokenBaseKind_Operator, TokenCppKind_Arrow},
    {"++",  TokenBaseKind_Operator, TokenCppKind_PlusPlus},
    {"--",  TokenBaseKind_Operator, TokenCppKind_MinusMinus},
    {"==",  TokenBaseKind_Operator, TokenCppKind_EqEq},
    {"!=",  TokenBaseKind_Operator, TokenCppKind_NotEq},
    {"<=",  TokenBaseKind_Operator, TokenCppKind_LessEq},
    {">=",  TokenBaseKind_Operator, TokenCppKind_GrtrEq},
    {":=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"<<",  TokenBaseKind_Operator, TokenCppKind_LeftLeft},
    {">>",  TokenBaseKind_Operator, TokenCppKind_RightRight},
    {"&^",  TokenBaseKind_Operator, TokenCppKind_And},
    {"+=",  TokenBaseKind_Operator, TokenCppKind_PlusEq},
    {"-=",  TokenBaseKind_Operator, TokenCppKind_MinusEq},
    {"*=",  TokenBaseKind_Operator, TokenCppKind_StarEq},
    {"/=",  TokenBaseKind_Operator, TokenCppKind_DivEq},
    {"%=",  TokenBaseKind_Operator, TokenCppKind_ModEq},
    {"&=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"|=",  TokenBaseKind_Operator, TokenCppKind_Eq},
    {"^=",  TokenBaseKind_Operator, TokenCppKind_Eq},
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
    {"!",   TokenBaseKind_Operator, TokenCppKind_Not},
    {"<",   TokenBaseKind_Operator, TokenCppKind_Less},
    {">",   TokenBaseKind_Operator, TokenCppKind_Grtr},
    {"=",   TokenBaseKind_Operator, TokenCppKind_Eq},
    {"~",   TokenBaseKind_Operator, TokenCppKind_Tilde},
};

function b32
go_char_is_ident_start(u8 c){
    return(character_is_alpha(c) || c >= 0x80);
}

function b32
go_char_is_ident(u8 c){
    return(character_is_alpha_numeric(c) || c >= 0x80);
}

// NOTE: Go spec: a semicolon is inserted after a line's final token if that
// token is an identifier, a literal, one of the keywords break, continue,
// fallthrough, return, or one of ++ -- ) ] }.
function b32
go_token_ends_statement(Token *token, u8 *str){
    b32 result = false;
    switch (token->kind){
        case TokenBaseKind_Identifier:
        case TokenBaseKind_LiteralInteger:
        case TokenBaseKind_LiteralFloat:
        case TokenBaseKind_LiteralString:
        case TokenBaseKind_ParentheticalClose:
        case TokenBaseKind_ScopeClose:
        {
            result = true;
        }break;
        case TokenBaseKind_Keyword:
        {
            switch (token->sub_kind){
                case TokenCppKind_Break:
                case TokenCppKind_Continue:
                case TokenCppKind_Return:
                case TokenCppKind_NullPtr:
                {
                    result = true;
                }break;
                case TokenCppKind_KeywordGeneric:
                {
                    String_Const_u8 lexeme = SCu8(str + token->pos, token->size);
                    result = (string_match(lexeme, string_u8_litexpr("fallthrough")) ||
                              string_match(lexeme, string_u8_litexpr("iota")));
                }break;
            }
        }break;
        case TokenBaseKind_Operator:
        {
            result = (token->sub_kind == TokenCppKind_PlusPlus ||
                      token->sub_kind == TokenCppKind_MinusMinus);
        }break;
    }
    return(result);
}

function i64
go_lex_number(u8 *str, i64 i, i64 size, b32 *is_float_out){
    b32 is_float = false;
    if (str[i] == '0' && i + 1 < size &&
        (str[i + 1] == 'x' || str[i + 1] == 'X' ||
         str[i + 1] == 'o' || str[i + 1] == 'O' ||
         str[i + 1] == 'b' || str[i + 1] == 'B')){
        b32 hex = (str[i + 1] == 'x' || str[i + 1] == 'X');
        i += 2;
        for (;i < size;){
            u8 c = str[i];
            if (character_is_alpha_numeric(c) || c == '.'){
                if (hex && (c == 'p' || c == 'P')){
                    is_float = true;
                    i += 1;
                    if (i < size && (str[i] == '+' || str[i] == '-')){
                        i += 1;
                    }
                    continue;
                }
                if (c == '.'){
                    is_float = true;
                }
                i += 1;
            }
            else{
                break;
            }
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
    }
    if (i < size && str[i] == 'i'){
        is_float = true;
        i += 1;
    }
    *is_float_out = is_float;
    return(i);
}

function Token_List
lex_full_input_go(Arena *arena, String_Const_u8 input){
    Token_List list = {};
    u8 *str = input.str;
    i64 size = (i64)input.size;
    i64 i = 0;

    // NOTE: Last significant (non-whitespace, non-comment) token, used for
    // semicolon insertion at newlines.
    Token *last_significant = 0;
    
    // NOTE: Track "case"/"default" labels so their ':' ends the statement.
    b32 in_case_label = false;
    i32 paren_depth = 0;
    i32 case_paren_depth = 0;

    for (;i < size;){
        Token token = {};
        token.pos = i;
        u8 c = str[i];
        b32 contains_newline = false;

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'){
            for (;i < size;){
                u8 w = str[i];
                if (!(w == ' ' || w == '\t' || w == '\n' || w == '\r' || w == '\f' || w == '\v')){
                    break;
                }
                if (w == '\n'){
                    contains_newline = true;
                }
                i += 1;
            }
            token.kind = TokenBaseKind_Whitespace;
            token.sub_kind = TokenCppKind_Whitespace;
        }
        else if (c == '/' && i + 1 < size && str[i + 1] == '/'){
            for (;i < size && str[i] != '\n';){
                i += 1;
            }
            token.kind = TokenBaseKind_Comment;
            token.sub_kind = TokenCppKind_LineComment;
        }
        else if (c == '/' && i + 1 < size && str[i + 1] == '*'){
            i += 2;
            for (;i < size;){
                if (str[i] == '*' && i + 1 < size && str[i + 1] == '/'){
                    i += 2;
                    break;
                }
                if (str[i] == '\n'){
                    contains_newline = true;
                }
                i += 1;
            }
            i = clamp_top(i, size);
            token.kind = TokenBaseKind_Comment;
            token.sub_kind = TokenCppKind_BlockComment;
        }
        else if (c == '"'){
            i += 1;
            for (;i < size && str[i] != '"' && str[i] != '\n';){
                if (str[i] == '\\'){
                    i += 1;
                }
                i += 1;
            }
            if (i < size && str[i] == '"'){
                i += 1;
            }
            i = clamp_top(i, size);
            token.kind = TokenBaseKind_LiteralString;
            token.sub_kind = TokenCppKind_LiteralString;
        }
        else if (c == '`'){
            i += 1;
            for (;i < size && str[i] != '`';){
                i += 1;
            }
            if (i < size){
                i += 1;
            }
            token.kind = TokenBaseKind_LiteralString;
            token.sub_kind = TokenCppKind_LiteralStringRaw;
        }
        else if (c == '\''){
            i += 1;
            for (;i < size && str[i] != '\'' && str[i] != '\n';){
                if (str[i] == '\\'){
                    i += 1;
                }
                i += 1;
            }
            if (i < size && str[i] == '\''){
                i += 1;
            }
            i = clamp_top(i, size);
            token.kind = TokenBaseKind_LiteralString;
            token.sub_kind = TokenCppKind_LiteralCharacter;
        }
        else if (go_char_is_ident_start(c)){
            for (;i < size && go_char_is_ident(str[i]);){
                i += 1;
            }
            String_Const_u8 lexeme = SCu8(str + token.pos, i - token.pos);
            token.kind = TokenBaseKind_Identifier;
            token.sub_kind = TokenCppKind_Identifier;
            for (i32 k = 0; k < ArrayCount(go_keywords); k += 1){
                if (string_match(lexeme, go_keywords[k].text)){
                    token.kind = go_keywords[k].base_kind;
                    token.sub_kind = go_keywords[k].sub_kind;
                    break;
                }
            }
        }
        else if (character_is_base10(c) ||
                 (c == '.' && i + 1 < size && character_is_base10(str[i + 1]))){
            b32 is_float = false;
            if (c == '.'){
                i = go_lex_number(str, i + 1, size, &is_float);
                is_float = true;
            }
            else{
                i = go_lex_number(str, i, size, &is_float);
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
        else{
            b32 matched = false;
            for (i32 k = 0; k < ArrayCount(go_operators); k += 1){
                char *op = go_operators[k].text;
                i64 len = (i64)cstring_length(op);
                if (i + len <= size && block_match(str + i, op, len)){
                    i += len;
                    token.kind = go_operators[k].base_kind;
                    token.sub_kind = go_operators[k].sub_kind;
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

        if (token.kind == TokenBaseKind_ParentheticalOpen || token.kind == TokenBaseKind_ScopeOpen){
            paren_depth += 1;
        }
        else if (token.kind == TokenBaseKind_ParentheticalClose || token.kind == TokenBaseKind_ScopeClose){
            paren_depth -= 1;
        }
        if (token.kind == TokenBaseKind_Keyword &&
            (token.sub_kind == TokenCppKind_Case || token.sub_kind == TokenCppKind_Default)){
            token.flags |= TokenBaseFlag_OutdentLabel;
            in_case_label = true;
            case_paren_depth = paren_depth;
        }
        else if (in_case_label && token.sub_kind == TokenCppKind_Colon &&
                 paren_depth == case_paren_depth){
            token.flags |= TokenBaseFlag_StatementEnd;
            in_case_label = false;
        }

        if (contains_newline && last_significant != 0){
            if (go_token_ends_statement(last_significant, str)){
                last_significant->flags |= TokenBaseFlag_StatementEnd;
            }
            last_significant = 0;
        }

        token_list_push(arena, &list, &token);

        if (token.kind != TokenBaseKind_Whitespace &&
            token.kind != TokenBaseKind_Comment){
            Token_Block *block = list.last;
            last_significant = &block->tokens[block->count - 1];
        }
    }

    if (last_significant != 0 && go_token_ends_statement(last_significant, str)){
        last_significant->flags |= TokenBaseFlag_StatementEnd;
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

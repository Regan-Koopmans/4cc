/*
4coder_data_langs.cpp - Lexers for Markdown, YAML, and TOML.
*/

// TOP

#if !defined(FCODER_DATA_LANGS_CPP)
#define FCODER_DATA_LANGS_CPP

// NOTE: These lexers exist for highlighting. They map constructs onto the
// base kinds the theme already colors:
//   Keyword        - headings, keys, table headers
//   Preprocessor   - markdown fences/quotes/list markers, yaml anchors/tags
//   LiteralString  - strings, inline code, code blocks
//   LiteralInteger / LiteralFloat - numbers
//   LiteralTrue/False sub kinds - booleans
//   Comment        - comments

////////////////////////////////
// NOTE: Shared helpers

function void
data_lex_push(Arena *arena, Token_List *list, i64 pos, i64 end, Token_Base_Kind kind, i16 sub_kind){
    if (end > pos){
        Token token = {};
        token.pos = pos;
        token.size = end - pos;
        token.kind = kind;
        token.sub_kind = sub_kind;
        token_list_push(arena, list, &token);
    }
}

function void
data_lex_push_eof(Arena *arena, Token_List *list, i64 size){
    Token eof = {};
    eof.pos = size;
    eof.kind = TokenBaseKind_EOF;
    eof.sub_kind = TokenCppKind_EOF;
    token_list_push(arena, list, &eof);
}

function b32
data_is_space(u8 c){
    return(c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}

// NOTE: Emits whitespace (including newlines) starting at i; returns new i.
function i64
data_lex_whitespace(Arena *arena, Token_List *list, u8 *str, i64 i, i64 size){
    i64 start = i;
    for (;i < size && (data_is_space(str[i]) || str[i] == '\n');){
        i += 1;
    }
    data_lex_push(arena, list, start, i, TokenBaseKind_Whitespace, TokenCppKind_Whitespace);
    return(i);
}

function i64
get_line_start_from_pos_raw(u8 *str, i64 i){
    for (;i > 0 && str[i - 1] != '\n';){
        i -= 1;
    }
    return(i);
}

function i64
get_line_indent_raw(u8 *str, i64 line_start, i64 size){
    i64 i = line_start;
    for (;i < size && (str[i] == ' ' || str[i] == '\t');){
        i += 1;
    }
    return(i - line_start);
}

function i64
data_line_end(u8 *str, i64 i, i64 size){
    for (;i < size && str[i] != '\n';){
        i += 1;
    }
    return(i);
}

function i64
data_lex_quoted(u8 *str, i64 i, i64 size, b32 allow_escape, b32 multiline){
    u8 quote = str[i];
    i += 1;
    for (;i < size;){
        u8 c = str[i];
        if (allow_escape && c == '\\'){
            i += 2;
            continue;
        }
        if (c == quote){
            // NOTE: YAML single quotes escape by doubling.
            if (!allow_escape && quote == '\'' && i + 1 < size && str[i + 1] == '\''){
                i += 2;
                continue;
            }
            i += 1;
            break;
        }
        if (c == '\n' && !multiline){
            break;
        }
        i += 1;
    }
    return(clamp_top(i, size));
}

// NOTE: Classifies a bare scalar for YAML/TOML: bool, null, int, float.
function void
data_classify_scalar(String_Const_u8 s, Token_Base_Kind *kind, i16 *sub_kind){
    *kind = TokenBaseKind_Identifier;
    *sub_kind = TokenCppKind_Identifier;
    if (string_match(s, string_u8_litexpr("true")) ||
        string_match(s, string_u8_litexpr("True")) ||
        string_match(s, string_u8_litexpr("TRUE")) ||
        string_match(s, string_u8_litexpr("yes")) ||
        string_match(s, string_u8_litexpr("on"))){
        *kind = TokenBaseKind_LiteralInteger;
        *sub_kind = TokenCppKind_LiteralTrue;
        return;
    }
    if (string_match(s, string_u8_litexpr("false")) ||
        string_match(s, string_u8_litexpr("False")) ||
        string_match(s, string_u8_litexpr("FALSE")) ||
        string_match(s, string_u8_litexpr("no")) ||
        string_match(s, string_u8_litexpr("off"))){
        *kind = TokenBaseKind_LiteralInteger;
        *sub_kind = TokenCppKind_LiteralFalse;
        return;
    }
    if (string_match(s, string_u8_litexpr("null")) ||
        string_match(s, string_u8_litexpr("Null")) ||
        string_match(s, string_u8_litexpr("NULL")) ||
        string_match(s, string_u8_litexpr("~")) ||
        string_match(s, string_u8_litexpr("inf")) ||
        string_match(s, string_u8_litexpr("nan"))){
        *kind = TokenBaseKind_Keyword;
        *sub_kind = TokenCppKind_NullPtr;
        return;
    }
    if (s.size == 0){
        return;
    }
    u64 j = 0;
    if (s.str[0] == '+' || s.str[0] == '-'){
        j = 1;
    }
    if (j >= s.size){
        return;
    }
    if (s.size > j + 1 && s.str[j] == '0' &&
        (s.str[j + 1] == 'x' || s.str[j + 1] == 'o' || s.str[j + 1] == 'b')){
        for (u64 k = j + 2; k < s.size; k += 1){
            if (!(character_is_base16(s.str[k]) || s.str[k] == '_')){
                return;
            }
        }
        *kind = TokenBaseKind_LiteralInteger;
        *sub_kind = TokenCppKind_LiteralInteger;
        return;
    }
    b32 saw_digit = false;
    b32 is_float = false;
    for (u64 k = j; k < s.size; k += 1){
        u8 c = s.str[k];
        if (character_is_base10(c) || c == '_'){
            saw_digit = saw_digit || character_is_base10(c);
        }
        else if (c == '.' || c == 'e' || c == 'E' ||
                 ((c == '+' || c == '-') && k > 0 && (s.str[k - 1] == 'e' || s.str[k - 1] == 'E'))){
            is_float = true;
        }
        else{
            return;
        }
    }
    if (saw_digit){
        *kind = is_float?TokenBaseKind_LiteralFloat:TokenBaseKind_LiteralInteger;
        *sub_kind = is_float?TokenCppKind_LiteralFloat64:TokenCppKind_LiteralInteger;
    }
}

////////////////////////////////
// NOTE: Markdown

function i64
markdown_lex_inline(Arena *arena, Token_List *list, u8 *str, i64 i, i64 end){
    i64 text_start = i;
    for (;i < end;){
        u8 c = str[i];
        if (c == '`'){
            data_lex_push(arena, list, text_start, i, TokenBaseKind_Identifier, TokenCppKind_Identifier);
            i64 start = i;
            i64 ticks = 0;
            for (;i < end && str[i] == '`';){
                ticks += 1;
                i += 1;
            }
            // NOTE: Find matching run of the same length on this line.
            i64 close = -1;
            for (i64 k = i; k < end;){
                if (str[k] == '`'){
                    i64 run = 0;
                    i64 m = k;
                    for (;m < end && str[m] == '`';){
                        run += 1;
                        m += 1;
                    }
                    if (run == ticks){
                        close = m;
                        break;
                    }
                    k = m;
                }
                else{
                    k += 1;
                }
            }
            i = (close >= 0)?close:end;
            data_lex_push(arena, list, start, i, TokenBaseKind_LiteralString, TokenCppKind_LiteralString);
            text_start = i;
        }
        else if ((c == '*' || c == '_') && i + 1 < end && str[i + 1] == c){
            // NOTE: Bold. Color the delimited span as a keyword.
            i64 k = i + 2;
            for (;k + 1 < end && !(str[k] == c && str[k + 1] == c);){
                k += 1;
            }
            if (k + 1 < end){
                data_lex_push(arena, list, text_start, i, TokenBaseKind_Identifier, TokenCppKind_Identifier);
                data_lex_push(arena, list, i, k + 2, TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric);
                i = k + 2;
                text_start = i;
            }
            else{
                i += 2;
            }
        }
        else if (c == '[' || (c == '!' && i + 1 < end && str[i + 1] == '[')){
            // NOTE: Link or image: [text](url)
            i64 k = i + ((c == '!')?2:1);
            for (;k < end && str[k] != ']';){
                k += 1;
            }
            if (k + 1 < end && str[k + 1] == '('){
                i64 m = k + 2;
                for (;m < end && str[m] != ')';){
                    m += 1;
                }
                if (m < end){
                    data_lex_push(arena, list, text_start, i, TokenBaseKind_Identifier, TokenCppKind_Identifier);
                    data_lex_push(arena, list, i, k + 1, TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric);
                    data_lex_push(arena, list, k + 1, m + 1, TokenBaseKind_LiteralString, TokenCppKind_PPIncludeFile);
                    i = m + 1;
                    text_start = i;
                    continue;
                }
            }
            i += 1;
        }
        else{
            i += 1;
        }
    }
    data_lex_push(arena, list, text_start, end, TokenBaseKind_Identifier, TokenCppKind_Identifier);
    return(end);
}

function Token_List
lex_full_input_markdown(Arena *arena, String_Const_u8 input){
    Token_List list = {};
    u8 *str = input.str;
    i64 size = (i64)input.size;
    i64 i = 0;
    b32 in_fence = false;
    u8 fence_char = 0;
    i64 fence_len = 0;

    for (;i < size;){
        if (str[i] == '\n' || data_is_space(str[i])){
            // NOTE: Leading whitespace of a line (and blank lines).
            i64 ws_start = i;
            for (;i < size && (data_is_space(str[i]) || str[i] == '\n');){
                i += 1;
            }
            if (!in_fence){
                data_lex_push(arena, &list, ws_start, i, TokenBaseKind_Whitespace, TokenCppKind_Whitespace);
                continue;
            }
            i = ws_start;
        }

        i64 line_start = i;
        i64 line_end = data_line_end(str, i, size);
        i64 p = line_start;
        for (;p < line_end && data_is_space(str[p]);){
            p += 1;
        }

        // Code fence open/close.
        if (p + 2 < line_end + 1 && (str[p] == '`' || str[p] == '~')){
            u8 fc = str[p];
            i64 run = 0;
            i64 m = p;
            for (;m < line_end && str[m] == fc;){
                run += 1;
                m += 1;
            }
            if (run >= 3){
                if (!in_fence){
                    in_fence = true;
                    fence_char = fc;
                    fence_len = run;
                    data_lex_push(arena, &list, line_start, line_end, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
                    i = line_end;
                    continue;
                }
                else if (fc == fence_char && run >= fence_len){
                    in_fence = false;
                    data_lex_push(arena, &list, line_start, line_end, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
                    i = line_end;
                    continue;
                }
            }
        }

        if (in_fence){
            data_lex_push(arena, &list, line_start, line_end, TokenBaseKind_LiteralString, TokenCppKind_LiteralStringRaw);
            i = line_end;
            if (i < size){
                // Newline inside a fence.
                data_lex_push(arena, &list, i, i + 1, TokenBaseKind_Whitespace, TokenCppKind_Whitespace);
                i += 1;
            }
            continue;
        }

        // Heading.
        if (str[p] == '#'){
            i64 m = p;
            for (;m < line_end && str[m] == '#';){
                m += 1;
            }
            if (m - p <= 6 && (m == line_end || str[m] == ' ' || str[m] == '\t')){
                data_lex_push(arena, &list, line_start, line_end, TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric);
                i = line_end;
                continue;
            }
        }

        // Horizontal rule.
        {
            u8 rc = str[p];
            if (rc == '-' || rc == '*' || rc == '_'){
                i64 count = 0;
                b32 only = true;
                for (i64 m = p; m < line_end; m += 1){
                    if (str[m] == rc){
                        count += 1;
                    }
                    else if (!data_is_space(str[m])){
                        only = false;
                        break;
                    }
                }
                if (only && count >= 3){
                    data_lex_push(arena, &list, line_start, line_end, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
                    i = line_end;
                    continue;
                }
            }
        }

        i64 content = p;
        // Block quote marker.
        if (str[p] == '>'){
            content = p + 1;
            data_lex_push(arena, &list, line_start, content, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
        }
        // List markers: "-", "*", "+", "1." / "1)"
        else if ((str[p] == '-' || str[p] == '*' || str[p] == '+') &&
                 p + 1 < line_end && str[p + 1] == ' '){
            content = p + 1;
            data_lex_push(arena, &list, line_start, content, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
        }
        else if (character_is_base10(str[p])){
            i64 m = p;
            for (;m < line_end && character_is_base10(str[m]);){
                m += 1;
            }
            if (m < line_end && (str[m] == '.' || str[m] == ')') &&
                m + 1 < line_end && str[m + 1] == ' '){
                content = m + 1;
                data_lex_push(arena, &list, line_start, content, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
            }
        }
        if (content == p && line_start < p){
            data_lex_push(arena, &list, line_start, p, TokenBaseKind_Whitespace, TokenCppKind_Whitespace);
        }

        markdown_lex_inline(arena, &list, str, content, line_end);
        i = line_end;
    }

    data_lex_push_eof(arena, &list, size);
    return(list);
}

////////////////////////////////
// NOTE: YAML

function Token_List
lex_full_input_yaml(Arena *arena, String_Const_u8 input){
    Token_List list = {};
    u8 *str = input.str;
    i64 size = (i64)input.size;
    i64 i = 0;

    // NOTE: Block scalar (| or >) state: lines indented deeper than the key
    // are literal text.
    b32 in_block_scalar = false;
    i64 block_parent_indent = 0;

    for (;i < size;){
        if (str[i] == '\n' || data_is_space(str[i])){
            i = data_lex_whitespace(arena, &list, str, i, size);
            continue;
        }

        i64 line_start = get_line_start_from_pos_raw(str, i);
        i64 indent = i - line_start;
        i64 line_end = data_line_end(str, i, size);
        b32 at_line_start = true;
        for (i64 k = line_start; k < i; k += 1){
            if (!data_is_space(str[k])){
                at_line_start = false;
                break;
            }
        }

        if (in_block_scalar){
            if (at_line_start && indent > block_parent_indent){
                data_lex_push(arena, &list, i, line_end, TokenBaseKind_LiteralString, TokenCppKind_LiteralStringRaw);
                i = line_end;
                continue;
            }
            in_block_scalar = false;
        }

        // Document markers.
        if (at_line_start && indent == 0 && i + 3 <= size &&
            (block_match(str + i, (void*)"---", 3) || block_match(str + i, (void*)"...", 3)) &&
            (i + 3 == size || data_is_space(str[i + 3]) || str[i + 3] == '\n')){
            data_lex_push(arena, &list, i, i + 3, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
            i += 3;
            continue;
        }

        u8 c = str[i];
        if (c == '#'){
            data_lex_push(arena, &list, i, line_end, TokenBaseKind_Comment, TokenCppKind_LineComment);
            i = line_end;
            continue;
        }
        if (c == '"' || c == '\''){
            i64 end = data_lex_quoted(str, i, size, c == '"', true);
            // NOTE: Quoted key?
            i64 m = end;
            for (;m < line_end && data_is_space(str[m]);){
                m += 1;
            }
            b32 is_key = (m < line_end && str[m] == ':' &&
                          (m + 1 >= size || data_is_space(str[m + 1]) || str[m + 1] == '\n'));
            data_lex_push(arena, &list, i, end,
                          is_key?TokenBaseKind_Keyword:TokenBaseKind_LiteralString,
                          is_key?TokenCppKind_KeywordGeneric:TokenCppKind_LiteralString);
            i = end;
            continue;
        }
        if (c == '-' && (i + 1 >= size || data_is_space(str[i + 1]) || str[i + 1] == '\n')){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
            i += 1;
            continue;
        }
        if (c == '&' || c == '*' || c == '!'){
            i64 start = i;
            i += 1;
            if (c == '!' && i < size && str[i] == '!'){
                i += 1;
            }
            for (;i < size && !data_is_space(str[i]) && str[i] != '\n' &&
                 str[i] != ',' && str[i] != ']' && str[i] != '}';){
                i += 1;
            }
            data_lex_push(arena, &list, start, i, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
            continue;
        }
        if (c == '|' || c == '>'){
            // Block scalar indicator with optional chomping/indent modifiers.
            i64 start = i;
            i += 1;
            for (;i < line_end && (str[i] == '-' || str[i] == '+' || character_is_base10(str[i]));){
                i += 1;
            }
            data_lex_push(arena, &list, start, i, TokenBaseKind_Operator, TokenCppKind_Or);
            in_block_scalar = true;
            block_parent_indent = get_line_indent_raw(str, line_start, size);
            continue;
        }
        if (c == '[' || c == '{'){
            data_lex_push(arena, &list, i, i + 1,
                          (c == '[')?TokenBaseKind_ParentheticalOpen:TokenBaseKind_ScopeOpen,
                          (c == '[')?TokenCppKind_BrackOp:TokenCppKind_BraceOp);
            i += 1;
            continue;
        }
        if (c == ']' || c == '}'){
            data_lex_push(arena, &list, i, i + 1,
                          (c == ']')?TokenBaseKind_ParentheticalClose:TokenBaseKind_ScopeClose,
                          (c == ']')?TokenCppKind_BrackCl:TokenCppKind_BraceCl);
            i += 1;
            continue;
        }
        if (c == ','){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_StatementClose, TokenCppKind_Comma);
            i += 1;
            continue;
        }
        if (c == ':' && (i + 1 >= size || data_is_space(str[i + 1]) || str[i + 1] == '\n')){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_Operator, TokenCppKind_Colon);
            i += 1;
            continue;
        }

        // Plain scalar: runs until ": ", " #", flow indicator, or line end.
        i64 start = i;
        for (;i < line_end;){
            u8 d = str[i];
            if (d == ':' && (i + 1 >= size || data_is_space(str[i + 1]) || str[i + 1] == '\n')){
                break;
            }
            if (d == '#' && i > start && data_is_space(str[i - 1])){
                break;
            }
            if (d == ',' || d == ']' || d == '}'){
                break;
            }
            i += 1;
        }
        i64 end = i;
        for (;end > start && data_is_space(str[end - 1]);){
            end -= 1;
        }
        b32 is_key = (i < line_end && str[i] == ':');
        if (is_key){
            data_lex_push(arena, &list, start, end, TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric);
        }
        else{
            Token_Base_Kind kind = 0;
            i16 sub_kind = 0;
            data_classify_scalar(SCu8(str + start, end - start), &kind, &sub_kind);
            if (kind == TokenBaseKind_Identifier){
                kind = TokenBaseKind_LiteralString;
                sub_kind = TokenCppKind_LiteralString;
            }
            data_lex_push(arena, &list, start, end, kind, sub_kind);
        }
        if (end < i){
            data_lex_push(arena, &list, end, i, TokenBaseKind_Whitespace, TokenCppKind_Whitespace);
        }
    }

    data_lex_push_eof(arena, &list, size);
    return(list);
}

////////////////////////////////
// NOTE: TOML

function Token_List
lex_full_input_toml(Arena *arena, String_Const_u8 input){
    Token_List list = {};
    u8 *str = input.str;
    i64 size = (i64)input.size;
    i64 i = 0;
    i32 bracket_depth = 0;
    i32 brace_depth = 0;
    b32 expect_key = true;

    for (;i < size;){
        u8 c = str[i];
        if (c == '\n'){
            if (bracket_depth == 0 && brace_depth == 0){
                expect_key = true;
            }
            i = data_lex_whitespace(arena, &list, str, i, size);
            continue;
        }
        if (data_is_space(c)){
            i64 start = i;
            for (;i < size && data_is_space(str[i]);){
                i += 1;
            }
            data_lex_push(arena, &list, start, i, TokenBaseKind_Whitespace, TokenCppKind_Whitespace);
            continue;
        }
        if (c == '#'){
            i64 end = data_line_end(str, i, size);
            data_lex_push(arena, &list, i, end, TokenBaseKind_Comment, TokenCppKind_LineComment);
            i = end;
            continue;
        }
        // Table header: [name] or [[name]] at line start.
        if (c == '[' && expect_key && bracket_depth == 0 && brace_depth == 0){
            i64 end = data_line_end(str, i, size);
            i64 m = i;
            for (;m < end && str[m] != ']';){
                m += 1;
            }
            for (;m < end && str[m] == ']';){
                m += 1;
            }
            data_lex_push(arena, &list, i, m, TokenBaseKind_Preprocessor, TokenCppKind_PPUnknown);
            i = m;
            continue;
        }
        if (c == '"' || c == '\''){
            i64 start = i;
            b32 triple = (i + 2 < size && str[i + 1] == c && str[i + 2] == c);
            if (triple){
                i += 3;
                for (;i < size;){
                    if (c == '"' && str[i] == '\\'){
                        i += 2;
                        continue;
                    }
                    if (str[i] == c && i + 2 < size && str[i + 1] == c && str[i + 2] == c){
                        i += 3;
                        // NOTE: Up to two extra quotes may close the string.
                        for (i32 extra = 0; extra < 2 && i < size && str[i] == c; extra += 1){
                            i += 1;
                        }
                        break;
                    }
                    i += 1;
                }
                i = clamp_top(i, size);
            }
            else{
                i = data_lex_quoted(str, i, size, c == '"', false);
            }
            b32 is_key = expect_key && brace_depth >= 0 && bracket_depth == 0;
            data_lex_push(arena, &list, start, i,
                          is_key?TokenBaseKind_Keyword:TokenBaseKind_LiteralString,
                          is_key?TokenCppKind_KeywordGeneric:(triple?TokenCppKind_LiteralStringRaw:TokenCppKind_LiteralString));
            continue;
        }
        if (c == '='){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_Operator, TokenCppKind_Eq);
            i += 1;
            expect_key = false;
            continue;
        }
        if (c == '.'){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_Operator, TokenCppKind_Dot);
            i += 1;
            continue;
        }
        if (c == ','){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_StatementClose, TokenCppKind_Comma);
            i += 1;
            if (brace_depth > 0 && bracket_depth == 0){
                expect_key = true;
            }
            continue;
        }
        if (c == '['){
            bracket_depth += 1;
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_ParentheticalOpen, TokenCppKind_BrackOp);
            i += 1;
            continue;
        }
        if (c == ']'){
            bracket_depth = clamp_bot(0, bracket_depth - 1);
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_ParentheticalClose, TokenCppKind_BrackCl);
            i += 1;
            continue;
        }
        if (c == '{'){
            brace_depth += 1;
            expect_key = true;
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_ScopeOpen, TokenCppKind_BraceOp);
            i += 1;
            continue;
        }
        if (c == '}'){
            brace_depth = clamp_bot(0, brace_depth - 1);
            expect_key = false;
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_ScopeClose, TokenCppKind_BraceCl);
            i += 1;
            continue;
        }

        // Bare key or value: letters, digits, '_', '-', '+', ':', '.' in values.
        i64 start = i;
        for (;i < size;){
            u8 d = str[i];
            if (character_is_alpha_numeric(d) || d == '-' || d == '+' ||
                (!expect_key && (d == '.' || d == ':'))){
                i += 1;
            }
            else if (!expect_key && d == ' ' && i + 1 < size && character_is_base10(str[i + 1]) &&
                     i > start && character_is_base10(str[i - 1])){
                // NOTE: Datetime with space separator.
                i += 1;
            }
            else{
                break;
            }
        }
        if (i == start){
            data_lex_push(arena, &list, i, i + 1, TokenBaseKind_LexError, TokenCppKind_LexError);
            i += 1;
            continue;
        }
        String_Const_u8 lexeme = SCu8(str + start, i - start);
        if (expect_key){
            data_lex_push(arena, &list, start, i, TokenBaseKind_Keyword, TokenCppKind_KeywordGeneric);
        }
        else{
            Token_Base_Kind kind = 0;
            i16 sub_kind = 0;
            data_classify_scalar(lexeme, &kind, &sub_kind);
            if (kind == TokenBaseKind_Identifier && lexeme.size > 0 && character_is_base10(lexeme.str[0])){
                // NOTE: Dates and times.
                kind = TokenBaseKind_LiteralInteger;
                sub_kind = TokenCppKind_LiteralInteger;
            }
            data_lex_push(arena, &list, start, i, kind, sub_kind);
        }
    }

    data_lex_push_eof(arena, &list, size);
    return(list);
}

#endif

// BOTTOM

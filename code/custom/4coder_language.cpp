/*
4coder_language.cpp - Per-buffer language selection and lexer dispatch.
*/

// TOP

#if !defined(FCODER_LANGUAGE_CPP)
#define FCODER_LANGUAGE_CPP

function Buffer_Language
buffer_language_from_file_name(String_Const_u8 file_name){
    Buffer_Language result = BufferLanguage_Cpp;
    String_Const_u8 ext = string_file_extension(file_name);
    if (string_match(ext, string_u8_litexpr("py")) ||
        string_match(ext, string_u8_litexpr("pyw")) ||
        string_match(ext, string_u8_litexpr("pyi"))){
        result = BufferLanguage_Python;
    }
    else if (string_match(ext, string_u8_litexpr("go"))){
        result = BufferLanguage_Go;
    }
    else if (string_match(ext, string_u8_litexpr("md")) ||
             string_match(ext, string_u8_litexpr("markdown"))){
        result = BufferLanguage_Markdown;
    }
    else if (string_match(ext, string_u8_litexpr("yaml")) ||
             string_match(ext, string_u8_litexpr("yml"))){
        result = BufferLanguage_Yaml;
    }
    else if (string_match(ext, string_u8_litexpr("toml"))){
        result = BufferLanguage_Toml;
    }
    return(result);
}

function Buffer_Language
buffer_get_language(Application_Links *app, Buffer_ID buffer){
    Buffer_Language result = BufferLanguage_Cpp;
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Buffer_Language *ptr = scope_attachment(app, scope, buffer_language, Buffer_Language);
    if (ptr != 0){
        result = *ptr;
    }
    return(result);
}

function Token_List
lex_full_input_for_language(Arena *arena, Buffer_Language language, String_Const_u8 input){
    Token_List result = {};
    switch (language){
        case BufferLanguage_Python:
        {
            result = lex_full_input_python(arena, input);
        }break;
        case BufferLanguage_Go:
        {
            result = lex_full_input_go(arena, input);
        }break;
        case BufferLanguage_Markdown:
        {
            result = lex_full_input_markdown(arena, input);
        }break;
        case BufferLanguage_Yaml:
        {
            result = lex_full_input_yaml(arena, input);
        }break;
        case BufferLanguage_Toml:
        {
            result = lex_full_input_toml(arena, input);
        }break;
        default:
        {
            result = lex_full_input_cpp(arena, input);
        }break;
    }
    return(result);
}

// NOTE: Languages where indentation is meaningful or the content is prose.
// The brace indenter must not touch them; newlines carry indentation instead.
function b32
language_uses_line_indent(Buffer_Language language){
    return(language == BufferLanguage_Python ||
           language == BufferLanguage_Markdown ||
           language == BufferLanguage_Yaml ||
           language == BufferLanguage_Toml);
}

function String_Const_u8
language_line_comment(Buffer_Language language){
    String_Const_u8 result = string_u8_litexpr("//");
    switch (language){
        case BufferLanguage_Python:
        case BufferLanguage_Yaml:
        case BufferLanguage_Toml:
        {
            result = string_u8_litexpr("#");
        }break;
    }
    return(result);
}

#endif

// BOTTOM

#ifndef hsfv_ctype_h
#define hsfv_ctype_h

#ifdef __cplusplus
extern "C" {
#endif

#define HSFV_IS_DIGIT(c) ('0' <= (c) && (c) <= '9')
#define HSFV_IS_LCALPHA(c) (('a' <= (c) && (c) <= 'z'))
#define HSFV_IS_ALPHA(c) (('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z'))

#define HSFV_IS_KEY_LEAADING_CHAR(c) (HSFV_IS_LCALPHA(c) || (c) == '*')
#define HSFV_IS_KEY_TRAILING_CHAR(c)                                                                                               \
    (HSFV_IS_LCALPHA(c) || HSFV_IS_DIGIT(c) || (c) == '_' || (c) == '-' || (c) == '.' || (c) == '*')

#define HSFV_IS_TOKEN_LEADING_CHAR(c) (HSFV_IS_ALPHA(c) || (c) == '*')

/*
 * token_trailing_char = tchar / ":" / "/"
 * tchar is defined at
 * https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2.6
 */
extern const char hsfv_token_trailing_char_map[256];
#define HSFV_IS_TOKEN_TRAILING_CHAR(c) hsfv_token_trailing_char_map[(unsigned char)(c)]

#define HSFV_IS_ASCII(c) ((c) <= '\x7f')

bool hsfv_is_ascii_string(const char *input, const char *input_end);

#ifdef __cplusplus
}
#endif

#endif

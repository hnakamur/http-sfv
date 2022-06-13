#ifndef hsfv_ctype_h
#define hsfv_ctype_h

#ifdef __cplusplus
extern "C" {
#endif

#define hsfv_is_digit(c) ('0' <= (c) && (c) <= '9')
#define hsfv_is_lcalpha(c) (('a' <= (c) && (c) <= 'z'))
#define hsfv_is_alpha(c) (('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z'))

#define hsfv_is_key_leaading_char(c) (hsfv_is_lcalpha(c) || (c) == '*')
#define hsfv_is_key_trailing_char(c)                                                                                               \
    (hsfv_is_lcalpha(c) || hsfv_is_digit(c) || (c) == '_' || (c) == '-' || (c) == '.' || (c) == '*')

#define hsfv_is_token_leading_char(c) (hsfv_is_alpha(c) || (c) == '*')

/*
 * token_trailing_char = tchar / ":" / "/"
 * tchar is defined at
 * https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2.6
 */
extern const char hsfv_token_trailing_char_map[256];
#define hsfv_is_trailing_token_char(c) hsfv_token_trailing_char_map[(unsigned char)(c)]

#define hsfv_is_ascii(c) ((c) <= '\x7f')

static inline bool hsfv_is_ascii_string(const char *input, const char *input_end)
{
    for (; input < input_end; ++input) {
        if (!hsfv_is_ascii(*input)) {
            return false;
        }
    }
    return true;
}

#ifdef __cplusplus
}
#endif

#endif

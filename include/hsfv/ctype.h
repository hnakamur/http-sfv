#ifndef hsfv_ctype_h
#define hsfv_ctype_h

#ifdef __cplusplus
extern "C" {
#endif

#define HSFV_IS_DIGIT(c) ('0' <= (c) && (c) <= '9')

extern const char hsfv_key_leading_char_map[256];
extern const char hsfv_key_trailing_char_map[256];

#define HSFV_IS_KEY_LEADING_CHAR(c) hsfv_key_leading_char_map[(unsigned char)(c)]
#define HSFV_IS_KEY_TRAILING_CHAR(c) hsfv_key_trailing_char_map[(unsigned char)(c)]

extern const char hsfv_token_leading_char_map[256];
extern const char hsfv_token_trailing_char_map[256];

#define HSFV_IS_TOKEN_LEADING_CHAR(c) hsfv_token_leading_char_map[(unsigned char)(c)]
#define HSFV_IS_TOKEN_TRAILING_CHAR(c) hsfv_token_trailing_char_map[(unsigned char)(c)]

extern const char hsfv_base64_char_map[256];

#define HSFV_IS_BASE64_CHAR(c) hsfv_base64_char_map[(unsigned char)(c)]

#define HSFV_IS_ASCII(c) ((c) <= '\x7f')

bool hsfv_is_ascii_string(const char *input, const char *input_end);

#ifdef __cplusplus
}
#endif

#endif

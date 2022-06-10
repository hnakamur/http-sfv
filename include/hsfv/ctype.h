#ifndef hsfv_ctype_h
#define hsfv_ctype_h

#ifdef __cplusplus
extern "C" {
#endif

#define hsfv_is_digit(c) ('0' <= (c) && (c) <= '9')
#define hsfv_is_lcalpha(c) (('a' <= (c) && (c) <= 'z'))
#define hsfv_is_alpha(c)                                                       \
  (('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z'))

extern const char *hsfv_token_char_map;
#define hsfv_is_token_char(c) hsfv_token_char_map[(unsigned char)(c)]

#ifdef __cplusplus
}
#endif

#endif

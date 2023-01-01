#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include<string.h>
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

static int is_white_space(char c) {
    if ((c == ' ' || c == '\t' || c == '\n' || c == '\r'))
        return 1;
    return 0;
}


typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (is_white_space(p[0]))
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (strlen(c->json) < 3)
            return LEPT_PARSE_INVALID_VALUE;
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
            return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    if (!is_white_space(c->json[0]) && c->json[0]!=0) {
        v->type = LEPT_INVALID;
        return LEPT_PARSE_INVALID_VALUE;
    }
    lept_parse_whitespace(c);
    if (c->json[0] != 0) {
        v->type = LEPT_INVALID;
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (strlen(c->json) < 4)
    return LEPT_PARSE_INVALID_VALUE;
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    if (!is_white_space(c->json[0]) && c->json[0] != 0){
        v->type = LEPT_INVALID;
        return LEPT_PARSE_INVALID_VALUE;
    }

    lept_parse_whitespace(c);
    if (c->json[0] != 0) {
        v->type = LEPT_INVALID;
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (strlen(c->json) < 3) return LEPT_PARSE_INVALID_VALUE;
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    if (!is_white_space(c->json[0]) && c->json[0] != 0) {
        v->type = LEPT_INVALID;
        return LEPT_PARSE_INVALID_VALUE;
    }
    lept_parse_whitespace(c);
    if (c->json[0] != 0) {
        v->type = LEPT_INVALID;
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    v->type =  LEPT_NULL;
    return LEPT_PARSE_OK;
}



static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_null(c, v);
        case 't': return lept_parse_true(c, v);
        case 'f': return lept_parse_false(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_INVALID;
    lept_parse_whitespace(&c);
    return lept_parse_value(&c, v);
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include<string.h>
#include<math.h>
#include<errno.h>
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define IS_DIGIT1_9(n)     (n>='1'&&n<='9')
#define IS_DIGIT(n) (n>='0'&&n<='9')
typedef struct {
    const char* json;
}lept_context;



static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value*v, const char* key,lept_type type) {
    EXPECT(c, key[0]);
    if (strlen(c->json) < strlen(key)-1)
        return LEPT_PARSE_INVALID_VALUE;
    for (int i = 1; key[i+1];++i) {
        if (key[i] != c->json[0])
            return LEPT_PARSE_INVALID_VALUE;
        c->json++;
    }
    c->json++;
    v->type = type;
    return LEPT_PARSE_OK;
}


static int lept_parse_true(lept_context* c, lept_value* v) {
    return lept_parse_literal(c, v, "true",LEPT_TRUE);
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    return lept_parse_literal(c, v, "false",LEPT_FALSE);
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    return lept_parse_literal(c, v, "null",LEPT_NULL);
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    v->type = LEPT_INVALID;
    char* end;
    int itr=0;
    //符号位
    if (c->json[0] == '-') itr++;

    if (!IS_DIGIT(c->json[itr])) {//首字符必须为数字
        return LEPT_PARSE_INVALID_VALUE;
    }
    //特判0
    if (c->json[itr] == '0' && c->json[itr+1]!='.'){//排除以0为开头的正小数,剩下的肯定只能是数字0

        if (c->json[itr+1] != ' ' && c->json[itr + 1] != '\n' && c->json[itr + 1] != '\t' && c->json[itr + 1] != '\r' && c->json[itr + 1] != 0) {//数字0之后只允许有空白字符
            return LEPT_PARSE_INVALID_VALUE;
        }
    c->json += (itr + 1);
    v->n = 0;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
    }
    if (! IS_DIGIT1_9(c->json[itr]) && c->json[itr+1]!='.') {//除非是以0开头的小数，否则首字符只能是1-9
        return LEPT_PARSE_INVALID_VALUE;
    }

    int count_dot = 0;
    int count_e=0;
    while (IS_DIGIT(c->json[itr])||c->json[itr]=='.'||c->json[itr]=='E'||c->json[itr]=='e') {
        if (c->json[itr] == '.') {
            if (count_dot || count_e||!IS_DIGIT(c->json[itr+1])) {//小数点只能出现一次，E/e必须在小数点之前，小数点之后必然是数字
                return LEPT_PARSE_INVALID_VALUE;
            }
            ++count_dot;
            ++itr;
            continue;
        }
        if (c->json[itr] == 'e' || c->json[itr] == 'E') {//E/e只能出现一次，E/e之后是一个可以有符号的数字
            if (c->json[itr + 1] == '-' || c->json[itr + 1] == '+') {
                ++itr;
            }
            if (count_e||(!IS_DIGIT(c->json[itr+1]))){
                return LEPT_PARSE_INVALID_VALUE;
            }
            ++count_e;
            ++itr;
            continue;
        }
        ++itr;
    }

    v->n = strtod(c->json, &end);
    if ((v->n == HUGE_VAL || v->n == -HUGE_VAL)&&errno==ERANGE) {//溢出错误处理
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_INVALID;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_INVALID;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

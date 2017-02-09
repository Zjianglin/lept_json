#ifndef LEPT_JSON_H__
#define LEPT_JSON_H__

#include <iostream>
#include <string>
#include <cassert>

enum lept_type
{
    LEPT_NULL = 11,
    LEPT_TRUE,
    LEPT_FALSE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
};

struct lept_value
{
    union  {
        double num;
    } u;
    lept_type type;
};

enum parse_return
{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

class LeptJson
{
  public:
    LeptJson() { parsed_v_.type = LEPT_NULL; }

    int parse(const std::string &json) ;

    void set_type(const lept_type nt) { parsed_v_.type = nt; }
    lept_type   get_type() const { return parsed_v_.type; }
    double      get_number() const { assert(parsed_v_.type == LEPT_NUMBER); return parsed_v_.u.num; }

  private:
    struct lept_context {
        const char *json;
    };
    lept_value parsed_v_;

    inline void lept_parse_init() { parsed_v_.type = LEPT_NULL; }
    void lept_parse_whitespace(lept_context &ctx);
    int lept_parse_value(lept_context &ctx, lept_value &v);
    int lept_parse_literal(lept_context &ctx, lept_value &v, const std::string &literal, lept_type type);
    int lept_parse_number(lept_context &ctx, lept_value &v);
};

#endif
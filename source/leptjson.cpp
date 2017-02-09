#include "leptjson.h"
#include <cmath>  // HUGE_VAL 
#include <cerrno> // errno
#include <cstdlib> // strtod

#define EXPECT(c, ch) do { assert(*c.json == (ch)); c.json++; } while(0)
#define ISDIGITS(ch)    ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

void LeptJson::lept_parse_whitespace(lept_context &ctx)
{
    const char *p = ctx.json;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    ctx.json = p;
}

int LeptJson::lept_parse_literal(lept_context &ctx, lept_value &v, const std::string &literal, lept_type type)
{
    for(size_t i = 0; i < literal.size(); ++i) {
        if(*ctx.json != literal[i])
            return LEPT_PARSE_INVALID_VALUE;
        ctx.json++;
    }
    v.type = type;
    return LEPT_PARSE_OK;
}

int LeptJson::lept_parse_number(lept_context &ctx, lept_value &v)
{
    const char *p = ctx.json;
    if (*p == '-') p++; // minus
    if (*p == '0') p++; // integrate part
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(++p; ISDIGITS(*p); ++p) ;
    }
    if (*p == '.') { // decimal
        p++;
        if(!ISDIGITS(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (++p; ISDIGITS(*p); ++p) ;
    }
    if (*p =='e' || *p == 'E') { //exponent
        p++;
        if (*p =='+' || *p == '-') p++;
        if(!ISDIGITS(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (++p ; ISDIGITS(*p); ++p) ;
    }
    errno = 0;
    // try {
    //     v.u.num = std::stod(ctx.json, nullptr);
    //     std::cout << "ctx.json: " << ctx.json << " => " << v.u.num << std::endl;
    // } catch (std::out_of_range e) {
    //     std::cout << "std::stod out_of_range: " << e.what() << " ||| ctx.json: " << ctx.json << " => " << v.u.num << std::endl;
    //     return LEPT_PARSE_NUMBER_TOO_BIG;
    // }
    v.u.num = strtod(ctx.json, nullptr);
    if(errno == ERANGE && (v.u.num == HUGE_VAL || v.u.num == -HUGE_VAL)) 
        return LEPT_PARSE_NUMBER_TOO_BIG;
    
    v.type = LEPT_NUMBER;
    ctx.json = p;
    return LEPT_PARSE_OK;
}

int LeptJson::lept_parse_value(lept_context &ctx, lept_value &v)
{
    switch(*ctx.json) {
        case 'n': return lept_parse_literal(ctx, v, "null", LEPT_NULL);
        case 't': return lept_parse_literal(ctx, v, "true", LEPT_TRUE);
        case 'f': return lept_parse_literal(ctx, v, "false", LEPT_FALSE);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default: return lept_parse_number(ctx, v);
    }
}




int LeptJson::parse(const std::string &json)
{
    lept_context ctx;
    ctx.json = json.c_str();
    int ret;
    lept_parse_init();
    lept_parse_whitespace(ctx);
    if ((ret = lept_parse_value(ctx, parsed_v_)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(ctx);
        if (*ctx.json != '\0') {
            lept_parse_init();
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;

}
#include "leptjson.h"
#include <cmath>  // HUGE_VAL 
#include <cerrno> // errno
#include <cstdlib> // strtod
#include <cstring>

#define EXPECT(c, ch) do { assert(*c.json == (ch)); c.json++; } while(0)
#define ISDIGITS(ch)    ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

using std::shared_ptr;

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

#define PUTC(ctx, ch) do { *(char*)ctx.push(sizeof(char)) = (ch); } while (0)
const char* LeptJson::lept_parse_hex4(const char *json, unsigned &u)
{
    u = 0;
    for (unsigned i = 0; i < 4; ++i) {
        auto ch = json[i];
        if (ISDIGITS(ch))                   u = (u << 4) + (ch - '0');
        else if ((ch) >='A' && (ch) <= 'F') u = (u << 4) + (ch - 'A') + 10;
        else if ((ch) >='a' && (ch) <= 'f') u = (u << 4) + (ch - 'a') + 10;
        else return nullptr;
    }
    return json + 4;
}
#define OutputByte(ch) PUTC(ctx, ch)
void LeptJson::lept_encode_utf8(lept_context &ctx, unsigned u)
{
    assert(u <= 0x10FFFF);
    if (u <= 0x007F) {
        OutputByte(u);
    }
    else if (u <= 0x07FF) {
        OutputByte(0xC0 | ((u >> 6) & 0x1F));
        OutputByte(0x80 | ((u     ) & 0x3F));
    }
    else if (u <= 0xFFFF) {
        OutputByte(0xE0 | ((u >> 12) & 0x0F)); // 0xE0 = 11100000
        OutputByte(0x80 | ((u >>  6) & 0x3F)); // 0x80 = 10000000
        OutputByte(0x80 | ((u      ) & 0x3F)); // 0x3F = 00111111
    }
    else {
        OutputByte(0xF0 | ((u >>  18) & 0x07)); // 0xF0 = 11110000
        OutputByte(0x80 | ((u >>  12) & 0x3F)); // 0x3F = 00111111
        OutputByte(0x80 | ((u >>   6) & 0x3F)); // 0x07 = 00000111
        OutputByte(0x80 | ((u       ) & 0x3F)); // 0x80 = 10000000
    }
}

int LeptJson::lept_parse_string(lept_context &ctx, lept_value &v)
{
    const char *str;
    size_t len;
    int ret = lept_parse_string_raw(ctx, &str, len);
    if (ret != LEPT_PARSE_OK) 
        return ret;
    lept_set_string(v, str, len);
    return ret;
}


#define STRING_ERROR(ret) do { ctx.top = head; return ret; } while (0)
int LeptJson::lept_parse_string_raw(lept_context &ctx, const char **str, size_t &len)
{
    EXPECT(ctx, '\"');
    const auto *p = ctx.json;
    size_t head = ctx.top;
    for (;;) {
        auto ch = *p++;
        switch (ch) {
            case '\"':
                len = ctx.top - head;
                *str = (char*)ctx.pop(len);
                ctx.json = p;
                return LEPT_PARSE_OK;
            case '\0':
                STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
            case '\\':
                switch (*p++) {
                    case '\"': PUTC(ctx, '\"'); break;
                    case '\\': PUTC(ctx, '\\'); break;
                    case '/': PUTC(ctx, '/'); break;
                    case 'b': PUTC(ctx, '\b'); break;
                    case 'f': PUTC(ctx, '\f'); break;
                    case 'r': PUTC(ctx, '\r'); break;
                    case 'n': PUTC(ctx, '\n'); break;
                    case 't': PUTC(ctx, '\t'); break; 
                    case 'u':
                        unsigned u; // codepoint
                        if (!(p = lept_parse_hex4(p, u))) 
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) { //surrogate pair
                            unsigned ls;
                            if (*p != '\\' || *(p + 1) != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            p += 2;
                            if (!(p = lept_parse_hex4(p, ls))) 
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            if (ls < 0xDC00 || ls > 0xDFFF)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            u = 0x10000 + ((u - 0xD800) << 10) + (ls - 0xDC00);
                        }
                        lept_encode_utf8(ctx, u);
                        break;
                    default: 
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);                  
                }
                break;
            default: 
                if ((unsigned char)ch < 0x20) {
                   STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                }
                PUTC(ctx, ch);
        }
    }

}


int LeptJson::lept_parse_value(lept_context &ctx, lept_value &v)
{
    switch(*ctx.json) {
        case 'n': return lept_parse_literal(ctx, v, "null", LEPT_NULL);
        case 't': return lept_parse_literal(ctx, v, "true", LEPT_TRUE);
        case 'f': return lept_parse_literal(ctx, v, "false", LEPT_FALSE);
        case '"': return lept_parse_string(ctx, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default: return lept_parse_number(ctx, v);
    }
}




int LeptJson::parse(const std::string &json)
{
    lept_context ctx;
    ctx.json = json.c_str();
    ctx.size = ctx.top = 0;
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

LeptJson::LeptJson() 
{ 
    parsed_v_.type = LEPT_NULL; 
}

LeptJson::~LeptJson() 
{ 
    lept_free(parsed_v_);
}


void LeptJson::lept_free(lept_value &v)
{
    switch (v.type) {
        case LEPT_STRING: 
            delete []v.u.s.s; 
            break;
        default: ;
    }
    v.type = LEPT_NULL;
}

void LeptJson::set_string(const char *s, size_t len)
{
    lept_set_string(parsed_v_, s, len);
}

void LeptJson::lept_set_string(lept_value &v, const char *s, size_t len)
{
    assert(s != nullptr || len == 0);
    lept_free(v);
    v.u.s.s = new char[len+1];
    std::memcpy(v.u.s.s, s, len);
    v.u.s.s[len] = '\0';
    v.u.s.len = len;
    v.type = LEPT_STRING;
}


int LeptJson::get_boolean() const
{ 
    assert(parsed_v_.type == LEPT_FALSE || parsed_v_.type == LEPT_TRUE); 
    return parsed_v_.type; 
}

const char* LeptJson::get_string() const
{
    assert(parsed_v_.type == LEPT_STRING);
    return parsed_v_.u.s.s;
}

size_t      LeptJson::get_string_length() const
{
    assert(parsed_v_.type == LEPT_STRING);
    return parsed_v_.u.s.len;
}

void* LeptJson::lept_context::push(size_t count)
{
    void *ret;
    assert(count > 0);
    if (top + count > size) {
        if (size == 0)
            size = LEPT_PARSE_STACK_INIT_SIZE;
        while (size <= (top + count))
            size += size >> 1;
        auto q = stack;
        // here must input a deleter to shared_ptr.reset, so the dynamical memory could be freed later
        stack.reset(new char[size], [](void *p) { delete [](char*)p ;});
        if (q) {
            std::memcpy(stack.get(), q.get(), top);
        }
    }
    ret = stack.get() + top;
    top += count;
    return ret;
}

void* LeptJson::lept_context::pop(size_t count)
{
    assert(top >= count);
    return stack.get() + (top -= count);
}


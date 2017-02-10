#ifndef LEPT_JSON_H__
#define LEPT_JSON_H__

#include <iostream>
#include <string>
#include <memory>
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

struct lept_member;
struct lept_value
{
    union  {
        double num;
        struct { lept_member *m; size_t size; } obj;
        struct { char*       s ; size_t len ; } s;
        struct { lept_value* e ; size_t size; } a;
    } u;
    lept_type type;
};

struct lept_member
{
    char *k = nullptr; size_t klen;
    lept_value v;
};

enum parse_return
{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LEPT_PARSE_MISS_KEY,
    LEPT_PARSE_MISS_COLON,
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

inline size_t            lept_value_get_array_size(const lept_value &v);
inline lept_value*       lept_value_get_array_element(const lept_value &v, size_t index);
inline size_t            lept_value_get_object_size(const lept_value &v);
inline size_t            lept_value_get_object_key_length(const lept_value &v, size_t index);
inline const char*       lept_value_get_object_key(const lept_value &v, size_t index);
inline const lept_value* lept_value_get_object_value(const lept_value &v, size_t index);


class LeptJson
{
  public:
    LeptJson();
    ~LeptJson();
    int parse(const std::string &json);

    void set_type(const lept_type nt)   {  parsed_v_.type = nt; }
    void set_null()                     { parsed_v_.type = LEPT_NULL;}
    void set_boolean(unsigned char b)   { parsed_v_.type = ( b ? LEPT_TRUE : LEPT_FALSE) ;}
    void set_number(double n)           { parsed_v_.type = LEPT_NUMBER; parsed_v_.u.num = n; }
    void set_string(const char *s, size_t len);
    
    lept_type   get_type() const        { return parsed_v_.type; }
    double      get_number() const      { assert(parsed_v_.type == LEPT_NUMBER); return parsed_v_.u.num; }
    int         get_boolean() const;    
    const char* get_string() const;
    size_t      get_string_length() const;
    lept_value* get_array_element(size_t index) const;
    size_t      get_array_size() const                 { return lept_value_get_array_size(parsed_v_); }
    size_t      get_object_size() const                { return lept_value_get_object_size(parsed_v_); }
    size_t      get_object_key_length(size_t id) const { return lept_value_get_object_key_length(parsed_v_, id); }
    const char* get_object_key(size_t id) const        { return lept_value_get_object_key(parsed_v_, id); }
    const lept_value* get_object_value(size_t id) const {return lept_value_get_object_value(parsed_v_, id); }

    void        clear()                 { lept_free(parsed_v_); }

  private:
    struct lept_context {
        const char *json;
        std::shared_ptr<char> stack;
        size_t size, top;

        void* push(size_t count);
        void* pop(size_t count);
    };
    lept_value parsed_v_;

    inline void lept_parse_init() { parsed_v_.type = LEPT_NULL; }
    inline void lept_set_string(lept_value &v, const char *s, size_t len);
    void lept_parse_whitespace(lept_context &ctx);
    int lept_parse_value(lept_context &ctx, lept_value &v);
    int lept_parse_literal(lept_context &ctx, lept_value &v, const std::string &literal, lept_type type);
    int lept_parse_number(lept_context &ctx, lept_value &v);
    int lept_parse_string(lept_context &ctx, lept_value &v);
    int lept_parse_string_raw(lept_context &ctx, char **s, size_t &len);
    int lept_parse_array(lept_context &ctx, lept_value &v);
    int lept_parse_object(lept_context &ctx, lept_value &v);
    const char* lept_parse_hex4(const char *json, unsigned &u);
    void lept_encode_utf8(lept_context &ctx, unsigned u);
    void lept_free(lept_value &v);
};

inline size_t lept_value_get_array_size(const lept_value &v) 
{ 
    assert(v.type == LEPT_ARRAY); 
    return v.u.a.size;
}

inline lept_value* lept_value_get_array_element(const lept_value &v, size_t index)
{
    assert(v.type == LEPT_ARRAY);
    assert(index < v.u.a.size);
    return v.u.a.e + index;
}

inline size_t lept_value_get_object_size(const lept_value &v)
{
    assert(v.type == LEPT_OBJECT);
    return v.u.obj.size;
}

inline size_t lept_value_get_object_key_length(const lept_value &v, size_t index)
{
    assert(v.type == LEPT_OBJECT);
    assert(v.u.obj.size > index);
    return v.u.obj.m[index].klen;
}

inline const char* lept_value_get_object_key(const lept_value &v, size_t index)
{
    assert(v.type == LEPT_OBJECT);
    assert(v.u.obj.size > index);
    return v.u.obj.m[index].k;
}

inline const lept_value* lept_value_get_object_value(const lept_value &v, size_t index)
{
    assert(v.type == LEPT_OBJECT);
    assert(v.u.obj.size > index);
    return &v.u.obj.m[index].v;
}
#endif
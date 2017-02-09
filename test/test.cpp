#include "../source/leptjson.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static unsigned main_ret = 0;
static unsigned test_count = 0;
static unsigned test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)                                                          \
    do                                                                                                            \
    {                                                                                                             \
        test_count++;                                                                                             \
        if (equality)                                                                                             \
            test_pass++;                                                                                          \
        else                                                                                                      \
        {                                                                                                         \
            fprintf(stderr, "%s%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1;                                                                                         \
        }                                                                                                         \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, len) EXPECT_EQ_BASE(strncmp((expect), (actual), len) == 0, expect, actual, "%s")

static void test_parse_null()
{
    LeptJson v;
    v.set_type(
        
        LEPT_TRUE);
    EXPECT_EQ_INT(LEPT_PARSE_OK, v.parse("null"));
    EXPECT_EQ_INT(LEPT_NULL, v.get_type());
}

static void test_parse_true()
{
    LeptJson v;
    v.set_type(LEPT_FALSE);
    EXPECT_EQ_INT(LEPT_PARSE_OK, v.parse("true"));
    EXPECT_EQ_INT(LEPT_TRUE, v.get_type());
}

static void test_parse_false() 
{
    LeptJson v;
    v.set_type(LEPT_TRUE);
    EXPECT_EQ_INT(LEPT_PARSE_OK, v.parse(" false"));
    EXPECT_EQ_INT(LEPT_FALSE, v.get_type());
}

#define TEST_PARSE_ERROR(error, json) \
    do { \
        LeptJson v; \
        v.set_type(LEPT_FALSE); \
        EXPECT_EQ_INT(error, v.parse(json)); \
        EXPECT_EQ_INT(LEPT_NULL, v.get_type()); \
    } while (0)

static void test_parse_invalid_value()
{
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "nul ");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "?");
    //invalid number
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "-1."); // at least one digit after "."
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, ".1");  // at least one digit before "."
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "1e");  // at least one digit after "e/E"
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "1E");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");

    #if 0
    TEST_PARSE_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "");
    #endif
}

static void test_parset_root_not_singular()
{
    TEST_PARSE_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
    TEST_PARSE_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
}

static void test_parse_expect_value() 
{
    TEST_PARSE_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_PARSE_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

#define TEST_NUMBER(expect, json) \
    do { \
        LeptJson v; \
        v.set_type(LEPT_NULL); \
        EXPECT_EQ_INT(LEPT_PARSE_OK, v.parse(json)); \
        EXPECT_EQ_INT(LEPT_NUMBER, v.get_type()); \
        EXPECT_EQ_DOUBLE(expect, v.get_number()); \
    } while (0)

static void test_parse_number()
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(123.4, "123.4");
    TEST_NUMBER(-123.4, "-123.4");
    TEST_NUMBER(3.1415926, "3.1415926");
    TEST_NUMBER(-3.1415926, "-3.1415926");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");    
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E10, "1.234E10");
    TEST_NUMBER(-1.234E10, "-1.234E10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
    //boundary cases
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); // the smallest number > 1
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); //Min subnormal positive double
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009E-308, "2.2250738585072009E-308"); //Max subnormal double
    TEST_NUMBER(-2.2250738585072009E-308, "-2.2250738585072009E-308");
    TEST_NUMBER(2.2250738585072014E-308, "2.2250738585072014E-308"); //Min normal positive double
    TEST_NUMBER(-2.2250738585072014E-308, "-2.2250738585072014E-308");
    TEST_NUMBER(1.797693134862315E308, "1.797693134862315E308"); //max double
    TEST_NUMBER(-1.797693134862315E308, "-1.797693134862315E308");
}
static void test_parse_number_too_big() {
    TEST_PARSE_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_PARSE_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

#define TEST_PARSE_STRING(expect, json) \
    do {\
        LeptJson v; \
        EXPECT_EQ_INT(LEPT_PARSE_OK, v.parse(json)); \
        EXPECT_EQ_INT(LEPT_STRING, v.get_type()); \
        EXPECT_EQ_STRING(expect, v.get_string(), v.get_string_length()); \
    } while (0)
static void test_parse_string()
{
    TEST_PARSE_STRING("", "\"\"");
    TEST_PARSE_STRING("abc", "\"abc\"");
    TEST_PARSE_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_PARSE_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    #if 1
    TEST_PARSE_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_PARSE_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_PARSE_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_PARSE_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_PARSE_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_PARSE_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
    #endif
}

static void test_parse_missing_quotation_mark() {
    TEST_PARSE_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_PARSE_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape()
{
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() 
{
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_surrogate()
{
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_invalid_unicode_hex() 
{
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}     

static void test_access_string()
{
    LeptJson v;
    v.set_string("", 0);
    EXPECT_EQ_STRING("", v.get_string(), v.get_string_length());
    v.set_string("Hello", 5);
    EXPECT_EQ_STRING("Hello", v.get_string(), v.get_string_length());
    
}

static void test_access_boolean()
{
    LeptJson v;
    v.set_boolean(0);
    EXPECT_EQ_INT(LEPT_FALSE, v.get_boolean());
    v.set_boolean(1);
    EXPECT_EQ_INT(LEPT_TRUE, v.get_boolean());  
}

static void test_access_number()
{
    LeptJson v;
    v.set_number(123);
    EXPECT_EQ_DOUBLE(123.0, v.get_number());
    v.set_number(0.2E12);
    EXPECT_EQ_DOUBLE(0.2E12, v.get_number());    
}

static void test_access()
{
    test_access_boolean();
    test_access_number();
    test_access_string();
}

static void test_parse() 
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_number_too_big();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parset_root_not_singular();
    test_access();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed.\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}



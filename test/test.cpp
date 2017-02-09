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


static void test_parse() 
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_number_too_big();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parset_root_not_singular();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed.\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}



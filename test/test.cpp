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

static void test_parse() 
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed.\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}



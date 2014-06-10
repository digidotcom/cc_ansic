#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "connector_api.h"

size_t dp_process_string(char * const string, char * const buffer, size_t const bytes_available, size_t * bytes_used_ptr);

}

TEST_GROUP(connector_test)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(connector_test, testNoSpaces)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "No_spaces_string";
    char expected_string[] = "No_spaces_string";
    char buffer[100];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(string), bytes_copied);
    CHECK_EQUAL(bytes_copied, bytes_processed);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testSpaced)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "s p a c e s";
    char expected_string[] = "\"s p a c e s\"";
    char buffer[100];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(string), bytes_copied);
    CHECK_EQUAL(strlen(expected_string), bytes_processed);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testCommas)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "c,o,m,m,a,s";
    char expected_string[] = "\"c,o,m,m,a,s\"";
    char buffer[100];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(string), bytes_copied);
    CHECK_EQUAL(strlen(expected_string), bytes_processed);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testCarriage)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "C\rA\nR\nR\nI\r \tAbulator";
    char expected_string[] = "\"C\rA\nR\nR\nI\r \tAbulator\"";
    char buffer[100];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(string), bytes_copied);
    CHECK_EQUAL(strlen(expected_string), bytes_processed);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testQuotes)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "\"Q\"u\"o\"t\"e\"s";
    char expected_string[] = "\"\"\"Q\"\"u\"\"o\"\"t\"\"e\"\"s\"";
    char buffer[100];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(string), bytes_copied);
    CHECK_EQUAL(strlen(expected_string), bytes_processed);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testMixed)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "M i,x \" E\nD\r ";
    char expected_string[] = "\"M i,x \"\" E\nD\r \"";
    char buffer[100];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(string), bytes_copied);
    CHECK_EQUAL(strlen(expected_string), bytes_processed);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testNotEnoughSize)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "string";
    char expected_string[] = "str";
    char buffer[4];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(expected_string), bytes_processed);
    CHECK_EQUAL(bytes_copied, bytes_copied);
    STRCMP_EQUAL(expected_string, buffer);
}

TEST(connector_test, testNotEnoughSizeQuoted)
{
    size_t bytes_processed;
    size_t bytes_copied = 0;
    char string[] = "s,tring";
    char first_expected_string[] = "\"s,";
    char second_expected_string[] = "tri";
    char buffer[4];

    bytes_processed = dp_process_string(string, buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(first_expected_string), bytes_processed);
    CHECK_EQUAL(2, bytes_copied);
    STRCMP_EQUAL(first_expected_string, buffer);

    bytes_processed = dp_process_string(&string[bytes_copied], buffer, sizeof buffer, &bytes_copied);
    CHECK_EQUAL(strlen(second_expected_string), bytes_processed);
    CHECK_EQUAL(3, bytes_copied);
    STRCMP_EQUAL(second_expected_string, buffer);
}

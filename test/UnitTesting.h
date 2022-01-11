/*
Copyright 2022 Joonas Salonpää

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <iostream>
#include <string>
#include <vector>

struct UnitTestResult
{
    std::string name;
    std::string failure;
    bool failed;
};

class UnitTest
{
public:
    std::string failure;
    bool failed;
    std::string name;
    bool verboseLogging;
    std::vector<struct UnitTestResult> *unitTestResults;

    UnitTest(const std::string &name,
             std::vector<struct UnitTestResult> *unitTestResults,
             bool verboseLogging)
        : name(name), unitTestResults(unitTestResults), verboseLogging(verboseLogging), failed(false)
    {
    }

    virtual void test() = 0;

    void testEnd()
    {
        struct UnitTestResult result = {
            .name = name,
            .failure = failure,
            .failed = failed,
        };
        unitTestResults->push_back(result);
    }
};

#define XLOG(event)     \
    if (verboseLogging) \
    std::cout << event << '\n'

#define TEST_LOG(event) \
    std::cout << event << '\n';

#define EXPECT(clause)                                  \
    do                                                  \
    {                                                   \
        if (unitTestCtx->verboseLogging)                \
            std::cout << "testing " << #clause << '\n'; \
        bool isTrue = (clause);                         \
        if (!isTrue)                                    \
        {                                               \
            if (unitTestCtx->verboseLogging)            \
                std::cout << "Assert failed!\n";        \
            unitTestCtx->failure = #clause;             \
            unitTestCtx->failed = true;                 \
            return;                                     \
        }                                               \
    } while (0)

#define TEST(name)                                                           \
    void do_test_##name(UnitTest *);                                         \
    class UnitTest_##name : public UnitTest                                  \
    {                                                                        \
    public:                                                                  \
        UnitTest_##name(std::vector<struct UnitTestResult> *u, bool verbose) \
            : UnitTest(#name, u, verbose)                                    \
        {                                                                    \
        }                                                                    \
        void test()                                                          \
        {                                                                    \
            XLOG("* Running test " #name);                                   \
            do_test_##name(this);                                            \
            testEnd();                                                       \
        }                                                                    \
    };                                                                       \
    void do_test_##name(UnitTest *unitTestCtx)

#define TEST_INIT()                              \
    std::vector<UnitTestResult> unitTestResults; \
    std::string filter;                          \
    bool useFilter = false;                      \
    int argScanState = 0;                        \
    bool verboseLogging = false;                 \
    for (int i = 0; i < argc; i++)               \
    {                                            \
        const auto arg = std::string(argv[i]);   \
        if (arg == "--filter")                   \
        {                                        \
            useFilter = true;                    \
            argScanState = 1;                    \
        }                                        \
        else if (argScanState == 1)              \
            filter = arg;                        \
        else if (arg == "--verbose")             \
            verboseLogging = true;               \
        argScanState = 0;                        \
    }                                            \
    XLOG("Running in verbose logging mode");     \
    XLOG("Test filter: " << filter);

#define RUN_TEST(testName)                                       \
    do                                                           \
    {                                                            \
        std::string n(#testName);                                \
        if (useFilter && n.find(filter) != std::string::npos)    \
        {                                                        \
            XLOG("Skipping " #testName);                         \
            break;                                               \
        }                                                        \
        UnitTest_##testName t(&unitTestResults, verboseLogging); \
        t.test();                                                \
    } while (0)

#define TEST_END()                                                      \
    int failedTests = 0;                                                \
    XLOG("---- TEST SUMMARY ----");                                     \
    for (auto r : unitTestResults)                                      \
    {                                                                   \
        std::cout << "Test [" << r.name << "] result: ";                \
        if (r.failed)                                                   \
        {                                                               \
            failedTests++;                                              \
            std::cout << "FAIL, expected to be true: " << r.failure;    \
        }                                                               \
        else                                                            \
            std::cout << "PASS";                                        \
        std::cout << "\n";                                              \
    }                                                                   \
    std::cout << "Tests run " << unitTestResults.size()                 \
              << ", failed tests: " << failedTests << "\n"              \
              << "Test result: " << (failedTests > 0 ? "FAIL" : "PASS") \
              << "\n";                                                  \
    return failedTests

#define TEST_SUITE(name)                                             \
    void testSuite_##name(bool useFilter, const std::string &filter, \
                          bool verboseLogging, std::vector<UnitTestResult> &unitTestResults)

#define RUN_TEST_SUITE(name)                      \
    XLOG("*** Running test suite [" #name "] ***"); \
    testSuite_##name(useFilter, filter, verboseLogging, unitTestResults)

#define NEAR(var, val, delta) (((var) > (val) - (delta)) && ((var) < (val) + (delta)))

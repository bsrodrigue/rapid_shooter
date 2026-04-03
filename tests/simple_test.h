#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

class TestRunner {
public:
    static TestRunner& getInstance() {
        static TestRunner instance;
        return instance;
    }

    void addTest(const std::string& name, std::function<void()> testFunc) {
        tests.push_back({name, testFunc});
    }

    void run() {
        int passedCount = 0;
        for (const auto& test : tests) {
            try {
                test.func();
                std::cout << "[PASS] " << test.name << std::endl;
                passedCount++;
            } catch (const std::exception& e) {
                std::cerr << "[FAIL] " << test.name << ": " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "[FAIL] " << test.name << ": Unknown error" << std::endl;
            }
        }
        std::cout << "Tests passed: " << passedCount << "/" << tests.size() << std::endl;
        if (passedCount != (int)tests.size()) exit(1);
    }

private:
    struct Test {
        std::string name;
        std::function<void()> func;
    };
    std::vector<Test> tests;
};

#define TEST(name) \
    void name(); \
    static int dummy_##name = []() { \
        TestRunner::getInstance().addTest(#name, name); \
        return 0; \
    }(); \
    void name()

#define ASSERT_TRUE(condition) \
    if (!(condition)) throw std::runtime_error("Assertion failed: " #condition)

#define ASSERT_EQ(val1, val2) \
    if ((val1) != (val2)) throw std::runtime_error("Assertion failed: " #val1 " == " #val2)

#endif

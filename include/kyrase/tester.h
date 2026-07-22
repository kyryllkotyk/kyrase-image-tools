#ifndef TESTER_H_
#define TESTER_H_

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <stdexcept>

class Tester {
public:
    void add_test(
        const std::string& test_name,
        const std::function<void()>& test,
        bool expected_throw
    );

    bool run_tests() const;

private:
    bool run_test(
        const std::string& test_name,
        const std::function<void()>& test,
        bool expected_throw
    ) const;

    struct TestCase {
        std::string name;
        std::function<void()> test;
        bool expected_throw;
    };

    std::vector<TestCase> tests;
    
};

#endif
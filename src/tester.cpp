#include "kyrase/tester.h"

void Tester::add_test(
    const std::string& test_name,
    const std::function<void()>& test,
    bool expected_throw
) {
    tests.push_back({ test_name, test, expected_throw });
}

bool Tester::run_tests() const {
    bool all_passed = true;

    for (const TestCase& test_case : tests) {
        all_passed &= run_test(
            test_case.name,
            test_case.test,
            test_case.expected_throw
        );
    }

    if (all_passed) {
        std::cout << "\nAll tests passed\n";
    }
    else {
        std::cout << "\nSome tests failed\n";
    }

    return all_passed;
}

bool Tester::run_test(
    const std::string& test_name,
    const std::function<void()>& test,
    bool expected_throw
) const {
    try {
        test();

        if (expected_throw) {
            std::cout << "FAILED: " << test_name << " did not throw\n";
            return false;
        }

        std::cout << "PASSED: " << test_name << " did not throw\n";
        return true;
    }
    catch (const std::invalid_argument& e) {
        if (expected_throw) {
            std::cout << "PASSED: " << test_name << " threw: " << e.what() << '\n';
            return true;
        }

        std::cout << "FAILED: " << test_name << " threw unexpectedly: " << e.what() << '\n';
        return false;
    }
    catch (const std::exception& e) {
        if (expected_throw) {
            std::cout << "PASSED: " << test_name << " threw: " << e.what() << '\n';
            return true;
        }

        std::cout << "FAILED: " << test_name << " threw unexpectedly: " << e.what() << '\n';
        return false;
    }
    catch (...) {
        std::cout << "FAILED: " << test_name << " threw unknown exception type\n";
        return false;
    }
}
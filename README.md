# testlib
Small c++ library for unit tests.

## Example

Code

```c++
#include "testlib.hpp"

int main() {
    testlib::init();
    testlib::test("test1", []() {
        check(foo == 0);
        check_exception(foo(), std::invalid_argument);
        testlib::cleanup();
    });
    return 0;
}
```

Possible output

```
TEST: test1 ............................................. OK [231 ms] 

No errors found.
```

## Functions

### init()

Initializes the test library; on Windows, it enables colors in the console.

### cleanup()

Prints the number of errors.

### test(test_name, test_function)

Invokes the test function, measures the test duration, prints the relevant diagnostics.

## Macros

### check(cond)

Checks the given condition; if it fails, it registers an error; prints the relevant error.

### check_exception(expression, exception_type)

Invokes the given expression, and checks if it throws the given exception type; if it does not throw the given exception type, then it registers an error.


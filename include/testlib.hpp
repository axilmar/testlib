#ifndef TESTLIB_TESTLIB_HPP
#define TESTLIB_TESTLIB_HPP


#include <string>
#include <stdexcept>
#include <chrono>
#include <iostream>


#ifdef _WIN32
#include <Windows.h>
#endif


#define check(cond) {\
    if (!(cond)) {\
        ++testlib::test_error_count;\
        throw testlib::test_error(__FILE__, __LINE__, #cond);\
    }\
}


#define check_enum_string(e, v)\
    check(##e##_to_string(e::v) == #v);\
    check(##e##_from_string(#v) == e::v);


#define check_exception(expr, ex)\
    do {\
        try {\
            expr;\
        }\
        catch (const ex&) {\
            break;\
        }\
        ++testlib::test_error_count;\
        throw testlib::test_error(__FILE__, __LINE__, #expr);\
    } while (0);


namespace testlib {


    class test_error : public std::runtime_error {
    public:
        test_error(const char* file, int line, const std::string& msg) : std::runtime_error(msg), m_file(file), m_line(line) {}

        const char* file() const { return m_file; }
        int line() const { return m_line; }

    private:
        const char* m_file;
        int m_line;
    };


    inline size_t test_error_count = 0;


    inline void init() {
        #ifdef _WIN32
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(console_handle, &mode);
        SetConsoleMode(console_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        #endif
    }


    inline void cleanup() {
        std::cout << '\n';
        if (test_error_count == 0) {
            std::cout << "No errors found.\n";
        }
        else {
            std::cout << "Found " << test_error_count << (test_error_count > 1 ? " errors.\n" : " error.\n");
        }
        std::cout << '\n';
    }


    //get duration string
    template <class T> std::string duration_string(const T& start, const T& end) {
        return " \u001b[34m[" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms]\u001b[0m";
    }


    //execute test
    template <class F> void test(const char* name, F&& proc) {
        static const auto dots = [](const std::string& s) {
            const size_t base = 50;
            const size_t count = s.size() < base ? base - s.size() : 5;
            return " \u001b[36m" + std::string(count - 2, '.') + "\u001b[0m ";
        };

        std::string title = std::string("TEST: ") + name;
        std::cout << "\u001b[36m" << "TEST: " << "\u001b[37;1m" << name << "\u001b[0m";

        const auto start = std::chrono::high_resolution_clock::now();

        try {
            proc();
        }
        catch (const test_error& ex) {
            const auto end = std::chrono::high_resolution_clock::now();
            std::cout << dots(title) << "\u001b[31mERROR\u001b[0m" << duration_string(start, end) << std::endl;
            std::cout << "    " << "\u001b[33m" << "File " << ex.file() << ", line " << ex.line() << ":" << "\u001b[0m" << std::endl;
            std::cout << "    " << "\u001b[33m" << ex.what() << "\u001b[0m" << std::endl;
            return;
        }

        const auto end = std::chrono::high_resolution_clock::now();
        std::cout << dots(title) << "\u001b[32mOK\u001b[0m" << duration_string(start, end) << std::endl;
    }


} //namespace testlib


#endif //TESTLIB_TESTLIB_HPP

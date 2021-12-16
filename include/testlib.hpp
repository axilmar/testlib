#ifndef TESTLIB_TESTLIB_HPP
#define TESTLIB_TESTLIB_HPP


#include <string>
#include <stdexcept>
#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
#include <deque>
#include <utility>
#include <iomanip>


#ifdef _WIN32
#include <Windows.h>
#endif


#define check(cond) {\
    if (!(cond)) {\
        ++testlib::get_globals().test_error_count;\
        std::lock_guard lock(testlib::get_globals().test_error_queue_mutex);\
        testlib::get_globals().test_error_queue.emplace_back(__FILE__, __LINE__, #cond);\
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
        ++testlib::get_globals().test_error_count;\
        std::lock_guard lock(testlib::get_globals().test_error_queue_mutex);\
        testlib::get_globals().test_error_queue.emplace_back(__FILE__, __LINE__, #expr);\
    } while (0);


#define fail_test_with_exception(ex) {\
        ++testlib::get_globals().test_error_count;\
        std::lock_guard lock(testlib::get_globals().test_error_queue_mutex);\
        testlib::get_globals().test_error_queue.emplace_back(__FILE__, __LINE__, std::string("\u001b[33;1mException:\u001b[0m\u001b[33m ") + ex.what());\
    }


namespace testlib {


    struct globals {
        size_t test_row_length = 80;
        std::atomic<size_t> test_error_count = 0;
        std::mutex test_error_queue_mutex;
        std::deque<std::tuple<const char*, int, std::string>> test_error_queue;
    };


    inline globals& get_globals() {
        static globals g;
        return g;
    }


    inline void init(size_t test_row_length_param = 80) {
        #ifdef _WIN32
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(console_handle, &mode);
        SetConsoleMode(console_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        #endif
        test_row_length_param = get_globals().test_row_length;
        get_globals().test_error_count = 0;
    }


    inline void cleanup() {
        std::cout << '\n';
        if (get_globals().test_error_count == 0) {
            std::cout << "No errors found.\n";
        }
        else {
            std::cout << "Found " << get_globals().test_error_count << (get_globals().test_error_count > 1 ? " errors.\n" : " error.\n");
        }
        std::cout << '\n';
    }


    //get duration string
    template <class T> std::string duration_string(const T& start, const T& end) {
        return " \u001b[34;1m[" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms]\u001b[0m";
    }


    //execute test
    template <class F> void test(const char* name, F&& proc) {
        static const auto dots = [](const std::string& s) {
            const size_t count = s.size() < get_globals().test_row_length ? get_globals().test_row_length - s.size() : 5;
            return " \u001b[36m" + std::string(count - 2, '.') + "\u001b[0m ";
        };

        std::string title = std::string("TEST: ") + name;
        std::cout << "\u001b[36m" << "TEST: " << "\u001b[37;1m" << name << "\u001b[0m";

        const auto start = std::chrono::high_resolution_clock::now();

        try {
            proc();
        }
        catch (const std::exception& ex) {
            const auto end = std::chrono::high_resolution_clock::now();
            std::cout << dots(title) << "\u001b[31;1mEXCEPTION\u001b[0m" << duration_string(start, end) << std::endl;
            std::cout << "    " << "\u001b[33m" << ex.what() << "\u001b[0m" << std::endl;
            return;
        }

        const auto end = std::chrono::high_resolution_clock::now();

        std::lock_guard lock(get_globals().test_error_queue_mutex);

        if (get_globals().test_error_queue.empty()) {
            std::cout << dots(title) << "\u001b[32;1mOK\u001b[0m" << duration_string(start, end) << std::endl;
        }

        else {
            std::cout << dots(title) << "\u001b[31;1m" << (get_globals().test_error_queue.size() > 1 ? "ERRORS" : "ERROR") << "\u001b[0m" << duration_string(start, end) << std::endl;
            size_t index = 1;
            for (const auto& error : get_globals().test_error_queue) {
                std::cout << std::setw(3) << index << ") \u001b[33m" << "File " << std::get<0>(error) << ", line " << std::get<1>(error) << ":" << "\u001b[0m" << std::endl;
                std::cout << "     " << "\u001b[33m" << std::get<2>(error) << "\u001b[0m" << std::endl;
                ++index;
            }
            get_globals().test_error_queue.clear();
        }
    }


    //execute test with std string as name.
    template <class F> void test(const std::string& name, F&& proc) {
        test(name.c_str(), std::forward<F>(proc));
    }


} //namespace testlib


#endif //TESTLIB_TESTLIB_HPP

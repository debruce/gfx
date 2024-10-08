#pragma once

#include <cxxabi.h>
#include <memory>
#include <typeinfo>

template <typename T>
std::string demangle(T&&) {
    auto name = typeid(T).name();
    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}
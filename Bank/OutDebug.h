#pragma once

#include "iostream"

#ifndef NDEBUG
    #define OUTDEBUG(Out) do { \
       std::cerr << __FILE__ << ":" << __LINE__ \
                 << " " << Out << std::endl; \
    } while(0)
#else
    #define OUTDEBUG(Out) do {}while(0)
#endif
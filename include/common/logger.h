#pragma once
#include <iostream>

#define LOG_INFO(...) \
    do { \
        std::cout << "[INFO] " << __VA_ARGS__ << std::endl; \
    } while (0)

#define LOG_WARN(...) \
    do { \
        std::cout << "[WARN] " << __VA_ARGS__ << std::endl; \
    } while (0)

#define LOG_ERROR(...) \
    do { \
        std::cerr << "[ERROR] " << __VA_ARGS__ << std::endl; \
    } while (0)

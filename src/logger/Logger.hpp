#pragma once

#include <cstdarg>

namespace Logger {
    void log(const char *fmt, ...);
    void log(const char *fmt, va_list args);
};

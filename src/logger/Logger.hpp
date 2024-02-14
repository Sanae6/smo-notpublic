#pragma once

#include <cstdarg>
#include <lib/diag/abort.hpp>

namespace Logger {
    void log(const char *fmt, ...);
    void log(const char *fmt, va_list args);
};

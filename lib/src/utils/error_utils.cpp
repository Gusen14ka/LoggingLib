#include <string.h>
#include <string>

#include "error_utils.hpp"

std::string strerror_safe(const int err)
{
    char buffer[256];
#if defined(__GLIBC__) && defined(_GNU_SOURCE)
    return strerror_r(err, buffer, sizeof(buffer));
#else
    int result = strerror_r(err, buffer, sizeof(buffer));

    if (result != 0)
        return "Unknown error";

    return buffer;
#endif
}

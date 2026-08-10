#include "util.h"

std::string utilFunction() {
#ifdef UTIL_FILE_DEFINE
    return "UTIL_FILE_DEFINE is active.";
#else
    return "utilFunction() default response.";
#endif
}

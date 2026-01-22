#include "logger.h"

#ifdef __ANDROID__
#include <android/log.h>
void Logger::info(const char* msg) {
    __android_log_print(ANDROID_LOG_INFO, "XVDriver", "%s", msg);
}
#else
#include <iostream>
void Logger::info(const char* msg) {
    std::cout << "[XVDriver] " << msg << std::endl;
}
#endif
#include "logger.h"
#include <fstream>

extern "C" void xv_log(const char* message) {
    std::ofstream log_file("C:\\Users\\mr795\\OneDrive\\Documentos\\Projeto\\xvdriver\\bin\\xv_log.txt", std::ios::app);
    if (log_file.is_open()) {
        log_file << "[XV] " << message << std::endl;
        log_file.flush();
        log_file.close();
    }
}

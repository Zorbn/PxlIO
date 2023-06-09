#pragma once

#include <iostream>
#include <string>

#define RUNTIME_ERROR(message)                                                                                         \
    std::cout << "Error in " << std::string(__FILE__).substr(SOURCE_PATH_SIZE) << "@" << __LINE__ << ": " << message   \
              << "\n";                                                                                                 \
    exit(1)
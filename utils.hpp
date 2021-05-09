// utility functions
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "common.h"

// modify this to display (or not display) warnings
const bool ALLOW_WARNING = false;

void printWarning(string msg)
{
    if (ALLOW_WARNING)
        std::cout << "WARNING: " << msg << std::endl;
}

void printError(string msg)
{
    std::cout << msg << std::endl;
    exit(0);
}

// uniformly random integer from [mn,mx]
int randomInt(int mn, int mx)
{
    std::uniform_int_distribution<int> dist(mn, mx);
    static std::mt19937 gen(time(0));
    return dist(gen);
}

#endif
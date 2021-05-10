// utility functions
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "common.h"

// modify this to display (or not display) warnings
const bool ALLOW_WARNING = false;
// modify this to display (or not display) debug info
const bool ALLOW_DEBUG = false;

void printWarning(string msg)
{
    if (ALLOW_WARNING)
        std::cout << "WARNING: " << msg << std::endl;
}

void printDebug(string msg)
{
    if (ALLOW_DEBUG)
        std::cout << "DEBUG: " << msg << std::endl;
}

void printError(string msg)
{
    std::cout << "ERROR: " << msg << std::endl;
    exit(0);
}

// uniformly random integer from [mn,mx]
int randomInt(int mn, int mx)
{
    std::uniform_int_distribution<int> dist(mn, mx);
    static std::mt19937 gen(time(0));
    return dist(gen);
}

// use prefix sum
//  to calculate 2D sum in O(1)
template <class T>
class PrefixSum
{
private:
    vector<vector<T>> s;

public:
    // initialize prefix sum
    void initSum(const vector<vector<T>> w)
    {
        int r = w.size();
        int c = w.front().size();
        s = vector<vector<T>>(r + 1,
                              vector<T>(c + 1, 0));
        for (int i = 1; i <= r; ++i)
            for (int j = 1; j <= c; ++j)
                s[i][j] = w[i - 1][j - 1] +
                          s[i - 1][j] + s[i][j - 1] -
                          s[i - 1][j - 1];
    }

    // up left down right
    // including boundary
    T getSum(int u, int l,
             int d, int r)
    {
        // actual l=l+1 ...
        return s[d + 1][r + 1] + s[u][l] -
               s[d + 1][l] - s[u][r + 1];
    }
};

#endif
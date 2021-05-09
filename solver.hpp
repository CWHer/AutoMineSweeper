#include "common.h"
#include "utils.hpp"

class Solver
{
private:
    // special status of board
    static const int MINE = 16;
    static const int FLAG = 32;
    static const int UNKNOWN = 64;

    bool is_empty;
    int mine_number;
    int width, height;
    vector<vector<int>> board;
    vector<Block> next_steps; // click these
    vector<Block> next_flags; // put flags on these

    // if a block has at least one unknown nearby
    //  then it can be one of the frontier
    // true frontier also requires board[i][j]<=8
    vector<vector<bool>> is_frontier;

    // detect whether there are some blocks
    //  whose capacity equals to # of mines nearby
    // their unknown neighbors are safe (if there's any)
    void detectSafe()
    {
        PrefixSum<int> s;
        vector<vector<int>> w(height,
                              vector<int>(width, 0));
        int stat = MINE | FLAG;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                w[i][j] = !!(board[i][j] & stat);
        s.initSum(w);

        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] <= 8 && is_frontier[i][j])
                {
                    int cnt = s.getSum(max(i - 1, 0),
                                       max(j - 1, 0),
                                       min(i + 1, height - 1),
                                       min(j + 1, width - 1));
                    if (cnt > board[i][j])
                        printWarning("detectSafe() Overflow on (" +
                                     std::to_string(i) + "," +
                                     std::to_string(j) + ")");
                    if (cnt == board[i][j])
                        for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                            for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                                if (board[x][y] == UNKNOWN)
                                    next_steps.emplace_back(make_pair(x, y));
                }
        if (!next_steps.empty())
        {
            std::sort(next_steps.begin(),
                      next_steps.end());
            next_steps.resize(std::unique(next_steps.begin(),
                                          next_steps.end()) -
                              next_steps.begin());
        }
    }

    // detect whether there are some blocks
    //  whose capacity equals to # of unknown blocks + mines
    // their unknown neighbors must be mines
    void detectUnsafe()
    {
        PrefixSum<int> s;
        vector<vector<int>> w(height,
                              vector<int>(width, 0));
        int stat = MINE | UNKNOWN | FLAG;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                w[i][j] = !!(board[i][j] & stat);
        s.initSum(w);

        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] <= 8 && is_frontier[i][j])
                {
                    // int cnt = 0;
                    // for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                    //     for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                    //         cnt += !!(board[x][y] & stat);
                    int cnt = s.getSum(max(i - 1, 0),
                                       max(j - 1, 0),
                                       min(i + 1, height - 1),
                                       min(j + 1, width - 1));
                    if (cnt == board[i][j])
                        for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                            for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                                if (board[x][y] == UNKNOWN)
                                    next_flags.emplace_back(make_pair(x, y));
                }
        if (!next_flags.empty())
        {
            std::sort(next_flags.begin(),
                      next_flags.end());
            next_flags.resize(std::unique(next_flags.begin(),
                                          next_flags.end()) -
                              next_flags.begin());
        }
    }

    // randomly choose next position
    void randomNext()
    {
        printWarning("Random Step!");
        vector<Block> points;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] == UNKNOWN)
                    points.emplace_back(make_pair(i, j));
        if (points.empty())
            printError("No Unknown Blocks But Not Terminate");
        std::shuffle(points.begin(), points.end(),
                     std::mt19937(time(0)));
        next_steps.emplace_back(points.front());
    }

public:
    // input types
    // 0-8: # of mines nearby
    // 16: unknown
    // 32: flag
    // 64: unknown
    void readBoard(string file_name = "board.txt")
    {
        ifstream fin(file_name);
        if (!fin.is_open())
            printError("File Not Found!");
        fin >> height >> width >> mine_number;
        board = vector<vector<int>>(height,
                                    vector<int>(width, 0));
        is_frontier = vector<vector<bool>>(height,
                                           vector<bool>(width, false));
        is_empty = true;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
            {
                fin >> board[i][j];
                if (board[i][j] == UNKNOWN)
                {
                    for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                        for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                            is_frontier[x][y] = true;
                }
                else
                    is_empty = false;
            }
        fin.close();
    }

    // print next steps
    void printNextStep(string file_name = "steps.txt")
    {
        ofstream fout(file_name);
        if (!fout.is_open())
            printError("Can't Open Output File!");
        for (const auto &it : next_steps)
            fout << it.first << ' ' << it.second << '\n';
        fout.close();
    }

    // print flags
    void printNextFlag(string file_name = "flags.txt")
    {
        ofstream fout(file_name);
        if (!fout.is_open())
            printError("Can't Open Output File!");
        for (const auto &it : next_flags)
            fout << it.first << ' ' << it.second << '\n';
        fout.close();
    }

    // calculate next steps
    void solve()
    {
        next_steps.clear();
        next_flags.clear();
        // initial click
        if (is_empty)
        {
            next_steps.emplace_back(make_pair(
                randomInt(0, height - 1), randomInt(0, width - 1)));
            return;
        }

        // detect blocks with sufficient existing mines
        // their remaining must be safe
        detectSafe();
        if (!next_steps.empty())
            return;

        // detect blocks with sufficient mines + unknown
        // their remaining must be unsafe
        detectUnsafe();
        if (!next_flags.empty())
            return;

        randomNext();
    }
};
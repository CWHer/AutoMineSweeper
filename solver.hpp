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

    // (1) for known blocks
    // if it has at least one unknown nearby
    //  then it can be one of the borders
    // true border block also requires board[i][j]<=8
    // (2) for unknown blocks
    // if it has at least one known nearby
    //  then it is one of the borders

    // whether it has unknown nearby
    vector<vector<bool>> has_unknown;
    // whether it has known(not includeing flag) nearby
    vector<vector<bool>> has_known;

    // store independent partitions of borders
    vector<vector<Block>> border_partition;

private:
    inline bool inBoard(int x, int y)
    {
        return x >= 0 && x < height &&
               y >= 0 && y < width;
    }

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
                if (board[i][j] <= 8 && has_unknown[i][j])
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
        int stat = MINE | FLAG | UNKNOWN;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                w[i][j] = !!(board[i][j] & stat);
        s.initSum(w);

        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] <= 8 && has_unknown[i][j])
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

    // traverse connected borders
    void dfsPartition(int cur_x, int cur_y,
                      vector<vector<bool>> &is_border)
    {
        static const int DIR = 4; // U R D L
        static const int dx[DIR] = {-1, 0, 1, 0};
        static const int dy[DIR] = {0, 1, 0, -1};

        is_border[cur_x][cur_y] = false;
        border_partition.back()
            .emplace_back(make_pair(cur_x, cur_y));

        for (int i = 0; i < DIR; ++i)
        {
            int x = cur_x + dx[i];
            int y = cur_y + dy[i];
            if (!inBoard(x, y))
                continue;
            if (is_border[x][y])
                dfsPartition(x, y, is_border);
        }
    }

    // divide border into independent set
    void divideBorder()
    {
        border_partition.clear();
        // whether an unknown block is border
        //  and is set to false after visited
        auto is_border = vector<vector<bool>>(height,
                                              vector<bool>(width, false));
        // all unknown border blocks
        vector<Block> border_blocks;
        // find all border blocks
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (has_known[i][j] &&
                    board[i][j] == UNKNOWN)
                {
                    is_border[i][j] = true;
                    border_blocks.emplace_back(make_pair(i, j));
                }
        // partition connected border
        for (const auto &it : border_blocks)
            if (is_border[it.first][it.second])
            {
                border_partition.emplace_back(vector<Block>());
                dfsPartition(it.first, it.second, is_border);
            }

        {
            printDebug("The Number of Border Partition is " +
                       std::to_string(border_partition.size()));
            int mx = 0;
            for (const auto &it : border_partition)
                mx = max(mx, (int)it.size());
            printDebug("With Max Size " + std::to_string(mx));
        }
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
        has_unknown = vector<vector<bool>>(height,
                                           vector<bool>(width, false));
        has_known = vector<vector<bool>>(height,
                                         vector<bool>(width, false));
        is_empty = true;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
            {
                fin >> board[i][j];
                is_empty &= board[i][j] == UNKNOWN;
                auto &has_what = board[i][j] == UNKNOWN ? has_unknown
                                                        : has_known;
                for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                    for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                        if (board[i][j] != FLAG)
                            has_what[x][y] = true;
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

        divideBorder();

        randomNext();
    }
};
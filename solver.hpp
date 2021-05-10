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
    vector<vector<bool>> has_unknown; // whether it has unknown nearby
    vector<vector<bool>> has_known;   // whether it has known(flag) nearby

    // ------> auxiliary arrays for border partition
    // whether a border block is head (clockwise)
    // 1 H
    // 1 x
    vector<vector<bool>> head_border;
    // whether an unknown block is border
    vector<vector<bool>> is_border;
    // whether a border block is visited
    vector<vector<bool>> vis_border;
    // <------ auxiliary arrays for border partition

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

    // get next clockwise border block
    Block getClockwiseNext(int cur_x, int cur_y)
    {
        // to ensure clockwise traverse
        //  next border must have a 1~8 | FLAG block
        //  on the right hand (both Case 0 and Case 1)
        static const int DIR = 4; // U R D L
        static const int dx[DIR] = {-1, 0, 1, 0};
        static const int dy[DIR] = {0, 1, 0, -1};
        int stat = 0xf | FLAG;
        // Case 0           Case 1
        // x B x x x        B C N   don't go back
        // x B B B x        x x B
        // x C N B x        x x B
        // x B x x x
        for (int i = 0; i < DIR; ++i)
        {
            int x = cur_x + dx[i];
            int y = cur_y + dy[i];
            int x0 = x + dx[(i + 1) % DIR];
            int y0 = y + dy[(i + 1) % DIR];
            int x1 = x0 + dx[(i + 2) % DIR];
            int y1 = y0 + dy[(i + 2) % DIR];

            if (!inBoard(x, y) ||
                !is_border[x][y] ||
                !inBoard(x0, y0))
                continue;
            if (!!(board[x0][y0] & stat) ||
                (board[x0][y0] == UNKNOWN &&
                 !!(board[x1][y1] & stat)))
                return make_pair(x, y);
        }
        return make_pair(-1, -1);
    }

    // clockwise traverse from cur
    void dfsLink(int cur_x, int cur_y)
    {
        vis_border[cur_x][cur_y] = true;

        auto nxt = getClockwiseNext(cur_x,
                                    cur_y);
        int x = nxt.first;
        int y = nxt.second;
        if (x == -1 && y == -1)
            return;
        if (!vis_border[x][y])
            dfsLink(x, y);
        head_border[x][y] = false;
    }

    // traverse from head
    void dfsPartition(int cur_x, int cur_y)
    {
        vis_border[cur_x][cur_y] = false;
        border_partition.back()
            .emplace_back(make_pair(cur_x, cur_y));

        auto nxt = getClockwiseNext(cur_x,
                                    cur_y);
        int x = nxt.first;
        int y = nxt.second;
        if (x == -1 && y == -1)
            return;
        if (vis_border[x][y])
            dfsPartition(x, y);
    }

    // divide border into independent set
    void divideBorder()
    {
        border_partition.clear();
        head_border = vector<vector<bool>>(height,
                                           vector<bool>(width, true));
        is_border = vector<vector<bool>>(height,
                                         vector<bool>(width, false));
        vis_border = vector<vector<bool>>(height,
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
        // link all border clockwise
        for (const auto &it : border_blocks)
            if (!vis_border[it.first][it.second])
                dfsLink(it.first, it.second);
        // collect chains of border
        for (const auto &it : border_blocks)
            if (head_border[it.first][it.second])
            {
                border_partition.emplace_back(vector<Block>());
                dfsPartition(it.first, it.second);
            }
        // collect circles of border
        for (const auto &it : border_blocks)
            if (vis_border[it.first][it.second])
            {
                border_partition.emplace_back(vector<Block>());
                dfsPartition(it.first, it.second);
            }
        printDebug("The Number of Border Partition is " +
                   std::to_string(border_partition.size()));
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
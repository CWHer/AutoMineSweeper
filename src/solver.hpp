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
    int known_cnt;  // current known
    int mine_cnt;   // current mine
    int total_mine; // total mine
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
    // whether it has known(not including flag) nearby
    vector<vector<bool>> has_known;

    int border_sum; // # of border blocks
    // store independent partitions of borders
    vector<vector<Block>> border_partition;
    vector<Block> not_border; // unknown but not border
    // [i][j]: # of solutions with border[i] and j mines in total
    vector<vector<long long>> border_cnt;
    // [i][j][k]: # of solutions with border[i],
    //  j th block has a mine and k mines in total
    vector<vector<vector<long long>>> border_block_cnt;

    // probability of having MINE
    vector<vector<double>> mine_prob;

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

    // find next block with minimum mine_prob
    void randomNext()
    {
        double min_prob = 1;
        static const double eps = 1e-8;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] == UNKNOWN)
                    min_prob = min(min_prob,
                                   mine_prob[i][j]);

        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] == UNKNOWN &&
                    std::fabs(min_prob -
                              mine_prob[i][j]) < eps)
                {
                    next_steps.emplace_back(
                        make_pair(i, j));
                    printDebug("Choose a Block with Probability " +
                               std::to_string(min_prob));
                    return;
                }

        // printWarning("Random Step!");
        // vector<Block> points;
        // for (int i = 0; i < height; ++i)
        //     for (int j = 0; j < width; ++j)
        //         if (board[i][j] == UNKNOWN)
        //             points.emplace_back(make_pair(i, j));
        // if (points.empty())
        //     printError("No Unknown Blocks But Not Terminate");
        // std::shuffle(points.begin(), points.end(),
        //              std::mt19937(time(0)));
        // next_steps.emplace_back(points.front());
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
                if (board[i][j] == UNKNOWN)
                {
                    if (has_known[i][j])
                    {
                        is_border[i][j] = true;
                        border_blocks.emplace_back(
                            make_pair(i, j));
                    }
                    else
                        not_border.emplace_back(
                            make_pair(i, j));
                }
        // partition connected border
        border_sum = border_blocks.size();
        for (const auto &it : border_blocks)
            if (is_border[it.first][it.second])
            {
                border_partition.emplace_back(vector<Block>());
                dfsPartition(it.first, it.second, is_border);
                std::sort(border_partition.back().begin(),
                          border_partition.back().end());
            }
        // may find an empty block or mine quickly
        std::shuffle(border_partition.begin(),
                     border_partition.end(),
                     std::mt19937(time(0)));
        // debug info
        {
            printDebug("The Number of Border Partition is " +
                       std::to_string(border_partition.size()));
            int max_size = 0;
            for (const auto &it : border_partition)
                max_size = max(max_size, (int)it.size());
            printDebug("With Max Size " + std::to_string(max_size));
        }
    }

    // MINE + FLAG should <= known info
    // MINE + FLAG + UNKNOWN should >= known info
    bool isFeasible(int cur_x, int cur_y)
    {
        int stat1 = MINE | FLAG;
        int stat2 = stat1 | UNKNOWN;
        for (int x = max(cur_x - 1, 0); x <= min(cur_x + 1, height - 1); ++x)
            for (int y = max(cur_y - 1, 0); y <= min(cur_y + 1, width - 1); ++y)
                if (board[x][y] <= 8 && board[x][y] > 0)
                {
                    int cnt1 = 0, cnt2 = 0;
                    for (int i = max(x - 1, 0); i <= min(x + 1, height - 1); ++i)
                        for (int j = max(y - 1, 0); j <= min(y + 1, width - 1); ++j)
                        {
                            cnt1 += !!(board[i][j] & stat1);
                            cnt2 += !!(board[i][j] & stat2);
                        }
                    if (cnt1 > board[x][y] ||
                        cnt2 < board[x][y])
                        return false;
                }
        return true;
    }

    // use dfs to search all feasible solutions
    //  in each border partition
    // border_partition[idx][k]
    void dfsBorderMines(int idx, int k,
                        int cur_mine)
    {
        if (k == border_partition[idx].size())
        {
            for (int i = 0; i < k; ++i)
            {
                auto it = border_partition[idx][i];
                if (board[it.first][it.second] == MINE)
                    border_block_cnt[idx][i][cur_mine]++;
            }
            border_cnt[idx][cur_mine]++;
            return;
        }

        Block cur = border_partition[idx][k];
        int x = cur.first;
        int y = cur.second;

        board[x][y] = MINE;
        if (isFeasible(x, y)) // feasible prune
            dfsBorderMines(idx, k + 1, cur_mine + 1);

        board[x][y] = 0;
        if (isFeasible(x, y)) // feasible prune
            dfsBorderMines(idx, k + 1, cur_mine);

        board[x][y] = UNKNOWN;
    }

    // calculate probability inside each partition
    //  we assume each partition is independent here
    void calcLocalProb()
    {
        static const double eps = 1e-8;
        double min_prob = 1;
        double max_prob = 0;

        int unknown_mine = total_mine - mine_cnt;
        double avg_prob = (unknown_mine)*1.0 /
                          (width * height - known_cnt - mine_cnt); // not precise
        mine_prob = vector<vector<double>>(height,
                                           vector<double>(width, avg_prob));

        border_block_cnt.clear();
        border_cnt.clear();

        // use dfs to find all feasible solutions
        //  and calculate probability inside each partition
        for (int i = 0; i < border_partition.size(); ++i)
        {
            int cur_size = border_partition[i].size();
            border_cnt.emplace_back(vector<long long>(cur_size + 1, 0));
            border_block_cnt.emplace_back(
                vector<vector<long long>>(cur_size,
                                          vector<long long>(cur_size + 1, 0)));

            dfsBorderMines(i, 0, 0);

            for (int j = 0; j < cur_size; ++j)
            {
                double prob = 0, total = 0;
                // # of mine in this partition
                for (int k = 0; k < cur_size + 1; ++k)
                {
                    prob += border_block_cnt.back()[j][k];
                    total += border_cnt.back()[k];
                }
                prob /= total;
                min_prob = min(min_prob, prob);
                max_prob = max(max_prob, prob);
                auto it = border_partition[i][j];
                mine_prob[it.first][it.second] = prob;
            }
            // find empty block or mine
            //  return here to avoid further dfs
            if (min_prob < eps ||
                std::fabs(1 - max_prob) < eps)
            {
                calcProbFinish();
                return;
            }
        }
    }

    // calculate conditional probability
    //  partitions are not mutually independent
    void calcGlobalProb()
    {
        static const double eps = 1e-8;
        static const int border_threshold = 20;
        double min_prob = 1;
        double max_prob = 0;
        int unknown_mine = total_mine - mine_cnt;
        // if we have few remaining border,
        //  it is not accurate to assume each partition is independent
        // then, we can use dynamic programing to calculate solutions
        //  combining different partitions
        // moreover, here I assume that when # of unknown blocks
        //  but not border <= threshold, then it is few enough
        //  to consider dependency
        if (not_border.size() > border_threshold)
            return;
        // calculate solutions in not_border
        if (!not_border.empty())
        {
            border_partition.emplace_back(not_border);
            int cur_size = not_border.size();
            border_cnt.emplace_back(vector<long long>(cur_size + 1, 0));
            border_block_cnt.emplace_back(
                vector<vector<long long>>(cur_size,
                                          vector<long long>(cur_size + 1, 0)));

            dfsBorderMines(border_partition.size() - 1,
                           0, 0);
        }
        // calculate conditional probability
        for (int i = 0; i < border_partition.size(); ++i)
        {
            // f is for dynamic programing
            // f[i][j]: # of solutions with first i th block
            //  and j mines in total
            vector<long long> f(unknown_mine + 1, 0);
            f[0] = 1;

            for (int T = 0; T < border_partition.size(); ++T)
                if (i != T)
                    for (int j = unknown_mine; j >= 0; --j)
                    {
                        long long cnt = 0;
                        for (int k = 0; k <= j; ++k)
                            if (j - k < border_cnt[T].size())
                                cnt += f[k] * border_cnt[T][j - k];
                        f[j] = cnt;
                    }

            int cur_size = border_partition[i].size();
            for (int j = 0; j < cur_size; ++j)
            {
                double prob = 0, total = 0;
                // when this partition has k mines
                for (int k = 0; k <= min(unknown_mine, cur_size); ++k)
                    if (f[unknown_mine - k] != 0)
                    {
                        prob += border_block_cnt[i][j][k] *
                                f[unknown_mine - k];
                        total += border_cnt[i][k] *
                                 f[unknown_mine - k];
                    }
                prob /= total;
                min_prob = min(min_prob, prob);
                max_prob = max(max_prob, prob);
                auto it = border_partition[i][j];
                mine_prob[it.first][it.second] = prob;
            }
        }
        // try to find some
        calcProbFinish();
    }

    // calculate probability of having MINE
    //  on each border block
    void calcBorderProb()
    {
        divideBorder();
        if (border_partition.empty())
            printWarning("No Border!");

        calcLocalProb();
        if (!next_steps.empty() ||
            !next_flags.empty())
            return;

        calcGlobalProb();
    }

    // find some empty block or mine
    void calcProbFinish()
    {
        static const double eps = 1e-8;

        for (const auto &part : border_partition)
            for (const auto &it : part)
            {
                double prob = mine_prob[it.first][it.second];
                if (prob < eps) // must not be mine
                    next_steps.push_back(it);
                if (std::fabs(1 - prob) < eps) // must be mine
                    next_flags.push_back(it);
            }
        if (!next_steps.empty())
            printDebug("Find Empty Blocks");
        if (!next_flags.empty())
            printDebug("Find Mines");
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
        fin >> height >> width >> total_mine;
        board = vector<vector<int>>(height,
                                    vector<int>(width, 0));
        has_unknown = vector<vector<bool>>(height,
                                           vector<bool>(width, false));
        has_known = vector<vector<bool>>(height,
                                         vector<bool>(width, false));
        is_empty = true;
        known_cnt = mine_cnt = 0;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
            {
                int cur_type;
                fin >> cur_type;
                is_empty &= cur_type == UNKNOWN;
                mine_cnt += cur_type == FLAG;
                known_cnt += cur_type <= 8;
                auto &has_what = cur_type == UNKNOWN ? has_unknown
                                                     : has_known;
                for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                    for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                        if (cur_type != FLAG)
                            has_what[x][y] = true;
                board[i][j] = cur_type;
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

        // detect blocks with sufficient mines + unknown
        // their remaining must be unsafe
        detectUnsafe();
        if (!next_steps.empty() ||
            !next_flags.empty())
            return;

        // calculate probability of having MINE on border
        calcBorderProb();

        if (next_steps.empty() &&
            next_flags.empty())
            randomNext();
    }
};
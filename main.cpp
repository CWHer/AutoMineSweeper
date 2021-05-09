#include "common.h"
#include "solver.hpp"

class Designer
{
private:
    // special status of board
    static const int MINE = 16;
    static const int FLAG = 32;
    static const int UNKNOWN = 64;

    int mine_number;
    int width, height;
    vector<vector<int>> board;
    vector<vector<bool>> vis;

    // generate mines
    //  excluding first click
    void genMines(Pos first_click)
    {
        // generate mines
        vector<Pos> mines;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (i != first_click.first &&
                    j != first_click.second)
                    mines.emplace_back(make_pair(i, j));
        std::shuffle(mines.begin(), mines.end(),
                     std::mt19937(time(0)));
        mines.resize(mine_number);
        for (const auto &mine : mines)
            board[mine.first][mine.second] = MINE;
        // update board
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                if (board[i][j] == 0)
                {
                    int cnt = 0;
                    for (int x = max(i - 1, 0); x <= min(i + 1, height - 1); ++x)
                        for (int y = max(j - 1, 0); y <= min(j + 1, width - 1); ++y)
                            cnt += board[x][y] == MINE;
                    board[i][j] = cnt;
                }
    }

    // visit all neighbors of '0'
    void dfsVisit(Pos cur)
    {
        int x = cur.first, y = cur.second;
        for (int i = max(x - 1, 0); i <= min(x + 1, height - 1); ++i)
            for (int j = max(y - 1, 0); j <= min(y + 1, width - 1); ++j)
                if (!vis[i][j])
                {
                    vis[i][j] = true;
                    if (board[i][j] == 0)
                        dfsVisit(make_pair(i, j));
                }
    }

public:
    Designer(int width = 30, int height = 16,
             int mine_number = 99)
    {
        this->width = width;
        this->height = height;
        this->mine_number = mine_number;
    }

    // clear and initialize board
    void initBoard()
    {
        // clear
        vis.clear();
        vis = vector<vector<bool>>(height,
                                   vector<bool>(width, 0));
        board.clear();
        board = vector<vector<int>>(height,
                                    vector<int>(width, 0));
    }

    // print board
    void printBoard(string file_name = "board.txt")
    {
        ofstream fout(file_name);
        if (!fout.is_open())
            printError("Can't Open Output File!");
        fout << height << ' ' << width << ' '
             << mine_number << std::endl;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                fout.width(3);
                fout << (vis[i][j] ||
                                 board[i][j] == FLAG
                             ? board[i][j]
                             : UNKNOWN)
                     << ' ';
            }
            fout << std::endl;
        }
        fout.close();
    }

    // click points
    //  return true if encounter mines
    // moreover, if it is the first click,
    //  then generate mines.
    bool clickBoard(bool is_first,
                    string file_name = "steps.txt")
    {
        ifstream fin(file_name);
        if (!fin.is_open())
            printError("File Not Found!");

        vector<Pos> points;
        int x, y;
        while (fin >> x >> y)
            points.emplace_back(make_pair(x, y));
        if (points.empty())
            printWarning("No Next Step!");

        if (is_first)
            genMines(points.front());
        for (const auto &it : points)
            if (!vis[it.first][it.second])
            {
                vis[it.first][it.second] = true;
                if (board[it.first][it.second] == 0)
                    dfsVisit(it);
                if (board[it.first][it.second] == MINE)
                    return true;
            }
            else // warning may be caused by dfsVisit
                printWarning("Click Existing Block(" +
                             std::to_string(it.first) + "," +
                             std::to_string(it.second) + ")");
        return false;
    }

    // put flags
    void putFlag(string file_name = "flags.txt")
    {
        ifstream fin(file_name);
        if (!fin.is_open())
            printError("File Not Found!");

        vector<Pos> points;
        int x, y;
        while (fin >> x >> y)
            points.emplace_back(make_pair(x, y));
        if (points.empty())
            printWarning("No Next Flag!");

        for (const auto &it : points)
            if (!vis[it.first][it.second])
                board[it.first][it.second] = FLAG;
            else
                printWarning("Put Flag on Existing Block(" +
                             std::to_string(it.first) + "," +
                             std::to_string(it.second) + ")");
    }

    // return true if all mines are found
    bool isFinished()
    {
        int cnt = 0;
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j)
                cnt += vis[i][j];
        return cnt == height * width - mine_number;
    }
};

Solver solver;
Designer designer;
int main()
{
    int T = 1000, win_cnt = 0;
    for (int i = 1; i <= T; ++i)
    {
        bool is_first = true;
        designer.initBoard();
        while (!designer.isFinished())
        {
            designer.printBoard();
            solver.readBoard();
            solver.solve();
            solver.printNextStep();
            solver.printNextFlag();
            if (designer.clickBoard(is_first)) // boom!
            {
                win_cnt--; // offset cnt++
                break;
            }
            designer.putFlag();
            is_first = false;
        }
        win_cnt++;
        std::cout << '\r' << win_cnt << '/' << i << ' ';
        std::cout << std::setprecision(4)
                  << win_cnt * 1.0 / i << std::flush;
    }
    return 0;
}
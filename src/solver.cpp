#include "solver.hpp"

Solver solver;
int main()
{
    std::ios::sync_with_stdio(false);
    solver.readBoard();
    solver.solve();
    solver.printNextStep();
    solver.printNextFlag();
    return 0;
}
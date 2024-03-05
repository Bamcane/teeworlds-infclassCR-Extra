// The preceding version of this code was implemented in Python.
// Such an approach was initially adopted by myself, only to realize thereafter...
// TEEWORLDS IS A GAME ARCHITECTED IN C++!!! It is most certainly not founded upon Python!
// Consequently, I solicited the assistance of ChatGLM to transpose my codebase into the C++ paradigm.
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <set>
#include <cfloat>

using namespace std;

// 定义一个结构体来表示网格中的每个单元格
struct Cell
{
    int parent_i, parent_j;
    double f, g, h;
};

// 检查给定的单元格是否可通行
bool isValid(int row, int col, vector<vector<int>> &grid)
{
    return (row >= 0) && (row < grid.size()) && (col >= 0) && (col < grid[0].size()) && (grid[row][col] == 1);
}

// 计算H值，这里使用曼哈顿距离
double heuristic(int row, int col, int endRow, int endCol)
{
    return abs(row - endRow) + abs(col - endCol);
}

// 找到最短路径
void aStar(vector<vector<int>> &grid, pair<int, int> start, pair<int, int> end)
{
    int rows = grid.size();
    int cols = grid[0].size();

    vector<vector<Cell>> cellDetails(rows, vector<Cell>(cols));

    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            cellDetails[i][j].f = FLT_MAX;
            cellDetails[i][j].g = FLT_MAX;
            cellDetails[i][j].h = FLT_MAX;
            cellDetails[i][j].parent_i = -1;
            cellDetails[i][j].parent_j = -1;
        }
    }

    i = start.first, j = start.second;
    cellDetails[i][j].f = 0.0;
    cellDetails[i][j].g = 0.0;
    cellDetails[i][j].h = 0.0;
    cellDetails[i][j].parent_i = i;
    cellDetails[i][j].parent_j = j;

    set<pair<double, pair<int, int>>> openList;
    openList.insert(make_pair(0.0, make_pair(i, j)));

    bool foundDest = false;

    while (!openList.empty())
    {
        pair<double, pair<int, int>> p = *openList.begin();
        openList.erase(openList.begin());

        i = p.second.first;
        j = p.second.second;
        double gNew, hNew, fNew;

        if (i == end.first && j == end.second)
        {
            foundDest = true;
            break;
        }

        // 检查所有相邻的单元格
        for (int di = -1; di <= 1; di++)
        {
            for (int dj = -1; dj <= 1; dj++)
            {
                if (di == 0 && dj == 0)
                    continue;

                int ni = i + di;
                int nj = j + dj;

                if (isValid(ni, nj, grid))
                {
                    gNew = cellDetails[i][j].g + 1.0;
                    hNew = heuristic(ni, nj, end.first, end.second);
                    fNew = gNew + hNew;

                    if (cellDetails[ni][nj].f == FLT_MAX || cellDetails[ni][nj].f > fNew)
                    {
                        openList.insert(make_pair(fNew, make_pair(ni, nj)));

                        cellDetails[ni][nj].f = fNew;
                        cellDetails[ni][nj].g = gNew;
                        cellDetails[ni][nj].h = hNew;
                        cellDetails[ni][nj].parent_i = i;
                        cellDetails[ni][nj].parent_j = j;
                    }
                }
            }
        }
    }

    // 如果找到路径，则回溯以找到路径
    if (foundDest)
    {
        cout << "路径找到:" << endl;
        vector<pair<int, int>> path;
        int row = end.first;
        int col = end.second;

        path.push_back(make_pair(row, col));
        while (!(cellDetails[row][col].parent_i == row && cellDetails[row][col].parent_j == col))
        {
            path.push_back(make_pair(cellDetails[row][col].parent_i, cellDetails[row][col].parent_j));
            int temp_row = cellDetails[row][col].parent_i;
            int temp_col = cellDetails[row][col].parent_j;
            row = temp_row;
            col = temp_col;
        }

        // 打印路径
        for (int i = path.size() - 1; i >= 0; i--)
        {
            cout << "(" << path[i].first << ", " << path[i].second << ")" << endl;
        }
    }
    else
    {
        cout << "没有找到路径" << endl;
    }
}

int main()
{
    vector<vector<int>> grid{
        {1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

    pair<int, int> start{0, 0};
    pair<int, int> end{2, 2};

    aStar(grid, start, end);

    return 0;
}

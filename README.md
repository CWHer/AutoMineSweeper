# AutoMineSweeper

自动扫雷机

`Python`前端和`C++`后端

安装完依赖，保证截图正确后，运行`main.py`即可

（默认截图策略适用于Win10/1920x1080/100%缩放）

在Win10下正确率一般，大概25%~30%

![](assets/fig-succ.gif)



### 版本记录

- [x] Ver0.1

  完成自己的扫雷交互程序`Designer`

  

- [x] Ver0.2

  初步完成扫雷程序`Solver`

  目前只有`detectSafe`，`detectUnsafe`和`randomNext`

  大概有3/1000的胜率

  

- [x] Ver0.3

  提高正确率

  ~~参考资料3的推断也是一个比较好的idea，但是实现起来比较复杂，而且不能处理除了边界外的未知位置~~

  在这里选择了概率的方法，分块枚举边界

  现在大概有25%的正确率

  

- [x] Ver0.4

  继续提高正确率

  在剩余块较少时，将不是边界的块也进行考虑，并计算当前剩余地雷数下的条件概率

  现在大概有33%的正确率

  

- [ ] Ver0.5（x

  有雷的概率不一定是胜率



- [x] Ver0.6

  完成python前端

  使用图片RGB求和后的值作为判据

  ~~虽然觉得不靠谱，但是相当的快~~

  

- [x] Ver1.0

  完成前后端接口
  
  前端是`main.py`，可以调用后端接口`solver.exe`
  
  Win10下正确率一般，大概25%~30%



- [ ] Ver1.1

  优化数字识别的鲁棒性



### 后端

#### 默认生成文件

- `board.txt`

  存放地图。第一行给出宽、高和地雷的总数，接下来给出地图

  1~8对应普通格子，16表示地雷，32表示旗子，64表示未知

  ```text
  64 1 0
  64 3 2
  64 32 32
  ```

- `steps.txt`

  接下来要点击的格子

- `flags.txt`

  接下来要放的旗子
  
  

#### 主要类

- `Designer`类

  位于`main.cpp`

  交互程序，随机生成地图并测试

- `Solver`类

  位于`solver.hpp`，求解器

  部分函数

  - `divideBlock`

    ~~按照顺时针分解边界（没有实现，见`counter_example`）~~

    本来打算用DP来计数，类似[这个](https://www.luogu.com.cn/problem/P2327)

    但是边界不一定是单链，可能是树，或者更糟糕。就算是链也不一定是直的

    最后还是按照连通块划分，然后枚举了

    ![](assets/fig-partition.png)

  - `dfsBorderMines`

    枚举边界的方案数，并统计每个点有地雷的方案数

    可行性剪枝：MINE+FLAG <= NUMBER <= MINE+FLAG+UNKNOWN
  
    
  
  使用以下方式求解
  
  1. `detectSafe`
  
     如果一个格子周围的地雷数和自身相等，则未知的全是安全的
  
     使用前缀和优化求和
  
  2. `detectUnsafe`
  
     如果一个格子周围未知的加上现有的地雷数和自身相等，则未知的全是地雷
  
     使用前缀和优化求和
  
  3. `calcBorderProb` Case 0
  
     将边界按照连通性分块，块内枚举所有可能方案，用古典概型的方法求概率
  
     如果有概率为0/1的直接输出
  
  4. `calcBorderProb` Case 1
  
     当剩余的未知块不多时，块之间并非独立
  
     ![](assets/fig-few.png)
  
     如果以上情况只剩余一个地雷，则只有一种解
  
     记$f[i]$为除了该块以外，地雷数为$i$的方案数，用类似背包的DP即可
  
     $border[i]$为该块地雷数为$i$的方案数
  
     $block[i][j]$为这块中第$i$个有地雷且地雷总数为$j$的方案数
  
     $total$为剩余地雷的总数
     $$
     P[k]=\frac{\sum_{i=0} block[k][i]\times f[total-i]}{\sum_{i=0}border[i]\times f[total-i]}
     $$
     $P[k]$为已知剩余$total$的情况下，$k$中有地雷的概率
  
  5. `randomNext`
  
     选择一个有地雷概率最小的，除了边界外的未知位置取平均值
  
  

#### 代码文件

- `main.cpp`

  用自己写的扫雷程序来测试正确率
  
- `solver.cpp`

  前后端接口

  调用一次`Solver()`读取当前地图并计算



### 前端

#### 主要类

- `Board`

  保存地图状态

  截取图片后用基于hash值的精确匹配来判断类型

  ~~鲁棒性极差，但很快~~

  此外，每次截图后只识别地图上未知的位置
  
  

#### 代码文件

- `utils.py`

  含有`Win()`和`Lose()`两种异常来表示输赢

  `getFlag()`读取旗子并在`Board`上更新

  `clickBoard()`实际点击扫雷窗口

  `newGame()`按F2开始新游戏

- `main.py`

  前端，运行这个文件



### 参考资料

1. [_最强扫雷AI算法详解+源码分享_](https://zhuanlan.zhihu.com/p/136791369)
2. [_How to Write your own Minesweeper AI_](https://luckytoilet.wordpress.com/2012/12/23/2125/)
3. [_CS50 Introduction to Artificial Intelligence with Python_](https://cs50.harvard.edu/ai/2020/projects/1/minesweeper/)
4. [_I created a PERFECT minesweeper AI_](https://www.youtube.com/watch?v=cGUHehFGqBc)


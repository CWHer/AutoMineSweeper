import os

from board import Board
from settings import *
from utils import *

win_cnt = 0
for i in range(10):
    board = Board()
    try:
        while True:
            board.grabBoardImage()
            board.scanBoard()
            board.printBoard()
            os.system("solver.exe")
            board.setBlocks(getFlag(), FLAG)
            clickBoard()

    except Exception as e:
        if not isinstance(e, Win) and \
                not isinstance(e, Lose):
            printError("Terminate in Main() with {}"
                       .format(str(e)))
        win_cnt += 1 if isinstance(e, Win) else 0
        newGame()

    print("\r{}/{} {:.2%}".format(win_cnt, i+1,
                                  win_cnt/(i+1)), end="")

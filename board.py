from os import name
from PIL import Image
from PIL import ImageGrab
import numpy as np
from numpy.core.shape_base import block

from settings import *
from utils import *


# store board
class Board():
    def __init__(self):
        self.board = [[UNKNOWN for _ in range(BOARD_WIDTH)]
                      for _ in range(BOARD_HEIGHT)]
        self.board_img = None

    # clear board
    def clear(self):
        self.board = [[UNKNOWN for _ in range(BOARD_WIDTH)]
                      for _ in range(BOARD_HEIGHT)]

    # set board[i][j]=value
    def setBlocks(self, points, value):
        for x, y in points:
            self.board[x][y] = value

    # grab image of board
    def grabBoardImage(self):
        try:
            handle = gui.FindWindow(None, "Minesweeper")
            left, up, right, down = gui.GetWindowRect(handle)

            # can not set foreground when debuging
            # gui.SetForegroundWindow(handle)

            positions = (left + LEFT_EDGE, up + UP_EDGE,
                         right, down)
            self.board_img = ImageGrab.grab(positions)

            # debug
            self.board_img.save("cur.png")

        except Exception as e:
            printError("Terminate in Grab Image with {}"
                       .format(str(e)))

    # scan a single block
    # a simple but not so robust algorithm
    def __scanBlock(self, i, j, block_img):
        status = {934: UNKNOWN, 772: UNKNOWN,
                  987: 0, 157: 1, 691: 2, 301: 3, 1120: 4,
                  853: 5, 766: 6, 864: 7, 9999: 8,
                  41: ROUND_FAIL, 627: ROUND_FAIL}
        # ROUND_FAIL is mine

        # 1201 is a prime number
        block_sum = np.sum(block_img) % 1201

        if not block_sum in status:
            block_img.show()
            printError(
                "Can't Recognize this Block! with block_sum={}"
                .format(block_sum))

        result = status[block_sum]
        if result == ROUND_FAIL:
            raise Lose()
        # if there is no unknown block then we win
        # if (result == ROUND_WIN
        #         or result == FLAG):
        #     raise Win()
        self.board[i][j] = result

    # print board to text file
    def printBoard(self,
                   file_name="board.txt"):
        with open(file_name, "w") as f:
            f.write("{} {} {}\n".format(
                BOARD_HEIGHT, BOARD_WIDTH, TOTAL_MINE))
            for i in range(BOARD_HEIGHT):
                for j in range(BOARD_WIDTH):
                    f.write("{:>4d}".format(self.board[i][j]))
                f.write("\n")

    # scan every unknown block of board
    def scanBoard(self):
        has_unknow = False
        for i in range(BOARD_HEIGHT):
            for j in range(BOARD_WIDTH):
                if self.board[i][j] == UNKNOWN:
                    x, y = j * BLOCK_SIZE, i * BLOCK_SIZE
                    block_img = self.board_img.crop((x, y,
                                                     x + BLOCK_SIZE, y + BLOCK_SIZE))
                    # block_img.show()
                    self.__scanBlock(i, j, block_img)
                    has_unknow = True
        if not has_unknow:
            raise Win()


# #%% test
if __name__ == "__main__":
    f = Board()
    # f.grabBoardImage()
    f.board_img = Image.open("cur.png")
    f.scanBoard()
    f.printBoard()

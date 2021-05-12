import win32gui as gui
import win32con as con
import win32api as api
import time
import sys

from settings import *


# use exception to indicate results
class Win(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)


class Lose(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)


def printWarning(msg):
    if ALLOW_WARNING:
        print("WARNING: {}".format(msg))


def printDebug(msg):
    if ALLOW_DEBUG:
        print("DEBUG: {}".format(msg))


def printError(msg):
    print("Error: {}".format(msg))
    sys.exit(0)


# get flag position from "flags.txt"
# note that we do not actually put flags on actual board
# instead, we put flags on Board()
def getFlag(file_name="flags.txt"):
    flags = []

    with open(file_name, "r") as f:
        lines = f.readlines()
        for line in lines:
            flags.append(list(
                map(int, line.split())))

    return flags


# click blocks in actual board
def clickBoard(file_name="steps.txt"):
    steps = []

    with open(file_name, "r") as f:
        lines = f.readlines()
        for line in lines:
            steps.append(list(
                map(int, line.split())))

    handle = gui.FindWindow(None, "Minesweeper")
    left, up, _, _ = gui.GetWindowRect(handle)

    # can not set foreground when debuging
    # use a click to do so
    # gui.SetForegroundWindow(handle)
    api.SetCursorPos((left+CLICK_OFFSET,
                      up+CLICK_OFFSET))
    api.mouse_event(con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    api.mouse_event(con.MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)

    for i, j in steps:
        x = left + LEFT_EDGE + \
            j * BLOCK_SIZE + CLICK_OFFSET
        y = up + UP_EDGE + \
            i * BLOCK_SIZE + CLICK_OFFSET
        api.SetCursorPos((x, y))
        api.mouse_event(con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
        api.mouse_event(con.MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
        api.SetCursorPos((left, up))


# press F2 and start new game
def newGame():
    handle = gui.FindWindow(None, "Minesweeper")
    left, up, _, _ = gui.GetWindowRect(handle)

    # can not set foreground when debuging
    # gui.SetForegroundWindow(handle)
    api.SetCursorPos((left+CLICK_OFFSET,
                      up+CLICK_OFFSET))
    api.mouse_event(con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    api.mouse_event(con.MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)

    F2_KEYBOARD = 113
    api.keybd_event(F2_KEYBOARD, 0, 0, 0)
    api.keybd_event(F2_KEYBOARD, 0,
                    con.KEYEVENTF_KEYUP, 0)


# test
if __name__ == "__name__":
    clickBoard()

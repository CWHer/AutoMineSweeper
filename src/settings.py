# size of board
BOARD_WIDTH = 30
BOARD_HEIGHT = 16

# # of mines
TOTAL_MINE = 99

# display (or not display) debug info
ALLOW_DEBUG = True
ALLOW_WARNING = True

# special status of board
#   consistent with notations in Solver()
MINE = 16
FLAG = 32
UNKNOWN = 64
# special status of round
ROUND_FAIL = 128
ROUND_WIN = 256

# NOTICE!!!
# the following settings are suitable
#   on 1920x1080 100% scale windows 10
# use these on your risk

# size of edge
LEFT_EDGE = 15
UP_EDGE = 101
# size of block
BLOCK_SIZE = 16
# click offset from block edge
CLICK_OFFSET = 5

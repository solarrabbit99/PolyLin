import random

MAX_RADIUS = 20
MAX_OBJECT_SIZE = 100
MAX_TIME_DELTA = 50

def randInterval(time: int, width: int):
  a = time - random.randint(0, width)
  b = max(a + 1, time + random.randint(0, width))
  return (a, b)
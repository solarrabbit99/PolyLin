import random

MAX_RADIUS = 1000
MAX_OBJECT_SIZE = 100
MAX_TIME_DELTA = 10

def randInterval(time: int, width: int):
  a = time - random.randint(0, width)
  b = max(a + 1, time + random.randint(0, width))
  return (a, b)
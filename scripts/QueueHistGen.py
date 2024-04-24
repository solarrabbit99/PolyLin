#!/usr/bin/python3

import argparse
from collections import deque
from typing import TextIO
from commons import MAX_RADIUS, MAX_OBJECT_SIZE, MAX_TIME_DELTA, randInterval
import random

def toOpStr(proc: int, method: int, value: int, start: int, end: int):
  switcher = ['enq', 'peek', 'deq']
  methodStr = switcher[method]
  return ' '.join([str(proc), methodStr, str(value), str(start), str(end)]) + '\n'

def genLinHist(size: int, outFile: TextIO):
  queue = deque()
  time = 0
  while size:
    size -= 1
    time += random.randint(1, MAX_TIME_DELTA)
    a, b = randInterval(time, MAX_RADIUS)
    method = random.randint(0, 2)

    if len(queue) == MAX_OBJECT_SIZE:
      method = random.randint(1, 2)
    elif not len(queue):
      method = 0

    if method == 0:
      queue.append(time)
    outFile.write(toOpStr(time, method, queue[0 if method else -1], a, b))
    if method == 2:
      queue.popleft()

def main():
  parser = argparse.ArgumentParser('QueueHistGen')
  parser.add_argument('-l', action='store_true', help='whether the generated history should be linearizable')
  parser.add_argument('output', help='target file path')
  parser.add_argument('n', help='number of operations')
  args = parser.parse_args()
  isLin = args.l
  outPath = args.output
  size = int(args.n)
  with open(outPath, 'w') as f:
    f.write('# queue\n')
    if isLin:
      genLinHist(size, f)

if __name__ == '__main__':
  main()
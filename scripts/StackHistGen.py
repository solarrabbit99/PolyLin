#!/usr/bin/python3

import argparse
from collections import deque
from typing import TextIO
from commons import MAX_RADIUS, MAX_OBJECT_SIZE, MAX_TIME_DELTA, randInterval
import random

def toOpStr(proc: int, method: int, value: int, start: int, end: int):
  switcher = ['push', 'peek', 'pop']
  methodStr = switcher[method]
  return ' '.join([str(proc), methodStr, str(value), str(start), str(end)]) + '\n'

def genLinHist(size: int, outFile: TextIO):
  stack = deque()
  time = 0
  while size:
    size -= 1
    time += random.randint(1, MAX_TIME_DELTA)
    a, b = randInterval(time, MAX_RADIUS)
    method = random.randint(0, 2)

    if len(stack) == MAX_OBJECT_SIZE:
      method = random.randint(1, 2)
    elif not len(stack):
      method = 0

    if method == 0:
      stack.append(time)
    outFile.write(toOpStr(time, method, stack[-1], a, b))
    if method == 2:
      stack.pop()

def genNonLinHist(size: int, outFile: TextIO):
  if size < 4:
    value = 0
    while size:
      size -= 1
      value += 1
      a, b = randInterval(0, MAX_RADIUS)
      method = random.randint(1, 2)
      outFile.write(toOpStr(0, method, value, a, b))
  else:
    outFile.write(toOpStr(0, 0, -2, 0, 1))
    outFile.write(toOpStr(0, 0, -1, 1, 2))
    outFile.write(toOpStr(0, 2, -2, 2, 3))
    outFile.write(toOpStr(0, 2, -1, 3, 4))
    genLinHist(size - 4, outFile)

def main():
  parser = argparse.ArgumentParser('StackHistGen')
  parser.add_argument('-l', action='store_true', help='whether the generated history should be linearizable')
  parser.add_argument('output', help='target file path')
  parser.add_argument('n', help='number of operations')
  args = parser.parse_args()
  isLin = args.l
  outPath = args.output
  size = int(args.n)
  with open(outPath, 'w') as f:
    f.write('# stack\n')
    if isLin:
      genLinHist(size, f)
    else:
      genNonLinHist(size, f)

if __name__ == '__main__':
  main()
#!/usr/bin/python3

import argparse

parser = argparse.ArgumentParser('HistoryFormatter')
parser.add_argument('src_file')
parser.add_argument('dest_file')
args = parser.parse_args()

srcFile = args.src_file
dstFile = args.dest_file

class Operation:
  def __init__(self) -> None:
    self.method = ''
    self.value = 0
    self.invTime = 0
    self.resTime = 0
  
  def __str__(self) -> str:
    return ' '.join([self.method, str(self.value), str(self.invTime), str(self.resTime)])

with open(srcFile, 'r') as src, open(dstFile, 'w') as dst:
  dst.write('# deque\n')
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    tokens = line.split()
    
    prop = Operation()
    switcher = {
      '>': 'push_front',
      '<': 'push_back',
      '[': 'pop_front',
      ']': 'pop_back'
    }
    prop.method = switcher[tokens[0]]
    prop.value = int(tokens[1])
    if not prop.value: continue
    prop.invTime = int(tokens[2])
    prop.resTime = int(tokens[4])
    dst.write(str(prop) + '\n')
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
  dst.write('# stack\n')
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    tokens = line.split()
    
    prop = Operation()
    prop.method = 'push' if tokens[0] == '+' else 'pop'
    prop.value = int(tokens[1])
    if not prop.value: prop.value = -1
    prop.invTime = int(tokens[2])
    prop.resTime = int(tokens[4])
    dst.write(str(prop) + '\n')
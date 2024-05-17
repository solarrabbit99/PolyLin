#!/usr/bin/python3

import argparse
from typing import Dict

parser = argparse.ArgumentParser('HistoryFormatter')
parser.add_argument('src_file')
parser.add_argument('dest_file')
args = parser.parse_args()

srcFile = args.src_file
dstFile = args.dest_file

class Operation:
  def __init__(self) -> None:
    self.proc = 0
    self.method = ''
    self.value = 0
    self.invTime = 0
    self.resTime = 0
  
  def __str__(self) -> str:
    return ' '.join([str(self.proc), self.method, str(self.value), str(self.invTime), str(self.resTime)])

with open(srcFile, 'r') as src, open(dstFile, 'w') as dst:
  procLast : Dict[int, Operation] = {}
  time = 0
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    tokens = line.split()
    
    proc = int(tokens[0][1:-1])

    if proc in procLast:
      prop = procLast[proc]
      del procLast[proc]

      prop.resTime = time
      
      if len(tokens) == 3 and tokens[2] != 'empty':
        prop.value = int(tokens[2])

      dst.write(str(prop) + '\n')

    else:
      prop = Operation()
      prop.proc = proc
      prop.invTime = time
      prop.method = tokens[2].split('(')[0]

      if tokens[2].find('(') != -1:
        prop.value = int(tokens[2].split('(')[1][:-1])

      procLast[proc] = prop

    time += 1
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
    self.proc = 0
    self.method = ''
    self.value = 0
    self.invTime = 0
    self.resTime = 0
  
  def __str__(self) -> str:
    return ' '.join([str(self.proc), self.method, str(self.value), str(self.invTime), str(self.resTime)])

with open(srcFile, 'r') as src, open(dstFile, 'w') as dst:
  dst.write('# queue\n')
  proc = 0
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    tokens = line.split()
    
    prop = Operation()
    prop.proc = proc
    proc += 1
    prop.method = 'enq' if tokens[0] == '+' else 'deq'
    prop.value = int(tokens[1])
    if not prop.value: prop.value = -1
    prop.invTime = int(tokens[2])
    prop.resTime = int(tokens[4])
    dst.write(str(prop) + '\n')
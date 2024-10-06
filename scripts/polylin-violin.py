#!/usr/bin/python3

import yaml
import os
dir_path = os.path.dirname(__file__)

config = None

with open(dir_path + "/common.yml") as file:
  config = yaml.safe_load(file)

import argparse

parser = argparse.ArgumentParser('polylin-violin')
parser.add_argument('src_file')
parser.add_argument('dest_file')
args = parser.parse_args()

srcFile = args.src_file
dstFile = args.dest_file

with open(srcFile, 'r') as src, open(dstFile, 'w') as dst:
  obj = src.readline().strip()[2:]
  dst.write(f'# @object atomic-{obj}\n')
  mapping = config['violin'][obj]
  returnables = config['violin']['return']
  events = []
  lineNo = 1
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    methodPre, value, start_time, end_time = line.split()
    method = mapping[methodPre]
    if method in returnables:
      events.append((int(start_time), f'[{lineNo}] call {method}'))
      events.append((int(end_time), f'[{lineNo}] return {value}'))
    else:
      events.append((int(start_time), f'[{lineNo}] call {method}({value})'))
      events.append((int(end_time), f'[{lineNo}] return'))
    lineNo += 1
  events.sort()
  for event in events:
    dst.write(event[1] + '\n')
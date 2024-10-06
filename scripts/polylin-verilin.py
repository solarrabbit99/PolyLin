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
  mapping = config['verilin'][obj]
  lineNo = 1
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    methodPre, value, start_time, end_time = line.split()
    method = mapping[methodPre]
    isSuccess = 'true' if value != '-1' else 'false'
    dst.write(f'{start_time} {end_time} {lineNo} {method} {value} {isSuccess}\n')
    lineNo += 1
#!/usr/bin/python3

import yaml
import os 
dir_path = os.path.dirname(__file__)

config = None

with open(dir_path + "/common.yml") as file:
  config = yaml.safe_load(file)

import argparse

parser = argparse.ArgumentParser('scal-polylin')
parser.add_argument('src_file')
parser.add_argument('dest_file')
parser.add_argument('impl')
args = parser.parse_args()

srcFile = args.src_file
dstFile = args.dest_file
obj = config['impl'][args.impl]

mapping = config['mappings'][obj]

with open(srcFile, 'r') as src, open(dstFile, 'w') as dst:
  dst.write(f'# {obj}\n')
  while True:
    line = src.readline()
    if not line: break
    if line[0] == '#': continue
    tokens = line.split()
    dst.write(' '.join([mapping[tokens[0]], tokens[1] if tokens[1] else -1, tokens[2], tokens[4]]) + '\n')
#!/usr/bin/python3

import argparse
import os
import subprocess
import time

temp_filename = 'temp.log'
temp_filename_2 = 'temp2.log'

def get_tests(filename: str):
  with open(filename, 'r') as test_file:
    return [line.split(' ') for line in test_file.readlines()]

def generate_history(num_prod: int, num_cons: int, num_oper: int, exec_path: str):
  num_val = int(num_oper/2/num_prod)
  mem_total = num_val * 8
  mem_padded = (int(mem_total/4096) + 1) * 409600
  subprocess.Popen(f'{exec_path} -producers={num_prod} -consumers={num_cons} -operations={num_val} -log_operations=1 -print_summary=0 -shuffle_threads=1 -prealloc_size={mem_padded} > {temp_filename}', shell=True).wait()

def format_history(formatter_path: str):
  subprocess.Popen(f'{formatter_path} {temp_filename} {temp_filename_2}', shell=True).wait()

def run_test():
  start_time = time.time()
  results = subprocess.Popen(f'../build/engine {temp_filename_2}', shell=True, stdout=subprocess.PIPE).stdout.read()  
  return (int(results.decode()), time.time() - start_time)

def main():
  parser = argparse.ArgumentParser('runtest', './runtest.py <config> <scal> <format>', 'script for running test suite')
  parser.add_argument('config', help='provide test configuration file')
  parser.add_argument('scal', help='scal executable path for generating history')
  parser.add_argument('format', help='path to formatter')
  args = parser.parse_args()
  tests = get_tests(args.config)
  
  print('num_prod', 'num_cons', 'num_oper', 'result', 'runtime')
  for test in tests:
    result = 0
    while not result:
      num_prod = int(test[0])
      num_cons = int(test[1])
      num_oper = int(test[2])
      generate_history(num_prod, num_cons, num_oper, args.scal)
      format_history(args.format)
      result, runtime = run_test()
    os.remove(temp_filename)
    os.remove(temp_filename_2)
    print(num_prod, num_cons, num_oper, result, runtime)

if __name__ == '__main__':
  main()
#!/usr/bin/python3

import yaml
import os
dir_path = os.path.dirname(__file__)

config = None

with open(dir_path + "/common.yml") as file:
  config = yaml.safe_load(file)

import argparse
import subprocess
import time

scal_filename = 'scal.tmp.log'
polylin_filename = 'polylin.tmp.log'
violin_filename = 'violin.tmp.log'

def generate_history(num_prod: int, num_cons: int, num_oper: int, obj: str):
  num_val = int(num_oper/2/num_prod)
  mem_total = num_val * 8
  mem_padded = (int(mem_total/4096) + 1) * 409600
  subprocess.Popen(f'prodcon-{obj} -producers={num_prod} -consumers={num_cons} -operations={num_val} -log_operations=1 -print_summary=0 -shuffle_threads=1 -prealloc_size={mem_padded} > {scal_filename}', shell=True).wait()

def run_test():
  start_time = time.time()
  results = subprocess.Popen(f'../build/engine {polylin_filename}', shell=True, stdout=subprocess.PIPE).stdout.read()  
  return (int(results.decode()), time.time() - start_time)

def run_violin():
  start_time = time.time()
  executable = "../../violin/bin/logchecker.rb"
  cmd = executable + " " + violin_filename + " -a symbolic -t -r 5"
  proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
  proc.wait()
  total_time = time.time() - start_time
  output = proc.stdout.read().decode().split('\n')
  steps = [int(x[6:]) for x in output if x.startswith("STEPS:")][0]
  result = [int(x[10:]) for x in output if x.startswith("VIOLATION:")][0] != "true"
  return (result, total_time, steps)

def main():
  parser = argparse.ArgumentParser('runtest', './runtest.py <object>', 'script for running test suite')
  parser.add_argument('object', help='object to test')
  args = parser.parse_args()
  tests = []
  with open('test_config.csv', 'r') as test_file:
    tests = [line.split(' ') for line in test_file.readlines()]

  print('num_prod', 'num_cons', 'num_oper', 'result', 'runtime', 'violin-result', 'violin-runtime', 'violin-steps')
  for test in tests:
    num_prod = int(test[0])
    num_cons = int(test[1])
    num_oper = int(test[2])
    generate_history(num_prod, num_cons, num_oper, args.object)
    subprocess.Popen(f'./scal-polylin.py {scal_filename} {polylin_filename}', shell=True).wait()
    subprocess.Popen(f'./polylin-violin.py {polylin_filename} {violin_filename}', shell=True).wait()
    result, runtime = run_test()
    violin_result, violin_runtime, violin_steps = run_violin()
    print(num_prod, num_cons, num_oper, result, runtime, violin_result, violin_runtime, violin_steps)

if __name__ == '__main__':
  main()
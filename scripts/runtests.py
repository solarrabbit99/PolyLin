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
verilin_filename = 'verilin.tmp.log'

repeats = 10

# num_oper must be divisible by 20
def generate_history(num_oper: int, obj: str):
  num_prod = 10
  num_val = int(num_oper/2/num_prod)
  mem_total = num_val * 8
  mem_padded = (int(mem_total/4096) + 1) * 409600
  subprocess.Popen(f'./prodcon-{obj} -producers={num_prod} -consumers={num_prod} -operations={num_val} -log_operations=1 -print_summary=0 -shuffle_threads=1 -prealloc_size={mem_padded} > {scal_filename}', shell=True).wait()

def run_test():
  start_time = time.time()
  results = subprocess.Popen(f'../build/engine {polylin_filename}', shell=True, stdout=subprocess.PIPE).stdout.read()  
  return (int(results.decode()), time.time() - start_time)

def run_violin(max_steps):
  start_time = time.time()
  executable = config['violin']['path']
  cmd = executable + " " + violin_filename + " -a saturate -r -t 100 -s " + str(max_steps)
  proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
  proc.wait()
  total_time = time.time() - start_time
  output = proc.stdout.read().decode().split('\n')
  stepLine = [x[6:].strip() for x in output if x.startswith("STEPS:")][0]
  if not stepLine[-1].isdigit():
    stepLine = stepLine[:-1]
  steps = int(stepLine)
  result = [x[10:].strip() for x in output if x.startswith("VIOLATION:")][0] != "true"
  return (result, total_time, steps)

def run_verilin(impl):
  obj = config['impl'][impl]
  start_time = time.time()
  path = config['verilin']['path']
  cmd = f'java -Xss1024m -Xmx100g -cp {path} lockfree{obj}.VeriLin 0 0 {verilin_filename} 0 0'
  proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
  proc.wait()
  return time.time() - start_time

def main():
  parser = argparse.ArgumentParser('runtest', './runtest.py <object>', 'script for running test suite')
  parser.add_argument('object', help='object to test')
  args = parser.parse_args()
  tests = []
  with open('test_config.csv', 'r') as test_file:
    tests = [line.split(' ') for line in test_file.readlines()]

  print('num_oper', 'result', 'runtime', 'violin-result', 'violin-runtime', 'violin-steps', 'verilin-runtime')
  for test in tests:
    for _ in range(repeats):
      num_oper = int(test[0])
      generate_history(num_oper, args.object)
      subprocess.Popen(f'./scal-polylin.py {scal_filename} {polylin_filename} {args.object}', shell=True).wait()
      subprocess.Popen(f'./polylin-violin.py {polylin_filename} {violin_filename}', shell=True).wait()
      subprocess.Popen(f'./polylin-verilin.py {polylin_filename} {verilin_filename}', shell=True).wait()
      result, runtime = run_test()
      violin_result, violin_runtime, violin_steps = run_violin(num_oper*2)
      verilin_runtime = run_verilin(args.object)
      print(num_oper, result, runtime, violin_result, violin_runtime, violin_steps, verilin_runtime)

if __name__ == '__main__':
  main()
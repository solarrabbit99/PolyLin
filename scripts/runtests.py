#!/usr/bin/python3

import yaml
import os
dir_path = os.path.dirname(__file__)

config = None

with open(dir_path + "/common.yml") as file:
  config = yaml.safe_load(file)

import argparse
import subprocess

scal_filename = 'scal.tmp.log'
scal_pre_filename = 'scal-pre.tmp.log'
polylin_filename = 'polylin.tmp.log'
violin_filename = 'violin.tmp.log'
verilin_filename = 'verilin.tmp.log'

test_log = 'runtests.log'

cmd_prefix = '/usr/bin/time -f "%M %e" '

repeats = 10

# num_oper must be divisible by 20
def generate_history(num_oper: int, obj: str):
  num_prod = 10
  num_val = int(num_oper/2/num_prod)
  mem_total = num_val * 8
  mem_padded = (int(mem_total/4096) + 1) * 409600
  subprocess.Popen(f'./prodcon-{obj} -producers={num_prod} -consumers={num_prod} -operations={num_val} -log_operations=1 -print_summary=0 -shuffle_threads=1 -prealloc_size={mem_padded} > {scal_pre_filename}', shell=True).wait()

def remove_empties(src: str, dst: str):
  with open(src, 'r') as src_file, open(dst, 'w') as dst_file:
    while True:
      line = src_file.readline()
      if not line:
        break
      if line.split()[1] == '0': continue
      dst_file.write(line)

def run_test_cmd(cmd: str):
  full_cmd = cmd_prefix + cmd
  proc = subprocess.Popen(full_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  try:
    proc.wait(timeout=100)
  except subprocess.TimeoutExpired:
    proc.kill()
    return (-1, 100)
  output = proc.stdout.read().decode()
  with open(test_log, 'a') as log:
    log.write(full_cmd + '\n')
    log.write(output)
  return output.split('\n')[-2].split()

def run_polylin():
  return run_test_cmd(f'../build/polylin {polylin_filename}')

def run_violin():
  executable = config['violin']['path']
  cmd = executable + " " + violin_filename + " -a saturate -r"
  return run_test_cmd(cmd)

def run_verilin(impl):
  obj = config['impl'][impl]
  path = config['verilin']['path']
  cmd = f'java -Xss1024m -Xmx100g -cp {path} lockfree{obj}.VeriLin 0 0 {verilin_filename} 0 0'
  return run_test_cmd(cmd)

def main():
  parser = argparse.ArgumentParser('runtest', './runtest.py <object>', 'script for running test suite')
  parser.add_argument('object', help='object to test')
  args = parser.parse_args()
  tests = []
  with open('test_config.csv', 'r') as test_file:
    tests = [line.split(' ') for line in test_file.readlines()]

  print('num_oper', 'polylin-mem', 'polylin-time', 'violin-mem', 'violin-time', 'verilin-mem', 'verilin-time')
  for test in tests:
    for _ in range(repeats):
      num_oper = int(test[0])
      generate_history(num_oper, args.object)
      remove_empties(scal_pre_filename, scal_filename)
      subprocess.Popen(f'./scal-polylin.py {scal_filename} {polylin_filename} {args.object}', shell=True).wait()
      subprocess.Popen(f'./polylin-violin.py {polylin_filename} {violin_filename}', shell=True).wait()
      subprocess.Popen(f'./polylin-verilin.py {polylin_filename} {verilin_filename}', shell=True).wait()
      polylin_mem, polylin_time = run_polylin()
      violin_mem, violin_time = run_violin()
      verilin_mem, verilin_time = run_verilin(args.object)
      print(num_oper, polylin_mem, polylin_time, violin_mem, violin_time, verilin_mem, verilin_time)

if __name__ == '__main__':
  main()
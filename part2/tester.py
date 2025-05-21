import os
import subprocess

def compare_files(n):
    for i in range(2, n+1):
        diff = subprocess.run(['diff', 'output_1.txt', f'output_{i}.txt'], stdout=subprocess.PIPE)
        if diff.returncode != 0:
            print(diff.stdout.decode('utf-8'))

if __name__ == '__main__':
    n = int(input("Enter the value of n: "))
    compare_files(n)

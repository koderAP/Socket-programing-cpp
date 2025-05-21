import os
import subprocess
import glob

def compare_files():
    file_pattern = f'output_*'
    file_list = glob.glob(file_pattern)
    # print(file_list)
    if len(file_list) == 0:
        print(f"No files found for {file_pattern}")
        return
    for file_path in file_list:
        diff = subprocess.run(['diff', file_list[0], file_path], stdout=subprocess.PIPE)
        if diff.returncode != 0:
            print(diff.stdout.decode('utf-8'))

if __name__ == '__main__':
    compare_files()

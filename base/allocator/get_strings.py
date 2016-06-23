import os
import subprocess
import sys

import string
printable_tbl = [0] * 256
for i in range(256):
    if chr(i) in string.printable:
        printable_tbl[i] = 1
printable_tbl[ord('\n')] = 0
printable_tbl[ord('\r')] = 0
printable_tbl[ord('\t')] = 0

def is_printable(c):
    if printable_tbl[ord(c)] > 0:
        return True
    return False
    
def main():
    root_dir = os.path.dirname(os.path.abspath(__file__))
    input_file_path = sys.argv[1]
    output_file_path = sys.argv[2]
    with open(input_file_path, 'rb') as f:
        data = f.read()
    
    string_list = []
    min_length = 5
    
    off_start = -1
    for off in xrange(len(data)):
        if off_start == -1:
            if is_printable(data[off]):
                off_start = off
            else:
                pass
        else:
            if is_printable(data[off]):
                pass
            else:
                if off - off_start > min_length:
                    string_list.append(data[off_start:off])
                off_start = -1
    string_list = sorted(string_list)
    with open(output_file_path, 'wb+') as f:
        for s in string_list:
            if s.endswith('.obj'):
                f.write(s + '\n')
    
if __name__ == "__main__":
  sys.exit(main())

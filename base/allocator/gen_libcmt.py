import os
import subprocess
import sys

def assure_path_exist(p):
    if os.path.exists(p):
        return

    p = os.path.abspath(p)
    parent = os.path.dirname(p)
    if parent != p:
        if not os.path.exists(parent):
            assure_path_exist(parent)
    os.mkdir(p)
    
def find_vc_lib_path():
    lib_path = os.getenv('LIBPATH')
    for a in lib_path.split(';'):
        if '\Microsoft Visual Studio ' not in a:
            continue
        if a.rstrip('/\\').lower().endswith('vc\\lib'):
            return a
    return None
    
def main():
    root_dir = os.path.dirname(os.path.abspath(__file__))
    vc_lib_path = sys.argv[1]
    output_lib_path = sys.argv[2]
    if len(vc_lib_path) == 0:
        vc_lib_path = find_vc_lib_path()
        if vc_lib_path == None:
            print 'vc lib path not found'
            return -1
        else:
            print 'found vc lib path in env'
            
    print 'vc_lib_path %s'%vc_lib_path
    
    output_lib_path = os.path.abspath(output_lib_path)
    print 'output_lib_path %s'%output_lib_path
    
    assure_path_exist(output_lib_path)
    
    args = [
        sys.executable,
        os.path.join(root_dir, 'prep_libc.py'),
        vc_lib_path,
        output_lib_path,
        'ia32',
    ]
    subprocess.call(args)

if __name__ == "__main__":
  sys.exit(main())

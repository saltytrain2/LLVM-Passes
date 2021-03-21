import os
import sys #sys.argv python3 <exec> <filename>
import getopt
import subprocess
import tempfile
import string
from pathlib import Path
def main(argv):
    filename = argv[0].split(".")[0]
    binop = True

    # TODO: Remove this when ToB tool can create bc for us from executable
    os.system(f"clang -emit-llvm -o {filename}.bc -c {filename}.c")

    # build the mutate pass again
    process = subprocess.Popen("cd build; cmake ..; make",stdout=subprocess.PIPE, shell=True)
    proc_stdout = process.communicate()[0].strip()
    print(proc_stdout)


    instructions_to_modify = []

    with tempfile.TemporaryDirectory() as temp_dir_name:
        temp_file_path = Path(temp_dir_name) / "tempfile"
        os.system(f"clang -Xclang -load -Xclang build/pass/libSkeletonPass.so {filename}.bc > {temp_file_path}")
        
        with open(temp_file_path) as line_number_tmp_file:
            for line in line_number_tmp_file:
                opcode = line.split(' ')[0]
                opcode_number = int(line.split(' ')[1])

                # TODO: Determine which mutation you want to run and select for those opcodes
                # Examples of binary operators, add, mult, sdiv, sub
                print(f"Opcode: {opcode}, Linenumber: {opcode_number}")
                if opcode == "icmp":
                    instructions_to_modify.append(opcode_number)
    print(instructions_to_modify)

    mutantbc = []
    for index, instruction_number in enumerate(instructions_to_modify):
        os.system(f"opt -load build/pass/libSkeletonPass.so -mutatePass -mutation_loc={instruction_number} -mutation_op=icmp_eq < {filename}.bc > mutants/{filename}{instruction_number}.bc")
        mutantbc.append(f"mutants/{filename}{instruction_number}.bc")
    for fil in mutantbc:
        newexecutable = fil.split(".")[0] 
        os.system(f"llc -filetype=obj {fil}; clang {newexecutable}.o -o {newexecutable}")


if __name__ == "__main__":
   main(sys.argv[1:])
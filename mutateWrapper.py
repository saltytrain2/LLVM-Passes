import os
import sys #sys.argv python3 <exec> <filename>
import getopt
import subprocess
import tempfile
import string
from pathlib import Path
def main(argv):
    #Expected Usage : python3 mutateWrapper.py <filename relative path> <test suite marker> <opcode to modify> <category>
    #<opcode to modify> must be a llvm opcode
    #<category must be one of the following: "binop", "icmp"
    #Example Usage: python3 mutateWrapper.py exampleprograms/patchedDigit.c digit add binop 
    #Example Usage: python3 mutateWrapper.py exampleprograms/patchedDigit.c digit icmp icmp 
    filename = argv[0].split(".")[0]
    testsuite = argv[1]
    opcode_to_search = argv[2]
    category = argv[3]

    # TODO: Remove this when ToB tool can create bc for us from executable
    os.system(f"clang -emit-llvm -o {filename}.bc -c {filename}.c")

    # build the mutate pass again
    os.makedirs("build/", exist_ok=True)
    process = subprocess.Popen("cd build; cmake ..; make",stdout=subprocess.PIPE, shell=True)
    proc_stdout = process.communicate()[0].strip()
    print(proc_stdout)


    instructions_to_modify = []
    os.makedirs("exampleprograms/", exist_ok=True)
    os.makedirs("mutants/exampleprograms/", exist_ok=True)
    with tempfile.TemporaryDirectory() as temp_dir_name:
        temp_file_path = Path(temp_dir_name) / "tempfile"
        os.system(f"clang -Xclang -load -Xclang build/pass/libSkeletonPass.so {filename}.bc > {temp_file_path}")
        
        with open(temp_file_path) as line_number_tmp_file:
            for line in line_number_tmp_file:
                opcode = line.split(' ')[0]
                opcode_number = int(line.split(' ')[1])

                print(f"Opcode: {opcode}, Linenumber: {opcode_number}")
                if opcode == opcode_to_search:
                    instructions_to_modify.append(opcode_number)
    print(instructions_to_modify)

    mutantbc = []#list of mutant bc files generated
    # Edit the following mutant ops lists as needed to produce mutant files
    mutantops = ["add", "sub", "mul", "sdiv"] if category == "binop" else ["icmp_eq", "icmp_ne", "icmp_ugt", "icmp_uge","icmp_ult", "icmp_ule", "icmp_sgt", "icmp_sge", "icmp_slt", "icmp_sle"]
    for index, instruction_number in enumerate(instructions_to_modify):
        # os.system(f"opt -load build/pass/libSkeletonPass.so -mutatePass -mutation_loc={instruction_number} -mutation_op=icmp_eq < {filename}.bc > mutants/{filename}{instruction_number}.bc")
        for mutation in mutantops:
            os.system(f"clang -emit-llvm -o {filename}.bc -c {filename}.c")
            os.system(f"opt -load build/pass/libSkeletonPass.so -mutatePass -mutation_loc={instruction_number} -mutation_op={mutation} < {filename}.bc > mutants/{filename}_{instruction_number}_{mutation}.bc")

            mutantbc.append(f"mutants/{filename}_{instruction_number}_{mutation}.bc")

    executables = [] #list of executable names to be tested against a suite
    for fil in mutantbc:
        newexecutable = fil.split(".")[0] 
        os.system(f"llc -filetype=obj {fil}; clang {newexecutable}.o -o {newexecutable}")
        print(newexecutable)
        executables.append(newexecutable)
        
    # Running the generated mutants against the pytest suite.
    for exe in executables:
        print("Testing file: ", exe)
        os.system(f"python3 -m pytest -m {testsuite} --file {exe}")

    



if __name__ == "__main__":
   main(sys.argv[1:])
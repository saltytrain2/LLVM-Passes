
Dependencies required for the wrapper script: CMake 3.16+, LLVM 9+, opt, llc, pytest

How to use wrapper script to generate mutants and test them against a test suite:
Expected Usage : python3 mutateWrapper.py \<filename relative path\> \<test suite marker\> \<opcode to modify\> \<category\>
    
\<opcode to modify\> must be a llvm opcode
    
\<category\> must be one of the following: "binop", "icmp"
          
Example Usage: python3 mutateWrapper.py exampleprograms/patchedDigit.c digit add binop 

Example Usage: python3 mutateWrapper.py exampleprograms/patchedDigit2.c digit add binop 

Example Usage: python3 mutateWrapper.py exampleprograms/patchedDigit3.c digit add binop 
          
Example Usage: python3 mutateWrapper.py exampleprograms/patchedDigit.c digit icmp icmp 

Example Usage: python3 mutateWrapper.py exampleprograms/unpatchedDigit.c digit icmp icmp 


To add a new test suite: 
Edit the pytest.ini file with a new marker called {marker}, and then create a pytest file called "test_{filename}.py" and 
mark test functions with @pytest.mark.{marker}.


Then to test:
python3 -m pytest --file {name of executable found in mutants/exampleprograms/}

Or to manually make mutations:
Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:
    To run passes that aren't mutate: edit the pass in RegisterMyPass. 
    Ex:
  static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerLabelPass);
    Then run:
    $ clang -Xclang -load -Xclang build/pass/libSkeletonPass.* example.c

    _________________________________
    To run mutate passes:
    1. Emit bc file from source: clang -emit-llvm -o example.bc -c example.c
    2. Find instruction number using labelPass: clang -Xclang -load -Xclang build/pass/libSkeletonPass.* example.c
    3. Specify instruction location to mutate and mutation operand type and run the mutate pass on bc file: opt -load build/pass/libSkeletonPass.so -mutatePass -mutation_loc=33 -mutation_op=swapplus1toplus2 < functioncall.bc > functioncallnew.bc
    4. Create object file from edited bc file: llc -filetype=obj functioncallnew.bc
    5. Create executable: clang functioncallnew.o
    6. Run executable: ./a.out

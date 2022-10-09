
<!-- Dependencies required for the wrapper script: CMake 3.16+, LLVM 13, opt, llc, pytest -->

<!-- How to use wrapper script to generate mutants and test them against a test suite:
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
python3 -m pytest --file {name of executable found in mutants/exampleprograms/} -->

# Requirements
- LLVM 13
- CMake 3.14+

# Optional Requirements
- jsoncpp 


# Installation Steps
To build all passes, run the following commands, where PATH_TO_LLVM_DIR is the full path to root of the LLVM 13 folder

    $ mkdir build
    $ cd build
    $ cmake -DLT_LLVM_INSTALL_DIR=PATH_TO_LLVM_DIR ../
    $ make

# Running the Pass
Once your build the passes, all passes will be stored inside the libResearchPasses.so shared library, which will be ususally found under the build/lib directory. To run any pass on either llvm bitcode or readable llvm assembly, 

    opt -load <path/to/libResearchPasses.so> -command_line_options ... -load-pass-plugin <path/to/libResearchPasses.so> -passes=desired_pass file.bc

    opt -load <path/to/libResearchPasses.so> -command_line_options ... -load-pass-plugin <path/to/libResearchPasses.so> -passes=desired_pass file.ll -S

# Adding Passes
Additional Passes should be made into classes, where the .h goes into the include folder and the .cpp goes into the pass folder. An example pass header is shown below
```C
#ifndef _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H
#define _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H

#include <string>
#include <cstdint>

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class RegisterExitPass : public PassInfoMixin<RegisterExitPass>
{
public:
    RegisterExitPass(std::string outputFile);
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Module& M, ModuleAnalysisManager&);

private:
    std::string mOutputFile;

    void insertGlobals(Module& M);
    Constant* getInBoundsGEP(Type* ty, std::string anem, Module& M, uint64_t index);
    FunctionCallee insertHandler(Module& M);
    FunctionCallee insertSignalHandler(Module& M);
    bool runOnModule(Module& M);
}; 
} // namespace llvm

#endif // _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H
```
A couple of points:
- To access the types and methods inside the llvm namespace, it is recommended to encapsulate your pass inside the llvm namespace instead of `using namespace llvm;` to prevent accidental namespace imports. Importing the llvm namespace in your .cpp implemenation is absolutely fine
- Each pass you write should publicly inherit from PassInfoMixin\<YOUR_PASS_NAME>
- Passes are allowed to have alternate constructors, see below.
- Each pass must define a run method that returns `PreservedAnalyses::none()` if your pass modified the IR or `PreservedAnalyses::all()` if your pass left the IR unchanged.
- overriding `static bool isRequired()` to return true is highly recommended to allow your pass to run on IR that was created with -O0 or default optimization


Once your pass is made, simply add the pass to the ResearchPasses.cpp file by specifying the command line name you want to use to invoke your pass and adding its constructor to the appropriate Pass Manager. If you want your pass to have parameters, simply add your command line arguments to the ResearchPasses.cpp file by declaring a new cl::opt class with the desired command line name, description, and type and passing in the values to the constructor. 

# Additional Resources
- https://releases.llvm.org/13.0.0/docs/WritingAnLLVMNewPMPass.html More information about creating a pass
- https://github.com/banach-space/llvm-tutor Example code for different types of common passes that can be done (Assumes latest stable LLVM version)
- https://releases.llvm.org/13.0.0/docs/ProgrammersManual.html Programming tips and tricks with llvm
- https://llvm.org/doxygen/ API for latest version of llvm, mostly the same as LLVM 13
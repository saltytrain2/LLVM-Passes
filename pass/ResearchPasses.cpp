#include "llvm/Pass.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/CommandLine.h"

#include "SkeletonPass.h"
#include "MemoryPass.h"
#include "LabelPass.h"
#include "MutatePass.h"
#include "RegisterInfoPass.h"
#include "RegisterExitPass.h"
#include "WindowPass.h"
#include "SingleWindowPass.h"
#include "FuncNameChangePass.h"

using namespace llvm;

static cl::opt<uint32_t> MutationLocation("mutation_loc", cl::desc("Specify the instruction number that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> MutationOp("mutation_op", cl::desc("Specify operator to mutate with e.g., icmp_eq, swapFuncCall, swapFuncParam, funcConstParam"), cl::value_desc("String"));

static cl::opt<std::string> MutationVal("mutation_val", cl::desc("Specify value for constant mutation (Mutation_Op flag must be funcConstParam"), cl::value_desc("String"));

static cl::opt<uint32_t> ParameterLocation("parameter_loc", cl::desc("Specify the parameter number in the function that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> FunctionName("function_name", cl::desc("Specify name of the function to swap to (Mutation_Op flag must be swapFuncCall"), cl::value_desc("String"));

static cl::opt<std::string> OutputFile("output_file", cl::desc("Output filename."), cl::value_desc("String"));

static cl::opt<std::string> SwapParamNums("swap_param_nums", cl::desc("Specify the position of the two parameters to swap (Mutation_op flag must be swapFuncParam, declared as 'num1 num2')"), cl::value_desc("String"));

static cl::opt<std::string> HashAlgorithm("hash_algorithm", cl::desc("Specify the hashing algorithm to implement (Mutation_op flag must be changeHash)"), cl::value_desc("String"));


llvm::PassPluginLibraryInfo getSkeletonPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "skeleton-pass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "skeleton-pass") {
                    FPM.addPass(SkeletonPass());
                    return true;
                  } else if (Name == "memory-pass") {
                    FPM.addPass(MemoryPass());
                    return true;
                  }
                  return false;
                });
            PB.registerPipelineParsingCallback(
              [](StringRef Name, ModulePassManager& MPM,
                 ArrayRef<PassBuilder::PipelineElement>) {
                   if (Name == "mutate-pass") {
                        MPM.addPass(MutatePass(MutationOp, MutationLocation, SwapParamNums, HashAlgorithm));
                        return true;
                   } else if (Name == "label-pass") {
                        MPM.addPass(LabelPass(OutputFile));
                        return true;
                   } else if (Name == "register-info-pass") {
                        MPM.addPass(RegisterInfoPass(MutationLocation));
                        return true;
                   } else if (Name == "register-exit-pass") {
                        MPM.addPass(RegisterExitPass(OutputFile));
                        return true;
                   } else if (Name == "window-pass") {
                        MPM.addPass(WindowPass(OutputFile, FunctionName));
                        return true;
                   } else if (Name == "single-window-pass") {
                         MPM.addPass(SingleWindowPass(MutationLocation, MutationOp));
                         return true;
                   } else if (Name == "func-name-change-pass") {
                         MPM.addPass(FuncNameChangePass(OutputFile));
                         return true;
                   }
                   return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getSkeletonPassPluginInfo();
}
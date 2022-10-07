#ifndef _LLVMPASSES__INCLUDE__FUNCNAMECHANGEPASS_H
#define _LLVMPASSES__INCLUDE__FUNCNAMECHANGEPASS_H

#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/IRBuilder.h"

class FuncNameChangePass : public llvm::PassInfoMixin<FuncNameChangePass>
{
public:
    FuncNameChangePass(std::string file);
    inline static bool isRequired() { return true; }
    llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager&);

private:
    std::ofstream mOutputFile;
    bool runOnModule(llvm::Module& M);
};

#endif // _LLVMPASSES__INCLUDE__FUNCNAMECHANGEPASS_H

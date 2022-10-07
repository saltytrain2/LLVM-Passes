#ifndef _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H
#define _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H

#include <string>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"


class RegisterExitPass : public llvm::PassInfoMixin<RegisterExitPass>
{
public:
    RegisterExitPass(std::string outputFile);
    inline static bool isRequired() { return true; }
    llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager&);

private:
    std::string mOutputFile;

    void insertGlobals(llvm::Module& M);
    llvm::Constant* getInBoundsGEP(llvm::Type* ty, std::string anem, llvm::Module& M, uint64_t index);
    llvm::FunctionCallee insertHandler(llvm::Module& M);
    llvm::FunctionCallee insertSignalHandler(llvm::Module& M);
    bool runOnModule(llvm::Module& M);
}; 

#endif // _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H
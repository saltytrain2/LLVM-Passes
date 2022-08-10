#ifndef _LLVMPASSES__INCLUDE__REGISTERINFOPASS_H
#define _LLVMPASSES__INCLUDE__REGISTERINFOPASS_H

#include <cstdint>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

class RegisterInfoPass : public llvm::PassInfoMixin<RegisterInfoPass>
{
public:
    RegisterInfoPass(uint32_t mutationLoc);
    inline static bool isRequired() { return true; }
    llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager&);

private:
    uint32_t mMutationLoc;

    bool runOnModule(llvm::Module& M);
    void insertGlobals(llvm::Module& M);
    void insertFunctions(llvm::Module& M);
    void insertConstants(llvm::Module& M);
    llvm::Constant* getInBoundsGEP(llvm::Type* ty, std::string name, llvm::Module& M, uint64_t index);
    void injectRegisterInsts(llvm::Instruction* inst, llvm::Module& M);
};


#endif // _LLVMPASSES__INCLUDE__REGISTERINFOPASS_H
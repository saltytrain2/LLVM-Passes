#ifndef _LLVMPASSES__INCLUDE__REGISTERINFOPASS_H
#define _LLVMPASSES__INCLUDE__REGISTERINFOPASS_H

#include <cstdint>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

namespace llvm {
class RegisterInfoPass : public PassInfoMixin<RegisterInfoPass>
{
public:
    RegisterInfoPass(uint32_t mutationLoc);
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Module& M, ModuleAnalysisManager&);

private:
    uint32_t mMutationLoc;

    bool runOnModule(Module& M);
    void insertGlobals(Module& M);
    void insertFunctions(Module& M);
    void insertConstants(Module& M);
    Constant* getInBoundsGEP(Type* ty, std::string name, Module& M, uint64_t index);
    void injectRegisterInsts(Instruction* inst, Module& M);
};
} // namespace llvm

#endif // _LLVMPASSES__INCLUDE__REGISTERINFOPASS_H
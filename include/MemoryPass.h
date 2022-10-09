#ifndef _LLVMPASSES__INCLUDE__MEMORYPASS_H
#define _LLVMPASSES__INCLUDE__MEMORYPASS_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"


namespace llvm {
class MemoryPass : public PassInfoMixin<MemoryPass>
{
public:
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Function& F, FunctionAnalysisManager&);

private:
    bool runOnFunction(Function& F);
};
} // namespace llvm

#endif // _LLVMPASSES__INCLUDE__MEMORYPASS_H
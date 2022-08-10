#ifndef _LLVMPASSES__INCLUDE__SKELETONPASS_H
#define _LLVMPASSES__INCLUDE__SKELETONPASS_H

#include "llvm/Pass.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
using namespace llvm;

class SkeletonPass : public PassInfoMixin<SkeletonPass>
{
public:
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Function& F, FunctionAnalysisManager&);

private:
    bool runOnFunction(Function& F);
};

#endif // _LLVMPASSES__INCLUDE__SKELETONPASS_H
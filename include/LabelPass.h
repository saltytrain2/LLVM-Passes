#ifndef _LLVMPASSES__INCLUDE__LABELPASS_H
#define _LLVMPASSES__INCLUDE__LABELPASS_H

#include <fstream>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

class LabelPass : public PassInfoMixin<LabelPass>
{
public:
    LabelPass(std::string file);

    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Module& M, ModuleAnalysisManager&);

private:
    std::ofstream mStream;
    bool runOnModule(Module& M);
};

#endif // _LLVMPASSES__INCLUDE__LABELPASS_H
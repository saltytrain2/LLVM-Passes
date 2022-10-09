#ifndef _LLVMPASSES__INCLUDE__SINGLEWINDOWPASS_H
#define _LLVMPASSES__INCLUDE__SINGLEWINDOWPASS_H

#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/IRBuilder.h"

namespace llvm {
class SingleWindowPass : public PassInfoMixin<SingleWindowPass>
{
public:
    SingleWindowPass(uint32_t mutationLoc, std::string mutationOp);
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Module& M, ModuleAnalysisManager&);

private:
    uint32_t mMutationLoc;
    std::string mMutationOp;
    std::unique_ptr<std::vector<std::string>> mPredWindows;

    void write(Instruction* I, std::string& window);
    void createWindows(inst_iterator instToWrite, inst_iterator startInst, inst_iterator endInst);
    void insertBefore(inst_iterator instToWrite, inst_iterator startInst, std::string window, uint8_t depthleft);
    void insertAfter(inst_iterator instToWrite, inst_iterator endInst, std::string window, uint8_t depthleft);
    bool runOnModule(Module& M);
};
} // namespace llvm

#endif // _LLVMPASSES__INCLUDE__SINGLEWINDOWPASS_H
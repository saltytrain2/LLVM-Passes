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

class SingleWindowPass : public llvm::PassInfoMixin<SingleWindowPass>
{
public:
    SingleWindowPass(uint32_t mutationLoc, std::string mutationOp);
    inline static bool isRequired() { return true; }
    llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager&);

private:
    uint32_t mMutationLoc;
    std::string mMutationOp;
    std::unique_ptr<std::vector<std::string>> mPredWindows;

    void write(llvm::Instruction* I, std::string& window);
    void createWindows(llvm::inst_iterator instToWrite, llvm::inst_iterator startInst, llvm::inst_iterator endInst);
    void insertBefore(llvm::inst_iterator instToWrite, llvm::inst_iterator startInst, std::string window, uint8_t depthleft);
    void insertAfter(llvm::inst_iterator instToWrite, llvm::inst_iterator endInst, std::string window, uint8_t depthleft);
    bool runOnModule(llvm::Module& M);
};

#endif // _LLVMPASSES__INCLUDE__SINGLEWINDOWPASS_H
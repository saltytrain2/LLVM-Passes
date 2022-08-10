#ifndef _LLVMPASSES__INCLUDE__MUTATEPASS_H
#define _LLVMPASSES__INCLUDE__MUTATEPASS_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/IRBuilder.h"


class MutatePass : public llvm::PassInfoMixin<MutatePass>
{
public:
    MutatePass(std::string mutationOp, uint32_t mutationLoc, std::string swapParamNums, std::string hash);
    inline static bool isRequired() { return true; }
    llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager&);

private:
    uint32_t mMutationLoc;
    std::string mMutationOp;
    std::string mSwapParamNums;
    std::string mHash;
    bool runOnModule(llvm::Module& M);

    llvm::Instruction* getMutantLoadInst(llvm::Instruction* I);
    llvm::Instruction* getMutantCallInst(llvm::Instruction* I, llvm::Module& M);
    bool getMutantBranchInst(llvm::Instruction* I);
    llvm::Value* getTypeCast(llvm::IRBuilder<>& builder, llvm::Value* firstParam, llvm::Value* secondParam);
    llvm::Instruction* getMutantBinaryIntegerInst(llvm::BinaryOperator* binop);
    llvm::Instruction* getMutantBinaryFloatingInst(llvm::BinaryOperator* binop);
    llvm::Instruction* getMutantBinaryLogicalInst(llvm::BinaryOperator* binop);
    llvm::Instruction* getMutantBinaryShiftInst(llvm::BinaryOperator* binop);
    llvm::Instruction* getMutantBinaryInst(llvm::Instruction* I);
    llvm::Instruction* getMutantIcmpUnsignedInst(llvm::ICmpInst* cmpInst);
    llvm::Instruction* getMutantIcmpSignedInst(llvm::ICmpInst* cmpInst);
    llvm::Instruction* getMutantIcmpInst(llvm::Instruction* I);
    llvm::Instruction* getMutantFcmpOrderedInst(llvm::FCmpInst* cmpInst);
    llvm::Instruction* getMutantFcmpUnorderedInst(llvm::FCmpInst* cmpInst);
    llvm::Instruction* getMutantFcmpInst(llvm::Instruction* I);
    llvm::Instruction* getMutantIntegerSignCastInst(llvm::Instruction* I);
    
};

#endif // _LLVMPASSES__INCLUDE__MUTATEPASS_H
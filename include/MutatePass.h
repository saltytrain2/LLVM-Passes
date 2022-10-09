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


namespace llvm {
class MutatePass : public PassInfoMixin<MutatePass>
{
public:
    MutatePass(std::string mutationOp, uint32_t mutationLoc, std::string swapParamNums, std::string hash);
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Module& M, ModuleAnalysisManager&);

private:
    uint32_t mMutationLoc;
    std::string mMutationOp;
    std::string mSwapParamNums;
    std::string mHash;
    bool runOnModule(Module& M);

    Instruction* getMutantLoadInst(Instruction* I);
    Instruction* getMutantCallInst(Instruction* I, Module& M);
    bool getMutantBranchInst(Instruction* I);
    Value* getTypeCast(IRBuilder<>& builder, Value* firstParam, Value* secondParam);
    Instruction* getMutantBinaryIntegerInst(BinaryOperator* binop);
    Instruction* getMutantBinaryFloatingInst(BinaryOperator* binop);
    Instruction* getMutantBinaryLogicalInst(BinaryOperator* binop);
    Instruction* getMutantBinaryShiftInst(BinaryOperator* binop);
    Instruction* getMutantBinaryInst(Instruction* I);
    Instruction* getMutantIcmpUnsignedInst(ICmpInst* cmpInst);
    Instruction* getMutantIcmpSignedInst(ICmpInst* cmpInst);
    Instruction* getMutantIcmpInst(Instruction* I);
    Instruction* getMutantFcmpOrderedInst(FCmpInst* cmpInst);
    Instruction* getMutantFcmpUnorderedInst(FCmpInst* cmpInst);
    Instruction* getMutantFcmpInst(Instruction* I);
    Instruction* getMutantIntegerSignCastInst(Instruction* I);
    
};
} // namespace llvm

#endif // _LLVMPASSES__INCLUDE__MUTATEPASS_H
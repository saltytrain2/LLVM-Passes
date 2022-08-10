#include "MutatePass.h"

#include <sstream>
#include <string>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;

MutatePass::MutatePass(std::string mutationOp, uint32_t mutationLoc, std::string swapParamNums, std::string hash)
    :  mMutationLoc(mutationLoc), mMutationOp(mutationOp), mSwapParamNums(swapParamNums), mHash(hash)
{}

PreservedAnalyses MutatePass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool MutatePass::runOnModule(Module& M)
{
    bool bModified = false;
    uint64_t instrCnt = 0;

    for (auto& F: M) {
        if (F.isDeclaration()) 
            continue;

        for (inst_iterator i = inst_begin(F); i != inst_end(F) && !bModified; ++i) {
            if (instrCnt == mMutationLoc) {
                Instruction* I = &*i;
                Instruction* altI = nullptr;

                if (mMutationOp == "null") {
                    outs() << "nothing modified\n";
                    return false;
                }

                if (isa<LoadInst>(I)) {
                    outs() << "Replacing Load Instruction: ";
                    altI = getMutantLoadInst(I);
                } else if (isa<ICmpInst>(I)) {
                    outs() << "Replacing Immediate Comparison Instruction: ";
                    altI = getMutantIcmpInst(I);
                } else if (isa<FCmpInst>(I)) {
                    outs() << "Replacing Floating-point Comparison Instruction: ";
                    altI = getMutantFcmpInst(I);
                } else if (isa<BinaryOperator>(I)) {
                    outs() << "Replacing Binary Instruction: ";
                    altI = getMutantBinaryInst(I);
                } else if (isa<CallInst>(I)) {
                    outs() << "Replacing Call Instruction: ";
                    altI = getMutantCallInst(I, M);
                } else if (isa<BranchInst>(I)) {
                    outs() << "Replacing Branch Instruction: ";
                    if (!getMutantBranchInst(I)) {
                        outs() << "no else block detected\n";
                    } else {
                            outs() << "else block successfully eliminated\n";
                        bModified = true;
                    }
                    break;
                } else if (isa<CastInst>(I)) {
                    outs() << "Replacing Cast Instruction: ";
                    altI = getMutantIntegerSignCastInst(I);
                }

                if (altI == nullptr) {
                    outs() << "nothing modified\n";
                    return bModified;
                } else {
                    outs() << "\n";
                }

                ReplaceInstWithInst(I, altI);
                bModified = true;
                break;
            }
            ++instrCnt;
        }
    }

    // for (unsigned i = 0; i < instToDelete.size(); ++i) {
    //     outs() << "Deleted\n";
    //     instToDelete[i]->eraseFromParent();
    // }

    // for (auto &F : M) {
    //     stringToFunc[std::string(F.getName())] = &F;
    //     outs() << F.getName() << "\n";
    //}
    return bModified;
}

Instruction* MutatePass::getMutantLoadInst(Instruction* I) 
{
    if (mMutationOp == "loadint8") {
        auto *op = dyn_cast<LoadInst>(I);
        IRBuilder<> builder(op);
        LLVMContext &context = I->getFunction()->getContext();
        Type* type = Type::getInt8Ty(context); //Note this only should be done if the type is larger than 8. ie you mutate a 16 byte load to an 8 byte load.

        return dyn_cast<Instruction>(builder.CreateLoad(type, op->getOperand(0)));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantCallInst(Instruction* I, Module& M) 
{
    CallInst* callInst = dyn_cast<CallInst>(I);
    IRBuilder<> builder(callInst);
    // dont attempt to do any modifications with llvm intrinsic functions
    if (callInst->getCalledFunction() && callInst->getCalledFunction()->getName().startswith("llvm.")) {
        return nullptr;
    }

    if (mMutationOp == "swapFuncParam") { //TODO - make this function more robust for type-implicit conversions
        std::stringstream paramIndices(mSwapParamNums); // populates stringstream with two indices values
        std::vector<Value*> newArgs;

        // extract the two indices
        int firstIndex, secondIndex;
        paramIndices >> firstIndex;
        paramIndices >> secondIndex;

        for (uint32_t i = 0; i < callInst->getNumArgOperands(); ++i) {
            newArgs.push_back(callInst->getArgOperand(i));
        }

        llvm::Value* firstParam = newArgs[firstIndex];
        llvm::Value* secondParam = newArgs[secondIndex];

        if (firstParam->getType() == secondParam->getType()) {
            std::swap(newArgs[firstIndex], newArgs[secondIndex]);
        } else {
            auto firstCast = getTypeCast(builder, firstParam, secondParam);
            // if a type conversion is not supported, do not replace any instructions
            if (firstCast == nullptr) {
                return nullptr;
            }
            auto secondCast = getTypeCast(builder, secondParam, firstParam);
            std::swap(newArgs[firstIndex], secondCast);
            std::swap(newArgs[secondIndex], firstCast);
        } 
        return CallInst::Create(callInst->getFunctionType(), callInst->getCalledOperand(), newArgs);
    } else if (mMutationOp == "changeHash") {
        // assumes the user is using the EPV_DIGEST interface of openssl
        // Inject a call to EVP_md5
        auto& ctx = M.getContext();
        auto structTypes = M.getIdentifiedStructTypes();
        bool foundType = false;
        FunctionType* hashType = nullptr;

        for (auto i: structTypes) {
            if (i->getName() == StringRef("struct.evp_md_st")) {
                foundType = true;
                hashType = FunctionType::get(PointerType::getUnqual(i), false);
                break;
            }
        }

        if (!foundType) {
            hashType = FunctionType::get(PointerType::getUnqual(StructType::create(ctx, StringRef("struct.epv_md_st"))), false);
            return nullptr;
        }
        FunctionCallee EPV_hash = M.getOrInsertFunction(StringRef("EVP_" + mHash), hashType);

        return dyn_cast<Instruction>(builder.CreateCall(EPV_hash));
    } else if (mMutationOp == "removeVoidCall") {
        // eliminates a call to a void function, which changes state
        if (callInst->getCalledFunction() && !callInst->getCalledFunction()->getReturnType()->isVoidTy()) {
            return nullptr;
        }

        Function* nop = Intrinsic::getDeclaration(&M, Intrinsic::donothing);
        return CallInst::Create(nop->getFunctionType(), nop, NoneType::None);
    }

    return nullptr;
}

bool MutatePass::getMutantBranchInst(Instruction* I) 
{
    BranchInst* branchInst = dyn_cast<BranchInst>(I);
    IRBuilder<> builder(branchInst);

    if (mMutationOp == "removeElseBlock") {
        // check to make sure the current branch inst is actually an if-else block
        if (!branchInst->isConditional() || branchInst->getNumSuccessors() != 2) {
            return false;
        }

        Value* condValue = branchInst->getCondition();
        BasicBlock* thenBlock = branchInst->getSuccessor(0);
        BasicBlock* elseBlock = branchInst->getSuccessor(1);
        BasicBlock* thenBlockSuccessor = thenBlock->getUniqueSuccessor();
        BasicBlock* elseBlockSuccessor = elseBlock->getUniqueSuccessor();
        BasicBlock* thenBlockPredecessor = thenBlock->getUniquePredecessor();
        BasicBlock* elseBlockPredecessor = elseBlock->getUniquePredecessor();

        // check to make sure the if-else block has no nested blocks
        if (thenBlockSuccessor != elseBlockSuccessor || thenBlockPredecessor != elseBlockPredecessor ||
            thenBlockSuccessor == nullptr || thenBlockPredecessor == nullptr) {
            return false;
        }

        builder.CreateCondBr(condValue, thenBlock, thenBlockSuccessor);
        I->eraseFromParent();
        DeleteDeadBlock(elseBlock);
        return true;
    }
    return false;
}

Value* MutatePass::getTypeCast(IRBuilder<>& builder, llvm::Value* firstParam, llvm::Value* secondParam) 
{
    // begin a series of statements to determine the types of the parameters to generate the correct IRBuilder cast call
    // TODO, all references to integers are assumed to be unsigned, have to distinguish since llvm makes no explicit distinctions between signed and unsigned ints
    if (firstParam->getType()->isIntegerTy() && secondParam->getType()->isFloatingPointTy()) {
        return builder.CreateUIToFP(firstParam, secondParam->getType());
    } else if (firstParam->getType()->isFloatingPointTy() && secondParam->getType()->isIntegerTy()) {
        return builder.CreateFPToUI(firstParam, secondParam->getType());
    } else if (firstParam->getType()->isPointerTy() && secondParam->getType()->isIntegerTy()) {
        return builder.CreatePtrToInt(firstParam, secondParam->getType());
    } else if (firstParam->getType()->isIntegerTy() && secondParam->getType()->isPointerTy()) {
        return builder.CreateIntToPtr(firstParam, secondParam->getType());
    } else if (firstParam->getType()->isIntegerTy() && secondParam->getType()->isIntegerTy()) {
        return builder.CreateSExtOrTrunc(firstParam, secondParam->getType());
    } else if (firstParam->getType()->isPointerTy() && secondParam->getType()->isPointerTy()) {
        return builder.CreateBitOrPointerCast(firstParam, secondParam->getType());
    } else {
        return nullptr;
    }
}

Instruction* MutatePass::getMutantBinaryIntegerInst(BinaryOperator* binop) 
{
    if (mMutationOp == "add"){
        return binop->getOpcode() == Instruction::BinaryOps::Add ? nullptr : BinaryOperator::CreateAdd(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "sub") {
        return binop->getOpcode() == Instruction::BinaryOps::Sub ? nullptr : BinaryOperator::CreateSub(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "mul") {
        return binop->getOpcode() == Instruction::BinaryOps::Mul ? nullptr : BinaryOperator::CreateMul(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "udiv") {
        return binop->getOpcode() == Instruction::BinaryOps::UDiv ? nullptr : BinaryOperator::CreateUDiv(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "sdiv") {
        return binop->getOpcode() == Instruction::BinaryOps::SDiv ? nullptr : BinaryOperator::CreateSDiv(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "urem") {
        return binop->getOpcode() == Instruction::BinaryOps::URem ? nullptr : BinaryOperator::CreateURem(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "srem") {
        return binop->getOpcode() == Instruction::BinaryOps::SRem ? nullptr : BinaryOperator::CreateSRem(binop->getOperand(0), binop->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantBinaryFloatingInst(BinaryOperator* binop) 
{
    if (mMutationOp == "fadd") {
        return binop->getOpcode() == Instruction::BinaryOps::FAdd ? nullptr : BinaryOperator::CreateFAdd(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "fsub") {
        return binop->getOpcode() == Instruction::BinaryOps::FSub ? nullptr : BinaryOperator::CreateFSub(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "fmul") {
        return binop->getOpcode() == Instruction::BinaryOps::FMul ? nullptr : BinaryOperator::CreateFMul(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "fdiv") {
        return binop->getOpcode() == Instruction::BinaryOps::FDiv ? nullptr :  BinaryOperator::CreateFDiv(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "frem") {
        return binop->getOpcode() == Instruction::BinaryOps::FRem ? nullptr : BinaryOperator::CreateFRem(binop->getOperand(0), binop->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantBinaryLogicalInst(BinaryOperator* binop) 
{
    if (mMutationOp == "and") {
        return binop->getOpcode() == Instruction::BinaryOps::And ? nullptr : BinaryOperator::CreateAnd(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "or") {
        return binop->getOpcode() == Instruction::BinaryOps::Or ? nullptr : BinaryOperator::CreateOr(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "xor") {
        return binop->getOpcode() == Instruction::BinaryOps::Xor ? nullptr : BinaryOperator::CreateXor(binop->getOperand(0), binop->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantBinaryShiftInst(BinaryOperator* binop) 
{
    if (mMutationOp == "shl") {
        return binop->getOpcode() == Instruction::BinaryOps::Shl ? nullptr : BinaryOperator::CreateShl(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "lshr") {
        return binop->getOpcode() == Instruction::BinaryOps::LShr ? nullptr : BinaryOperator::CreateLShr(binop->getOperand(0), binop->getOperand(1));
    } else if (mMutationOp == "ashr") {
        return binop->getOpcode() == Instruction::BinaryOps::AShr ? nullptr : BinaryOperator::CreateAShr(binop->getOperand(0), binop->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantBinaryInst(Instruction* I) {
    BinaryOperator* binop = dyn_cast<BinaryOperator>(I);
    auto opcode = binop->getOpcode();
    auto opname = binop->getOpcodeName(opcode);

    if (opcode < Instruction::BinaryOps::Shl && opname[0] != 'f') {
        return getMutantBinaryIntegerInst(binop);
    } else if (opcode < Instruction::BinaryOps::Shl) {
        return getMutantBinaryFloatingInst(binop);
    } else if (opcode < Instruction::BinaryOps::And) {
        return getMutantBinaryShiftInst(binop);
    } else {
        return getMutantBinaryLogicalInst(binop);
    } 
}

Instruction* MutatePass::getMutantIcmpUnsignedInst(ICmpInst* cmpInst)
{
    if (mMutationOp == "icmpUgt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_UGT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_UGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if(mMutationOp == "icmpUge") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_UGE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_UGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if(mMutationOp == "icmpUlt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_ULT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_ULT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if(mMutationOp == "icmpUle") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_ULE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_ULE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    }

    return nullptr;
}

Instruction* MutatePass::getMutantIcmpSignedInst(ICmpInst* cmpInst)
{
    if(mMutationOp == "icmpSgt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGT ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if(mMutationOp == "icmpSge") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGE ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if(mMutationOp == "icmpSlt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLT ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SLT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if(mMutationOp == "icmpSle") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLE ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SLE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantIcmpInst(Instruction* I) 
{
    ICmpInst* cmpInst = dyn_cast<ICmpInst>(I);
    IRBuilder<> builder(cmpInst);

    if (mMutationOp == "icmpEq") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_EQ ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_EQ, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "icmpNe") {
        return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_NE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_NE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (cmpInst->getPredicate() < CmpInst::Predicate::ICMP_SGT) {
        return getMutantIcmpUnsignedInst(cmpInst);
    } else {
        return getMutantIcmpSignedInst(cmpInst);
    }
}

Instruction* MutatePass::getMutantFcmpOrderedInst(FCmpInst* cmpInst)
{
    if (mMutationOp == "fcmpOeq") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OEQ ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OEQ, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpOne") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ONE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ONE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpOgt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OGT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpOge") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OGE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpOlt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OLT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OLT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpOle") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OLE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OLE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpOrd") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ORD ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ORD, cmpInst->getOperand(0), cmpInst->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantFcmpUnorderedInst(FCmpInst* cmpInst)
{
    if (mMutationOp == "fcmpUeq") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UEQ ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UEQ, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpUne") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UNE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UNE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpUgt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UGT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpUge") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UGE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpUlt") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ULT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ULT, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpUle") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ULE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ULE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpUno") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UNO ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UNO, cmpInst->getOperand(0), cmpInst->getOperand(1));
    }
    return nullptr;
}

Instruction* MutatePass::getMutantFcmpInst(Instruction* I) 
{
    FCmpInst* cmpInst = dyn_cast<FCmpInst>(I);

    if (mMutationOp == "fcmpTrue") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_TRUE ? nullptr : cmpInst-> CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_TRUE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (mMutationOp == "fcmpFalse") {
        return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_FALSE ? nullptr : cmpInst-> CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_FALSE, cmpInst->getOperand(0), cmpInst->getOperand(1));
    } else if (!(cmpInst->getPredicate() & 0x8)) {
        return getMutantFcmpOrderedInst(cmpInst);
    } else {
        return getMutantFcmpUnorderedInst(cmpInst);
    }

    return nullptr;
}

Instruction* MutatePass::getMutantIntegerSignCastInst(Instruction* I) 
{
    CastInst* castInst = dyn_cast<CastInst>(I);
    if (mMutationOp == "zext") {
        return castInst->getOpcode() == Instruction::CastOps::SExt ? CastInst::Create(Instruction::CastOps::ZExt, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
    } else if (mMutationOp == "sext") {
        return castInst->getOpcode() == Instruction::CastOps::ZExt ? CastInst::Create(Instruction::CastOps::SExt, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
    } else if (mMutationOp == "fptosi") {
        return castInst->getOpcode() == Instruction::CastOps::FPToUI ? CastInst::Create(Instruction::CastOps::FPToSI, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
    } else if (mMutationOp == "fptoui") {
        return castInst->getOpcode() == Instruction::CastOps::FPToSI ? CastInst::Create(Instruction::CastOps::FPToUI, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
        } else if (mMutationOp == "sitofp") {
        return castInst->getOpcode() == Instruction::CastOps::UIToFP ? CastInst::Create(Instruction::CastOps::SIToFP, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
    } else if (mMutationOp == "uitofp") {
        return castInst->getOpcode() == Instruction::CastOps::SIToFP ? CastInst::Create(Instruction::CastOps::UIToFP, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
    }
    return nullptr;
}
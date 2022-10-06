#include "SingleWindowPass.h"

#include <string>
#include <algorithm>
#include <fstream>
#include <memory>

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

SingleWindowPass::SingleWindowPass(uint32_t mutationLoc, std::string mutationOp)
    : mMutationLoc(mutationLoc),
      mMutationOp(mutationOp),
      mPredWindows(std::make_unique<std::vector<std::string>>())
{}

PreservedAnalyses SingleWindowPass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

void SingleWindowPass::write(Instruction* I, std::string& window)
{
    window += I->getOpcodeName();
    if (isa<CmpInst>(I)) {
        CmpInst* op = dyn_cast<CmpInst>(I);
        window += op->getPredicateName(op->getPredicate()).str();
    } else if (isa<BinaryOperator>(I)) {
        BinaryOperator* op = dyn_cast<BinaryOperator>(I);
        if (op->hasNoUnsignedWrap()) {
            window += "nuw";
        }
        if (op->hasNoSignedWrap()) {
            window += "nsw";
        }
        if (op->isExact()) {
            window += "exact";
        }
        if (op->isFast()) {
            window += "fast";
        }
    }
    window += " ";
}

void SingleWindowPass::insertBefore(inst_iterator instToWrite, inst_iterator startInst, std::string window, uint8_t depthleft)
{
    if (depthleft == 0) {
        mPredWindows->push_back(window);
        return;
    }

    if (depthleft == 5) {
        if (mMutationOp.find("swapfunccall") != std::string::npos) {
            window = "call ";
        } else if (mMutationOp != "removevoidcall") {
            window = mMutationOp + " ";
        }
    } else {
        write(&*instToWrite, window);
    }

    if (!window.empty()) {
        auto reverseIterator = window.rbegin() + 1;
        while (*reverseIterator != ' ' && reverseIterator != window.rend()) {
            ++reverseIterator;
        }
        std::rotate(window.rbegin(), reverseIterator, window.rend());
    }

    if (instToWrite == startInst) {
        mPredWindows->push_back(window);
        return;
    }

    inst_iterator prevIterator(instToWrite);
    --prevIterator;
    BasicBlock* parent = instToWrite->getParent();
    if (prevIterator->isTerminator() && depthleft != 1) {
        for (BasicBlock* pred: predecessors(parent)) {
            inst_iterator predIter(prevIterator);
            while (predIter->getParent() != pred && predIter != startInst) {
                --predIter;
            }
            // reached beginning, predecessor after current block
            if (predIter == startInst) {
                while (predIter->getParent() != pred && !predIter->isTerminator()) {
                    ++predIter;
                }
            }
            insertBefore(predIter, startInst, window, depthleft - 1);
        }
    } else {
        insertBefore(prevIterator, startInst, window, depthleft - 1);
    }
}

void SingleWindowPass::insertAfter(inst_iterator instToWrite, inst_iterator endInst, std::string window, uint8_t depthleft)
{
    if (instToWrite == endInst || depthleft == 0) {
        outs() << window << '\n';
        return;
    }

    write(&*instToWrite, window);

    if (instToWrite->getOpcode() == Instruction::Unreachable) {
        outs() << window << '\n';
        return;
    }

    BasicBlock* parent = instToWrite->getParent();
    inst_iterator nextIterator(instToWrite);
    ++nextIterator;
    if (instToWrite->isTerminator() && instToWrite->getNumSuccessors() != 0) {
        for (auto i = succ_begin(parent); i != succ_end(parent); ++i) {
            inst_iterator succIter(nextIterator);
            BasicBlock* succ = *i;
            while (succIter->getParent() != succ && succIter != endInst) {
                ++succIter;
            }
            // reached end, successor is before current basic block
            if (succIter == endInst) {
                while (succIter->getParent() != succ && !(--inst_iterator(succIter))->isTerminator()) {
                    --succIter;
                }
            }
            insertAfter(succIter, endInst, window, depthleft - 1);
        }
    } else {
        insertAfter(nextIterator, endInst, window, depthleft - 1);
    }
}

void SingleWindowPass::createWindows(inst_iterator instToWrite, inst_iterator startInst, inst_iterator endInst)
{
    mPredWindows->clear();
    insertBefore(instToWrite, startInst, "", 5);
    for (const auto& window: *mPredWindows) {
        insertAfter(++inst_iterator(instToWrite), endInst, window, 4);
    }
}

bool SingleWindowPass::runOnModule(Module& M)
{
    uint32_t instCnt = 0;
    for (auto& F: M) {
        if (F.isDeclaration())
            continue;

        for (inst_iterator i = inst_begin(F); i != inst_end(F); ++i) {
            if (instCnt == mMutationLoc) {
                createWindows(i, inst_begin(F), inst_end(F));
                return false;
            }
            ++instCnt;
        }
    }
    return false;
}
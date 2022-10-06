#include "WindowPass.h"

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

using namespace llvm;

WindowPass::WindowPass(std::string outputFile, std::string functionName)
    : mOutputStream(outputFile, std::ios::out),
      mFunctionName(functionName),
      mPredWindows(std::make_unique<std::vector<std::string>>())
{}

PreservedAnalyses WindowPass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

void WindowPass::write(Instruction* I, std::string& window)
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

void WindowPass::insertBefore(inst_iterator instToWrite, inst_iterator startInst, std::string window, uint8_t depthleft)
{
    if (depthleft == 0) {
        mPredWindows->push_back(window);
        return;
    }

    write(&*instToWrite, window);
    auto reverseIterator = window.rbegin() + 1;
    while (*reverseIterator != ' ' && reverseIterator != window.rend()) {
        ++reverseIterator;
    }
    std::rotate(window.rbegin(), reverseIterator, window.rend());

    if (instToWrite == startInst) {
        mPredWindows->push_back(window);
        return;
    }

    BasicBlock* parent = instToWrite->getParent();
    inst_iterator prevIterator(instToWrite);
    --prevIterator;
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

void WindowPass::insertAfter(inst_iterator instToWrite, inst_iterator endInst, std::string window, uint8_t depthleft)
{
    if (instToWrite == endInst || depthleft == 0) {
        mOutputStream << window << '\n';
        return;
    }

    write(&*instToWrite, window);

    if (instToWrite->getOpcode() == Instruction::Unreachable) {
        mOutputStream << window << '\n';
        return;
    }

    BasicBlock* parent = instToWrite->getParent();
    inst_iterator nextIterator(instToWrite);
    ++nextIterator;
    if (instToWrite->isTerminator() && instToWrite->getNumSuccessors() != 0) {
        for (BasicBlock* succ: successors(parent)) {
            inst_iterator succIter(nextIterator);
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

void WindowPass::createWindows(inst_iterator instToWrite, inst_iterator startInst, inst_iterator endInst)
{
    mPredWindows->clear();
    insertBefore(instToWrite, startInst, "", 5);
    for (const auto& window: *mPredWindows) {
        insertAfter(++inst_iterator(instToWrite), endInst, window, 4);
    }
}

bool WindowPass::runOnModule(Module& M)
{
    for (auto& F: M) {
        if (F.isDeclaration())
            continue;

        if (!F.getName().contains(mFunctionName))
            continue;

        for (inst_iterator i = inst_begin(F); i != inst_end(F); ++i) {
            createWindows(i, inst_begin(F), inst_end(F));
        }
    }
    return false;
}


#include "LabelPass.h"

#include <fstream>

#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;

LabelPass::LabelPass(std::string file)
{
    mStream.open(file, std::ios::out);
}

PreservedAnalyses LabelPass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LabelPass::runOnModule(Module& M)
{
    bool isModified = false;
    uint64_t instrCnt = 0;

    for (auto &F : M) {
        if (F.isDeclaration())
            continue;

        mStream << F.getName().str() << "\n";
        outs() << F.getName() << "\n";

        for (inst_iterator i = inst_begin(F); i != inst_end(F); ++i) {
            mStream << instrCnt << " " << i->getOpcodeName() << " ";
            outs() << instrCnt << " " << i->getOpcodeName() << " ";
            if (isa<CallInst>(&*i)) {
                CallInst* op = dyn_cast<CallInst>(&*i);
                if (!op->getCalledFunction()) {
                    mStream << "indirect " << op->getNumArgOperands() << '\n';
                    outs() << "indirect " << op->getNumArgOperands() << '\n';
                } else {
                    mStream << op->getCalledFunction()->getName().str() << ' ' << op->getNumArgOperands() << '\n';
                    outs() << op->getCalledFunction()->getName() << ' ' << op->getNumArgOperands() << '\n';
                }
            } else if (isa<GetElementPtrInst>(&*i)) {
                GetElementPtrInst* op = dyn_cast<GetElementPtrInst>(&*i);
                mStream << op->getNumIndices() << '\n';
                outs() << op->getNumIndices() << '\n';
            } else if (isa<CmpInst>(&*i)) {
                CmpInst* op = dyn_cast<CmpInst>(&*i);
                mStream << op->getPredicateName(op->getPredicate()).str() << '\n';
                outs() << op->getPredicateName(op->getPredicate()) << '\n';
            } else if (isa<BinaryOperator>(&*i)) {
                BinaryOperator* op = dyn_cast<BinaryOperator>(&*i);
                std::string flags = std::string();
                if (op->hasNoUnsignedWrap()) {
                    flags += "nuw ";
                }
                if (op->hasNoSignedWrap()) {
                    flags += "nsw ";
                }
                if (op->isExact()) {
                    flags += "exact ";
                }
                if (op->isFast()) {
                    flags += "fast ";
                }
                mStream << flags << '\n';
                outs() << flags << '\n';
            } else {
                mStream << '\n';
                outs() << '\n';
            }

            ++instrCnt;
        }
        mStream << "\n";
        outs() << "\n";
    }
    return isModified;
}


#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <sstream>
#include <string>
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      bool bModified = false;
      for (auto &B : F) {
        for (auto& I : B) {
       

        if (auto *op = dyn_cast<ICmpInst>(&I)) {
          // Insert at the point where the instruction `op` appears.
          IRBuilder<> builder(op);

          // Make a multiply with the same operands as `op`.

          // op->getOperand(0)->printAsOperand(errs());
          // errs() << "\n";
          // op->getOperand(1)->printAsOperand(errs());
          // errs() << "\n";
          errs() << *op << "\n";
          Value *lhs = op->getOperand(0);
          Value *rhs = op->getOperand(1);
          Value *inst = builder.CreateICmpEQ(lhs, rhs);

          // // Everywhere the old instruction was used as an operand, use our
          // // new Compare instruction instead.
          if (llvm::Constant* CI = dyn_cast<llvm::Constant>(op->getOperand(1))) {
            errs() << "entered constant op1 \n";
            if (CI->isNullValue()) {
              errs() << "entered null op1 \n";

              for (auto &U : op->uses()) {
                User *user = U.getUser();  // A User is anything with operands.
                user->setOperand(U.getOperandNo(), inst);
              }
              bModified |= true;
            }
          }
          }
        }
      }
      return bModified;
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);

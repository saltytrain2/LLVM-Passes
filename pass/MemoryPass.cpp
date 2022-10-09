#include "MemoryPass.h"

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

bool MemoryPass::runOnFunction(Function& F) {
      bool bModified = false;
      int counter = 0;
      for (auto &B : F) {
        for (auto& I : B) {
          if (auto *op = dyn_cast<LoadInst>(&I)) {
            //getAlign() Return the alignment of the memory that is being allocated by the instruction. More...
            // Insert at the point where the instruction `op` appears.
            IRBuilder<> builder(op);


            op->getOperand(0)->printAsOperand(errs());
            errs() << "\n";
            //errs() << op->getAlign().value() << "\n";
            //op->getOperand(1)->printAsOperand(errs());
            //errs() << "\n";
            //errs() << *op << "\n";
            Value *lhs = op->getOperand(0);
            //Value *rhs = op->getOperand(1);
            LLVMContext &context = F.getContext();
            Type* type = Type::getInt8Ty(context);
            Value *inst = builder.CreateLoad(type, lhs);
          
            for (auto &U : op->uses()) {
              User *user = U.getUser();  // A User is anything with operands.
              user->setOperand(U.getOperandNo(), inst);
            }
            bModified |= true;
        
            op->getOperand(0)->printAsOperand(errs());
            errs() << "\n";
            if(counter++ == 1){
              errs() << "Returned early" << "\n";
              return bModified;
            }
          }
        }
      }
      return bModified;
    }

    PreservedAnalyses MemoryPass::run(Function& F, FunctionAnalysisManager&)
    {
        return runOnFunction(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }
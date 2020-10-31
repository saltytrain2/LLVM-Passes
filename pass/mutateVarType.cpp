#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
        std::string svariable ("i");
        //ValueSymbolTable& symbolTable = F.getValueSymbolTable();
        Value* target = F.getValueSymbolTable()->lookup(svariable);
        AllocaInst* old_target = dyn_cast<AllocaInst>(target);
        errs() <<"old_target: " << *target << "\n";
        errs() <<"num of old_target_uses:" << old_target->getNumUses() <<"\n";

        //get the type of long double and construct new AllocaInst
        LLVMContext &context = F.getContext();
        Type* type = Type::getX86_FP80Ty(context);
        auto alignment = llvm::ConstantInt::get(llvm::Type::getInt64Ty(F.getContext()), 16);
        AllocaInst* new_target = new AllocaInst(type, 0, alignment, "new", old_target);
        new_target->takeName(old_target);

        // iterating through instructions using old AllocaInst
        Value::use_iterator it = old_target->use_begin();
        for(; it != old_target->use_end(); it++) {
            Value * temp = it->getUser();
            errs() <<"temp:" << *temp <<"\n";
            //transform() is under construction
            //transform(it, new_target, type, alignment);

        }
        old_target->eraseFromParent();
        return false;
}
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);

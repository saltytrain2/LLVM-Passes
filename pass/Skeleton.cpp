#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>
#include <string>
using namespace llvm;

static cl::opt<unsigned> MutationLocation("mutation_loc", cl::desc("Specify the instruction number that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<unsigned> MutationOperandLocation("mutation_op_loc", cl::desc("Specify the instruction number that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<unsigned> MutationOp("mutation_op", cl::desc("Specify operator to mutate with e.g., 8:add, 15:sub, 12:mul"), cl::value_desc("unsigned integer"));


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


namespace {
  struct MemoryPass : public FunctionPass {
    static char ID;
    MemoryPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      bool bModified = false;
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
          
        }
      }
    }
    return bModified;
  }
};
}

int instrCnt = 0;
namespace {
  struct LabelPass : public FunctionPass {
    static char ID;
    LabelPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      bool bModified = false;
      errs() << Instruction::Sub << "\n";
      for (auto &B : F) {
        for (auto& I : B) {
          instrCnt++;
          if (auto *op = dyn_cast<BinaryOperator>(&I)) { 
            errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";

          }
        
      }
    }
    return bModified;
  }
};
}


namespace {
  struct MutatePass : public FunctionPass {
    static char ID;
    MutatePass() : FunctionPass(ID) {}

    Instruction* getRequestedMutationBinaryOp(Instruction* I) {
      switch (MutationOp) {
        case Instruction::Add:
          return BinaryOperator::Create(Instruction::Add, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::Sub:
          return BinaryOperator::Create(Instruction::Sub, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::Mul:
          return BinaryOperator::Create(Instruction::Mul, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::UDiv:
          return BinaryOperator::Create(Instruction::UDiv, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::SDiv:
          return BinaryOperator::Create(Instruction::SDiv, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::URem:
          return BinaryOperator::Create(Instruction::URem, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::SRem:
          return BinaryOperator::Create(Instruction::SRem, I->getOperand(0), I->getOperand(1), "optimute");

        case Instruction::FAdd:
          return BinaryOperator::Create(Instruction::FAdd, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::FSub:
          return BinaryOperator::Create(Instruction::FSub, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::FMul:
          return BinaryOperator::Create(Instruction::FMul, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::FDiv:
          return BinaryOperator::Create(Instruction::FDiv, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::FRem:
          return BinaryOperator::Create(Instruction::FRem, I->getOperand(0), I->getOperand(1), "optimute");

        case Instruction::And:
          return BinaryOperator::Create(Instruction::And, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::Or:
          return BinaryOperator::Create(Instruction::Or, I->getOperand(0), I->getOperand(1), "optimute");
        case Instruction::Xor:
          return BinaryOperator::Create(Instruction::Xor, I->getOperand(0), I->getOperand(1), "optimute");
      }
    }
    virtual bool runOnFunction(Function &F) {
      bool bModified = false;
      
      for (auto &B : F) {
        for (BasicBlock::iterator DI = B.begin(); DI != B.end();) {
          Instruction *I = &*DI++;
          instrCnt++;
          if (auto *op = dyn_cast<BinaryOperator>(I)) { 
            errs()<< "instr #: " << (instrCnt) << " opcode: " << I->getOpcodeName() << "\n";

            if (instrCnt == MutationLocation) {
               errs() <<"modified: " <<instrCnt << "\n";
               Instruction* altI = getRequestedMutationBinaryOp(I);
               ReplaceInstWithInst(I, altI);
               bModified = true;
            }
          }
      }
    }
    return bModified;
  }
};
}


char SkeletonPass::ID = 0;
char MemoryPass::ID = 1;
char LabelPass::ID = 2;
char MutatePass::ID = 3;
static RegisterPass<MutatePass> X("mutatePass", "Apply Replacement Mutation");


// Automatically enable the pass.
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}

static void registerMemoryPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new MemoryPass());
}

static void registerLabelPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new LabelPass());
}

static void registerMutatePass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new MutatePass());
}


static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerMemoryPass);

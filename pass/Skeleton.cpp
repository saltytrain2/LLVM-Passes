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
#include <unordered_map>
#include <vector>
using namespace llvm;

static cl::opt<unsigned> MutationLocation("mutation_loc", cl::desc("Specify the instruction number that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> MutationOp("mutation_op", cl::desc("Specify operator to mutate with e.g., 8:add, 15:sub, 12:mul, swapFuncCall, swapFuncParam, funcConstParam"), cl::value_desc("String"));

static cl::opt<std::string> MutationVal("mutation_val", cl::desc("Specify value for constant mutation (Mutation_Op flag must be funcConstParam"), cl::value_desc("String"));

static cl::opt<unsigned> ParameterLocation("parameter_loc", cl::desc("Specify the parameter number in the function that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> FunctionName("function_name", cl::desc("Specify name of the function to swap to (Mutation_Op flag must be swapFuncCall"), cl::value_desc("String"));

std::unordered_map<std::string, Function*> stringToFunc;
std::vector<Instruction*> instToDelete;


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
};
}

int instrCnt = 0;
namespace {
  struct LabelPass : public FunctionPass {
    static char ID;
    LabelPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      bool bModified = false;
     
      for (auto &B : F) {
        
        for (auto& I : B) {
          errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";

          instrCnt++;
          // if (auto *op = dyn_cast<CallInst>(&I)) { 
          //   errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";
          // }
        
      }
    }
    return bModified;
  }
};
}


namespace {
  struct MutatePass : public ModulePass {
    static char ID;
    MutatePass() : ModulePass(ID) {}

    Instruction* getRequestSpecialOp(Instruction* I) {
      if(MutationOp == "loadint8"){
        auto *op = dyn_cast<LoadInst>(I);
        IRBuilder<> builder(op);
        LLVMContext &context = I->getFunction()->getContext();
        Type* type = Type::getInt8Ty(context); //Note this only should be done if the type is larger than 8. ie you mutate a 16 byte load to an 8 byte load.
        Instruction *inst = builder.CreateLoad(type, op->getOperand(0));
        
        return inst;
      }
      else if(MutationOp == "swapFuncParam"){ //TODO - update to work w/ arg list.
        auto *op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        Value *func = op->getCalledFunction();
        Value *arg1 = op->getArgOperand(0);
        Value *arg2 = op->getArgOperand(1);
        Value *newArgs[] = {arg2, arg1};
        Instruction *inst = builder.CreateCall(op->getCalledFunction(), newArgs);
        return inst;
      }
      else if(MutationOp == "swapFuncCall"){
        errs() << "Begin" << "\n";
        auto *op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        std::vector<Value*> argList;

        for(int i = 0; i < op->getCalledFunction()->arg_size(); ++i){
          argList.push_back(op->getOperand(i));
        }

        Instruction *inst = builder.CreateCall(stringToFunc[FunctionName], argList);
        //erase from parent here
        instToDelete.push_back(I);
        errs() << "End" << "\n";
        return inst;
      }
      // func(1,2,3); replace 3 with 4?
      else if(MutationOp == "funcConstParam"){
        auto *op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        std::vector<Value*> argList;
        

        for(int i = 0; i < op->getCalledFunction()->arg_size(); ++i){
          argList.push_back(op->getOperand(i));
        }
        //Mutate here
        // https://stackoverflow.com/questions/16246920/how-to-create-a-constantint-in-llvm
        LLVMContext &context = op->getCalledFunction()->getContext();
        llvm::Type *i32_type = llvm::IntegerType::getInt32Ty(context);
        llvm::Constant *i32_val = llvm::ConstantInt::get(i32_type, stoi(MutationVal) /*value*/, true);
        argList[ParameterLocation] = i32_val;

        builder.CreateCall(op->getCalledFunction(), argList);
        //erase from parent here
        instToDelete.push_back(I);

        //return inst;
      }
      return nullptr;
    }
    
    Instruction* getRequestedMutationBinaryOp(Instruction* I) {
      if(MutationOp == "add"){
        return BinaryOperator::Create(Instruction::Add, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "sub"){
        return BinaryOperator::Create(Instruction::Sub, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "mul"){
        return BinaryOperator::Create(Instruction::Mul, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "udiv"){
        return BinaryOperator::Create(Instruction::UDiv, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "sdiv"){
        return BinaryOperator::Create(Instruction::SDiv, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "urem"){
        return BinaryOperator::Create(Instruction::URem, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "srem"){
        return BinaryOperator::Create(Instruction::SRem, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "fadd"){
        return BinaryOperator::Create(Instruction::FAdd, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "fsub"){
        return BinaryOperator::Create(Instruction::FSub, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "fmul"){
        return BinaryOperator::Create(Instruction::FMul, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "fdiv"){
        return BinaryOperator::Create(Instruction::FDiv, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "frem"){
        return BinaryOperator::Create(Instruction::FRem, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "and"){
        return BinaryOperator::Create(Instruction::And, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "or"){
        return BinaryOperator::Create(Instruction::Or, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "xor"){
        return BinaryOperator::Create(Instruction::Xor, I->getOperand(0), I->getOperand(1), "optimute");
      }
      return nullptr; //User didn't specify a valid mutationop  
    }

    Instruction* getRequestedMutantIcmpInst(Instruction* I, ICmpInst* cmpInst) {
      if (MutationOp == "icmp_eq") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_EQ, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if (MutationOp == "icmp_ne"){
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_NE, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if (MutationOp == "icmp_ugt") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_UGT, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_uge") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_UGE, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_ult") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_ULT, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_ule") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_ULE, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_sgt") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_SGT, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_sge") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_SGE, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_slt") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_SLT, I->getOperand(0), I->getOperand(1), "optimute");
      }
      else if(MutationOp == "icmp_sle") {
        return CmpInst::Create(cmpInst->getOpcode(), CmpInst::ICMP_SLE, I->getOperand(0), I->getOperand(1), "optimute");
      }
      return nullptr;
      
    }

    virtual bool runOnModule(Module &M) {
      bool bModified = false;
      for (auto &F : M){
        stringToFunc[F.getName()] = &F;
        errs() << F.getName() << "\n";
      }

      for (auto &F: M) {
        
        for (auto &B : F) {
          for (BasicBlock::iterator DI = B.begin(); DI != B.end();) {
            Instruction *I = &*DI++;

            // if(isa<LoadInst>(*I)){
            //   MutationOp = "loadint8";
            //   Instruction* altI = getRequestSpecialOp(I);
            //   auto *op = dyn_cast<LoadInst>(I);

            //   for (auto &U : op->uses()) {
            //     User *user = U.getUser();  // A User is anything with operands.
            //     user->setOperand(U.getOperandNo(), altI);
            //   }
            //   //ReplaceInstWithInst(I, altI);
            // }
            
            if (instrCnt == MutationLocation) {
              errs() << "modified: " << instrCnt;
              if (isa<LoadInst>(*I)) {
                errs() << " Load Instruction Replaced" << "\n";
                Instruction* altI = getRequestSpecialOp(I);
                ReplaceInstWithInst(I, altI);
              }
              else if (ICmpInst *cmpInst = dyn_cast<ICmpInst>(I)) {
                errs() << " Comparison Instruction Replaced" << "\n";
                Instruction *altI = getRequestedMutantIcmpInst(I, cmpInst);
                ReplaceInstWithInst(I, altI);
              }
              else if (isa<BinaryOperator>(*I)) {
                errs() << " Binary Operator Replaced" << "\n";
                Instruction* altI = getRequestedMutationBinaryOp(I);
                ReplaceInstWithInst(I, altI);
              }
              else if(auto *op = dyn_cast<CallInst>(I)) {
                errs() << " Custom Mutation Applied" << "\n";
                Instruction* altI = getRequestSpecialOp(I);
                //ReplaceInstWithInst(I, altI);
                errs() << "Instruction Replaced" << "\n";
                
              }
            }
            instrCnt++;
            bModified = true; 
          }
        
        }
      }

      for (unsigned i = 0; i < instToDelete.size(); ++i) {
        errs() << "Deleted\n";
        instToDelete[i]->eraseFromParent();
      }
      return bModified;
    }
  };
}

// namespace {
//   struct AnvillLabelPass : public ModulePass {
//     static char ID;
//     AnvillLabelPass() : ModulePass(ID) {}

//     virtual bool runOnModule(Module &M) {
//       bool bModified = false;
     
//       for (auto &B : F) {
        
//         for (auto& I : B) {
//           errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";

//           instrCnt++;
//           // if (auto *op = dyn_cast<CallInst>(&I)) { 
//           //   errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";
//           // }
        
//       }
//     }
//     return bModified;
//   }
// };
// }

char SkeletonPass::ID = 0;
char MemoryPass::ID = 1;
char LabelPass::ID = 2;
char MutatePass::ID = 3;
// char AnvillRegisterPass::ID = 4;
static RegisterPass<MutatePass> X("mutatePass", "Apply Replacement Mutation");
static RegisterPass<LabelPass> Z("labelPass", "Print Labels");
static RegisterPass<MemoryPass> Z2("memoryPass", "Print Labels");
// static RegisterPass<AnvillRegisterPass> Z3("anvillRegisterPass", "Fix undefined reference to null registers");


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

static void registerAnvillRegisterPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new AnvillRegisterPass());
}


static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerLabelPass);

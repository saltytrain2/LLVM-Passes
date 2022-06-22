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

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
using namespace llvm;

static cl::opt<unsigned> MutationLocation("mutation_loc", cl::desc("Specify the instruction number that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> MutationOp("mutation_op", cl::desc("Specify operator to mutate with e.g., icmp_eq, swapFuncCall, swapFuncParam, funcConstParam"), cl::value_desc("String"));

static cl::opt<std::string> MutationVal("mutation_val", cl::desc("Specify value for constant mutation (Mutation_Op flag must be funcConstParam"), cl::value_desc("String"));

static cl::opt<unsigned> ParameterLocation("parameter_loc", cl::desc("Specify the parameter number in the function that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> FunctionName("function_name", cl::desc("Specify name of the function to swap to (Mutation_Op flag must be swapFuncCall"), cl::value_desc("String"));

static cl::opt<std::string> OutputFile("output_file", cl::desc("Output filename."), cl::value_desc("String"));

static cl::opt<std::string> SwapParamNums("swap_param_nums", cl::desc("Specify the position of the two parameters to swap (Mutation_op flag must be swapFuncParam, declared as 'num1 num2')"), cl::value_desc("String"));

static cl::opt<std::string> HashAlgorithm("hash_algorithm", cl::desc("Specify the hashing algorithm to implement (Mutation_op flag must be changeHash)"), cl::value_desc("String"));


std::unordered_map<std::string, Function*> stringToFunc;
std::vector<Instruction*> instToDelete;


namespace {
  struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
      bool Changed = runOnFunction(F);
      return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    bool runOnFunction(Function& F) {
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

    static bool isRequired() { return true; }
  };
}


namespace {
  struct MemoryPass : public PassInfoMixin<MemoryPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager &) {
      bool Changed = runOnFunction(F);
      return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    bool runOnFunction(Function& F) {
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
    static bool isRequired() { return true; }
  };
}

namespace {
  struct LabelPass : public PassInfoMixin<LabelPass> {
    std::ofstream mystream;

    PreservedAnalyses run(Module& M, ModuleAnalysisManager &) {
      mystream.open(OutputFile, std::ios::out);

      bool changed = runOnModule(M);
      return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    bool runOnModule(Module& M) {
      bool isModified = false;
      uint64_t instrCnt = 0;

      for (auto &F : M) {
        if (F.isDeclaration())
          continue;

        mystream << F.getName().str() << "\n";
        outs() << F.getName() << "\n";

        for (inst_iterator i = inst_begin(F); i != inst_end(F); ++i) {
          mystream << instrCnt << " " << i->getOpcodeName() << " ";
          outs() << instrCnt << " " << i->getOpcodeName() << " ";
          if (isa<CallInst>(&*i)) {
            CallInst* op = dyn_cast<CallInst>(&*i);
            if (op->getCalledFunction() != nullptr) {
              mystream << op->getCalledFunction()->getName().str() << ' ' << op->getNumArgOperands() << '\n';
              outs() << op->getCalledFunction()->getName() << ' ' << op->getNumArgOperands() << '\n';
            } else {
              mystream << "indirect " << op->getNumOperands() << '\n';
              outs() << "indirect " << op->getNumOperands() << '\n';
            }
          } else if (isa<GetElementPtrInst>(&*i)) {
            GetElementPtrInst* op = dyn_cast<GetElementPtrInst>(&*i);
            mystream << op->getNumIndices() << '\n';
            outs() << op->getNumIndices() << '\n';
          } else {
            mystream << '\n';
            outs() << '\n';
          }

          ++instrCnt;
        }
        mystream << "\n";
        outs() << "\n";
      }
      return isModified;
    }

    // bool runOnFunction(Function &F) {
    //   bool bModified = false;
     
    //   for (auto &B : F) {
        
    //     for (auto& I : B) {

	   
    //       this->mystream<< I.getOpcodeName() << " " << instrCnt << "\n"; //Stay as outs
    //      // outs()<< I.getOpcodeName() << " " << instrCnt << "\n"; //Stay as outs

	  

    //       instrCnt++;
    //       // if (auto *op = dyn_cast<CallInst>(&I)) { 
    //       //   errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";
    //       // }
        
    //     }
    //   }
    //   return bModified;
    // }
    static bool isRequired() { return true; }
  };
}


namespace {
  struct MutatePass : public PassInfoMixin<MutatePass> {
    PreservedAnalyses run(Module& M, ModuleAnalysisManager &) {
      bool Changed = runOnModule(M);
      return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    Instruction* getRequestCallOp(Instruction* I, Module& M) {
      if(MutationOp == "loadint8"){
        auto *op = dyn_cast<LoadInst>(I);
        IRBuilder<> builder(op);
        LLVMContext &context = I->getFunction()->getContext();
        Type* type = Type::getInt8Ty(context); //Note this only should be done if the type is larger than 8. ie you mutate a 16 byte load to an 8 byte load.
        Instruction *inst = builder.CreateLoad(type, op->getOperand(0));
        
        return inst;
      } else if(MutationOp == "swapFuncParam") { //TODO - make this function more robust for type-implicit conversions
        auto *op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        std::stringstream paramIndices(SwapParamNums); // populates stringstream with two indices values
        std::vector<Value*> newArgs;

        // don't attempt to swap llvm instrinsic functions
        if (op->getCalledFunction()->getName().startswith("llvm.")){
          return nullptr;
        }

        // extract the two indices
        int firstIndex, secondIndex;
        paramIndices >> firstIndex;
        paramIndices >> secondIndex;

        for (uint32_t i = 0; i < op->getNumArgOperands(); ++i) {
          newArgs.push_back(op->getArgOperand(i));
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
        Instruction *inst = builder.CreateCall(op->getCalledFunction(), newArgs);
        return inst;
      } else if(MutationOp == "swapFuncCall"){
        outs() << "Begin" << "\n";
        auto *op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        std::vector<Value*> argList;

        for(uint32_t i = 0; i < op->getCalledFunction()->arg_size(); ++i){
          argList.push_back(op->getOperand(i));
        }

        Instruction *inst = builder.CreateCall(stringToFunc[FunctionName], argList);
        //erase from parent here
        instToDelete.push_back(I);
        outs() << "End" << "\n";
        return inst;
      } else if(MutationOp == "funcConstParam"){
        auto *op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        std::vector<Value*> argList;
        

        for(uint32_t i = 0; i < op->getCalledFunction()->arg_size(); ++i){
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
      } else if (MutationOp == "changeHash") {
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
        }
        FunctionCallee EPV_hash = M.getOrInsertFunction(StringRef("EVP_" + HashAlgorithm), hashType);

        auto* op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);
        Instruction* inst = builder.CreateCall(EPV_hash);
        return inst;
      } else if (MutationOp == "removeVoidCall") {
        // eliminates a call to a void function, which changes state
        auto op = dyn_cast<CallInst>(I);
        IRBuilder<> builder(op);

        if (op->getCalledFunction()->getName().startswith("llvm.") || !op->getCalledFunction()->getReturnType()->isVoidTy()) {
          return nullptr;
        }

        Function* nop = Intrinsic::getDeclaration(&M, Intrinsic::donothing);
        auto inst = builder.CreateCall(nop);
        return inst;
      }

      return nullptr;
    }
    
    bool getRequestedBranchOp(Instruction* I) {
      if (MutationOp == "removeElseBlock") {
        auto* op = dyn_cast<BranchInst>(I);
        IRBuilder<> builder(op);
        // check to make sure the current branch inst is actually an if-else block
        if (!op->isConditional() || op->getNumSuccessors() != 2) {
          return false;
        }

        Value* condValue = op->getCondition();
        BasicBlock* thenBlock = op->getSuccessor(0);
        BasicBlock* elseBlock = op->getSuccessor(1);
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

    Value* getTypeCast(IRBuilder<>& builder, llvm::Value* firstParam, llvm::Value* secondParam) {
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

    Instruction* getRequestedMutantIcmpInst(Instruction* I) {
      ICmpInst* cmpInst = dyn_cast<ICmpInst>(I);
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

    Instruction* getRequestedMutantIntegerSignCastInst(Instruction* I) {
      CastInst* castInst = dyn_cast<CastInst>(I);
      IRBuilder<> builder(castInst);
      outs() << castInst->getNumOperands() << '\n';
      if (MutationOp == "zext") {
        if (castInst->getOpcode() == Instruction::SExt) {
          return dyn_cast<Instruction>(builder.CreateZExt(castInst->getOperand(0), castInst->getDestTy()));
        }
      } else if (MutationOp == "sext") {
        if (castInst->getOpcode() == Instruction::ZExt) {
          return dyn_cast<Instruction>(builder.CreateSExt(castInst->getOperand(0), castInst->getDestTy()));
        }
      } else if (MutationOp == "fptosi") {
        if (castInst->getOpcode() == Instruction::FPToUI) {
          return dyn_cast<Instruction>(builder.CreateFPToSI(castInst->getOperand(0), castInst->getDestTy()));
        }
      } else if (MutationOp == "fptoui") {
        if (castInst->getOpcode() == Instruction::FPToSI) {
          return dyn_cast<Instruction>(builder.CreateFPToUI(castInst->getOperand(0), castInst->getDestTy()));
        }
      } else if (MutationOp == "sitofp") {
        if (castInst->getOpcode() == Instruction::UIToFP) {
          return dyn_cast<Instruction>(builder.CreateSIToFP(castInst->getOperand(0), castInst->getDestTy()));
        }
      } else if (MutationOp == "uitofp") {
        if (castInst->getOpcode() == Instruction::SIToFP) {
          return dyn_cast<Instruction>(builder.CreateSIToFP(castInst->getOperand(0), castInst->getDestTy()));
        }
      }
      return nullptr;
    }

    Instruction* getRequestedMutantGEPInst(Instruction* I, Module& M)
    {
      // DataLayout moduleLayout(&M);
      // GetElementPtrInst* gepInst = dyn_cast<GetElementPtrInst>(I);
      // IRBuilder<> builder(gepInst);
      // // refers to last index of GEP
      // if (MutationOp == "incrementIndex") {
      //   std::vector<Value*> argsList;

      //   for (uint32_t i = 0; i < gepInst->getNumOperands(); ++i) {
      //     argsList.emplace_back(gepInst->getOperand(i));
      //   }

      //   Value* lastOperand = argsList.back();
      //   if (!isa<Constant>(lastOperand)) {
      //     Value* offset = builder.CreateAdd(lastOperand, ConstantInt::get(lastOperand->getType(), 1));
      //     argsList[argsList.size() - 1] = offset;
      //   } else {
      //     argsList[argsList.size() - 1] = lastOperand + ConstantInt::get(lastOperand->getType(), 1);
      //   }

      //   return dyn_cast<Instruction>(builder.CreateInBoundsGEP(gepInst->getType(), ))
      // }
      return nullptr;
    }

    bool runOnModule(Module &M) {
      bool bModified = false;
      uint64_t instrCnt = 0;

      for (auto& F: M) {
        if (F.isDeclaration()) 
          continue;

        for (inst_iterator i = inst_begin(F); i != inst_end(F) && !bModified; ++i) {
          if (instrCnt == MutationLocation) {
            Instruction* I = &*i;
            Instruction* altI = nullptr;

            if (MutationOp == "null") {
              outs() << "nothing modified";
              return false;
            }

            if (isa<LoadInst>(I)) {
              outs() << "Replacing Load Instruction\n";
              altI = getRequestCallOp(I, M);
            } else if (isa<ICmpInst>(I)) {
              outs() << "Replacing Comparison Instruction\n";
              altI = getRequestedMutantIcmpInst(I);
            } else if (isa<BinaryOperator>(I)) {
              outs() << "Replacing Binary Instruction\n";
              altI = getRequestedMutationBinaryOp(I);
            } else if (isa<CallInst>(I)) {
              outs() << "Replacing Call Instruction\n";
              altI = getRequestCallOp(I, M);
            } else if (isa<BranchInst>(I)) {
              outs() << "Replacing Branch Instruction\n";
              if (!getRequestedBranchOp(I)) {
                outs() << "no else block detected\n";
              } else {
                outs() << "else block successfully eliminated\n";
                bModified = true;
              }
              break;
            } else if (isa<CastInst>(I)) {
              outs() << "Replacing Cast Instruction\n";
              altI = getRequestedMutantIntegerSignCastInst(I);
            } else if (isa<GetElementPtrInst>(I)) {
              outs() << "Replacing GetElementPtr Instruction";
              altI = getRequestedMutantGEPInst(I, M);
            }

            if (altI == nullptr) {
              outs() << "nothing modified";
              return bModified;
            }

            ReplaceInstWithInst(I, altI);
            bModified = true;
          }
          ++instrCnt;
        }
      }

      for (unsigned i = 0; i < instToDelete.size(); ++i) {
        outs() << "Deleted\n";
        instToDelete[i]->eraseFromParent();
      }

      for (auto &F : M) {
        stringToFunc[std::string(F.getName())] = &F;
        outs() << F.getName() << "\n";
      }
      return bModified;
    }

    static bool isRequired() { return true; }
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

llvm::PassPluginLibraryInfo getSkeletonPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "skeleton-pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "skeleton-pass") {
                    FPM.addPass(SkeletonPass());
                    return true;
                  } else if (Name == "memory-pass") {
                    FPM.addPass(MemoryPass());
                    return true;
                  }
                  return false;
                });
            PB.registerPipelineParsingCallback(
              [](StringRef Name, ModulePassManager& MPM,
                 ArrayRef<PassBuilder::PipelineElement>) {
                   if (Name == "mutate-pass") {
                     MPM.addPass(MutatePass());
                     return true;
                   } else if (Name == "label-pass") {
                     MPM.addPass(LabelPass());
                     return true;
                   }
                   return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getSkeletonPassPluginInfo();
}
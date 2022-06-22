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

    Instruction* getRequestedLoadOp(Instruction* I) {
        if (MutationOp == "loadint8") {
            auto *op = dyn_cast<LoadInst>(I);
            IRBuilder<> builder(op);
            LLVMContext &context = I->getFunction()->getContext();
            Type* type = Type::getInt8Ty(context); //Note this only should be done if the type is larger than 8. ie you mutate a 16 byte load to an 8 byte load.

            return dyn_cast<Instruction>(builder.CreateLoad(type, op->getOperand(0)));
        }
        return nullptr;
    }

    Instruction* getRequestedCallOp(Instruction* I, Module& M) {
        CallInst* callInst = dyn_cast<CallInst>(I);
        IRBuilder<> builder(callInst);

        if (MutationOp == "swapFuncParam") { //TODO - make this function more robust for type-implicit conversions
            std::stringstream paramIndices(SwapParamNums); // populates stringstream with two indices values
            std::vector<Value*> newArgs;

            // don't attempt to swap llvm instrinsic functions
            if (callInst->getCalledFunction()->getName().startswith("llvm.")){
                return nullptr;
            }

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
            return dyn_cast<Instruction>(builder.CreateCall(callInst->getCalledFunction(), newArgs));
        } else if (MutationOp == "swapFuncCall") {
            outs() << "Begin" << "\n";
            std::vector<Value*> argList;

            for(uint32_t i = 0; i < callInst->getCalledFunction()->arg_size(); ++i){
                argList.push_back(callInst->getOperand(i));
            }

            Instruction *inst = builder.CreateCall(stringToFunc[FunctionName], argList);
            //erase from parent here
            instToDelete.push_back(I);
            outs() << "End" << "\n";
            return inst;
        } else if (MutationOp == "funcConstParam") {
            std::vector<Value*> argList;
        

            for (uint32_t i = 0; i < callInst->getCalledFunction()->arg_size(); ++i){
                argList.push_back(callInst->getOperand(i));
            }
            //Mutate here
            // https://stackoverflow.com/questions/16246920/how-to-create-a-constantint-in-llvm
            LLVMContext &context = callInst->getCalledFunction()->getContext();
            llvm::Type *i32_type = llvm::IntegerType::getInt32Ty(context);
            llvm::Constant *i32_val = llvm::ConstantInt::get(i32_type, stoi(MutationVal) /*value*/, true);
            argList[ParameterLocation] = i32_val;

            builder.CreateCall(callInst->getCalledFunction(), argList);
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

            return dyn_cast<Instruction>(builder.CreateCall(EPV_hash));
        } else if (MutationOp == "removeVoidCall") {
            // eliminates a call to a void function, which changes state
            if (callInst->getCalledFunction()->getName().startswith("llvm.") || !callInst->getCalledFunction()->getReturnType()->isVoidTy()) {
                return nullptr;
            }

            Function* nop = Intrinsic::getDeclaration(&M, Intrinsic::donothing);
            return dyn_cast<Instruction>(builder.CreateCall(nop));
        }

        return nullptr;
    }
    
    bool getRequestedBranchOp(Instruction* I) {
        BranchInst* branchInst = dyn_cast<BranchInst>(I);
        IRBuilder<> builder(branchInst);

        if (MutationOp == "removeElseBlock") {
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

    // encompases bitwise shifts, bitwise logical operations, and mathematical operations
    Instruction* getRequestedMutationBinaryOp(Instruction* I) {
        BinaryOperator* binop = dyn_cast<BinaryOperator>(I);
        IRBuilder<> builder(binop);

        if (MutationOp == "add"){
            return dyn_cast<Instruction>(builder.CreateAdd(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "sub") {
            return dyn_cast<Instruction>(builder.CreateSub(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "mul") {
            return dyn_cast<Instruction>(builder.CreateMul(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "udiv") {
            return dyn_cast<Instruction>(builder.CreateUDiv(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "sdiv") {
            return dyn_cast<Instruction>(builder.CreateSDiv(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "urem") {
            return dyn_cast<Instruction>(builder.CreateURem(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "srem") {
            return dyn_cast<Instruction>(builder.CreateSRem(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "fadd") {
            return dyn_cast<Instruction>(builder.CreateFAdd(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "fsub") {
            return dyn_cast<Instruction>(builder.CreateFSub(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "fmul") {
            return dyn_cast<Instruction>(builder.CreateFMul(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "fdiv") {
            return dyn_cast<Instruction>(builder.CreateFDiv(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "frem") {
            return dyn_cast<Instruction>(builder.CreateFRem(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "and") {
            return dyn_cast<Instruction>(builder.CreateAnd(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "or") {
            return dyn_cast<Instruction>(builder.CreateOr(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "xor") {
            return dyn_cast<Instruction>(builder.CreateXor(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "shl") {
            return dyn_cast<Instruction>(builder.CreateShl(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "lshr") {
            return dyn_cast<Instruction>(builder.CreateLShr(binop->getOperand(0), binop->getOperand(1)));
        } else if (MutationOp == "ashr") {
            return dyn_cast<Instruction>(builder.CreateAShr(binop->getOperand(0), binop->getOperand(1)));
        }
        return nullptr; //User didn't specify a valid mutationop  
    }

    Instruction* getRequestedMutantIcmpInst(Instruction* I) {
        ICmpInst* cmpInst = dyn_cast<ICmpInst>(I);
        IRBuilder<> builder(cmpInst);

        if (MutationOp == "icmp_eq") {
            return dyn_cast<Instruction>(builder.CreateICmpEQ(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "icmp_ne") {
            return dyn_cast<Instruction>(builder.CreateICmpNE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "icmp_ugt") {
            return dyn_cast<Instruction>(builder.CreateICmpUGT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_uge") {
            return dyn_cast<Instruction>(builder.CreateICmpUGE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_ult") {
            return dyn_cast<Instruction>(builder.CreateICmpULT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_ule") {
            return dyn_cast<Instruction>(builder.CreateICmpULE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_sgt") {
            return dyn_cast<Instruction>(builder.CreateICmpSGT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_sge") {
            return dyn_cast<Instruction>(builder.CreateICmpSGE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_slt") {
            return dyn_cast<Instruction>(builder.CreateICmpSLT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if(MutationOp == "icmp_sle") {
            return dyn_cast<Instruction>(builder.CreateICmpSLE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        }

        return nullptr;
    }

    Instruction* getRequestedMutantFcmpInst(Instruction* I) {
        FCmpInst* cmpInst = dyn_cast<FCmpInst>(I);
        IRBuilder<> builder(cmpInst);

        if (MutationOp == "fcmp_ueq") {
            return dyn_cast<Instruction>(builder.CreateFCmpUEQ(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_une") {
            return dyn_cast<Instruction>(builder.CreateFCmpUNE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_ugt") {
            return dyn_cast<Instruction>(builder.CreateFCmpUGT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_uge") {
            return dyn_cast<Instruction>(builder.CreateFCmpUGE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_ult") {
            return dyn_cast<Instruction>(builder.CreateFCmpULT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_ule") {
            return dyn_cast<Instruction>(builder.CreateFCmpULE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_oeq") {
            return dyn_cast<Instruction>(builder.CreateFCmpOEQ(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_one") {
            return dyn_cast<Instruction>(builder.CreateFCmpONE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_ogt") {
            return dyn_cast<Instruction>(builder.CreateFCmpOGT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_oge") {
            return dyn_cast<Instruction>(builder.CreateFCmpOGE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_olt") {
            return dyn_cast<Instruction>(builder.CreateFCmpOLT(cmpInst->getOperand(0), cmpInst->getOperand(1)));
        } else if (MutationOp == "fcmp_ole") {
            return dyn_cast<Instruction>(builder.CreateFCmpOLE(cmpInst->getOperand(0), cmpInst->getOperand(1)));
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
                        altI = getRequestedLoadOp(I, M);
                    } else if (isa<ICmpInst>(I)) {
                        outs() << "Replacing Immediate Comparison Instruction\n";
                        altI = getRequestedMutantIcmpInst(I);
                    } else if (isa<FCmpInst>(I)) {
                        outs() << "Replacing Floating-point Comparison Instruction\n";
                        altI = getRequestedMutantFcmpInst(I);
                    } else if (isa<BinaryOperator>(I)) {
                        outs() << "Replacing Binary Instruction\n";
                        altI = getRequestedMutationBinaryOp(I);
                    } else if (isa<CallInst>(I)) {
                        outs() << "Replacing Call Instruction\n";
                        altI = getRequestedCallOp(I, M);
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
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/CommandLine.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

#include "SkeletonPass.h"
#include "MemoryPass.h"
#include "LabelPass.h"
#include "MutatePass.h"
#include "RegisterInfoPass.h"
#include "RegisterExitPass.h"

using namespace llvm;

static cl::opt<uint32_t> MutationLocation("mutation_loc", cl::desc("Specify the instruction number that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> MutationOp("mutation_op", cl::desc("Specify operator to mutate with e.g., icmp_eq, swapFuncCall, swapFuncParam, funcConstParam"), cl::value_desc("String"));

static cl::opt<std::string> MutationVal("mutation_val", cl::desc("Specify value for constant mutation (Mutation_Op flag must be funcConstParam"), cl::value_desc("String"));

static cl::opt<uint32_t> ParameterLocation("parameter_loc", cl::desc("Specify the parameter number in the function that you would like to mutate"), cl::value_desc("unsigned integer"));

static cl::opt<std::string> FunctionName("function_name", cl::desc("Specify name of the function to swap to (Mutation_Op flag must be swapFuncCall"), cl::value_desc("String"));

static cl::opt<std::string> OutputFile("output_file", cl::desc("Output filename."), cl::value_desc("String"));

static cl::opt<std::string> SwapParamNums("swap_param_nums", cl::desc("Specify the position of the two parameters to swap (Mutation_op flag must be swapFuncParam, declared as 'num1 num2')"), cl::value_desc("String"));

static cl::opt<std::string> HashAlgorithm("hash_algorithm", cl::desc("Specify the hashing algorithm to implement (Mutation_op flag must be changeHash)"), cl::value_desc("String"));

// std::unordered_map<std::string, Function*> stringToFunc;
// std::vector<Instruction*> instToDelete;


// namespace {
//   struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
//     PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
//       bool Changed = runOnFunction(F);
//       return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
//     }

//     bool runOnFunction(Function& F) {
//       bool bModified = false;
//       for (auto &B : F) {
//         for (auto& I : B) {

//           if (auto *op = dyn_cast<ICmpInst>(&I)) {
//             // Insert at the point where the instruction `op` appears.
//             IRBuilder<> builder(op);

//             // op->getOperand(0)->printAsOperand(errs());
//             // errs() << "\n";
//             // op->getOperand(1)->printAsOperand(errs());
//             // errs() << "\n";
//             errs() << *op << "\n";
//             Value *lhs = op->getOperand(0);
//             Value *rhs = op->getOperand(1);
//             Value *inst = builder.CreateICmpEQ(lhs, rhs);

//             // // Everywhere the old instruction was used as an operand, use our
//             // // new Compare instruction instead.
//             if (llvm::Constant* CI = dyn_cast<llvm::Constant>(op->getOperand(1))) {
//               errs() << "entered constant op1 \n";
//               if (CI->isNullValue()) {
//                 errs() << "entered null op1 \n";

//                 for (auto &U : op->uses()) {
//                   User *user = U.getUser();  // A User is anything with operands.
//                   user->setOperand(U.getOperandNo(), inst);
//                 }
//                 bModified |= true;
//               }
//             }
//           }
//         }
//       }
//       return bModified;
//     }

//     static bool isRequired() { return true; }
//   };
// }


// namespace {
//   struct MemoryPass : public PassInfoMixin<MemoryPass> {
//     PreservedAnalyses run(Function& F, FunctionAnalysisManager &) {
//       bool Changed = runOnFunction(F);
//       return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
//     }

//     bool runOnFunction(Function& F) {
//       bool bModified = false;
//       int counter = 0;
//       for (auto &B : F) {
//         for (auto& I : B) {
//           if (auto *op = dyn_cast<LoadInst>(&I)) {
//             //getAlign() Return the alignment of the memory that is being allocated by the instruction. More...
//             // Insert at the point where the instruction `op` appears.
//             IRBuilder<> builder(op);


//             op->getOperand(0)->printAsOperand(errs());
//             errs() << "\n";
//             //errs() << op->getAlign().value() << "\n";
//             //op->getOperand(1)->printAsOperand(errs());
//             //errs() << "\n";
//             //errs() << *op << "\n";
//             Value *lhs = op->getOperand(0);
//             //Value *rhs = op->getOperand(1);
//             LLVMContext &context = F.getContext();
//             Type* type = Type::getInt8Ty(context);
//             Value *inst = builder.CreateLoad(type, lhs);
          
//             for (auto &U : op->uses()) {
//               User *user = U.getUser();  // A User is anything with operands.
//               user->setOperand(U.getOperandNo(), inst);
//             }
//             bModified |= true;
        
//             op->getOperand(0)->printAsOperand(errs());
//             errs() << "\n";
//             if(counter++ == 1){
//               errs() << "Returned early" << "\n";
//               return bModified;
//             }
//           }
//         }
//       }
//       return bModified;
//     }
//     static bool isRequired() { return true; }
//   };
// }

// namespace {
// struct LabelPass : public PassInfoMixin<LabelPass> {
//     std::ofstream mystream;

//     PreservedAnalyses run(Module& M, ModuleAnalysisManager &) {
//         mystream.open(OutputFile, std::ios::out);

//         bool changed = runOnModule(M);
//         return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
//     }

//     bool runOnModule(Module& M) {
//         bool isModified = false;
//         uint64_t instrCnt = 0;

//         for (auto &F : M) {
//             if (F.isDeclaration())
//                 continue;

//             mystream << F.getName().str() << "\n";
//             outs() << F.getName() << "\n";

//             for (inst_iterator i = inst_begin(F); i != inst_end(F); ++i) {
//                 mystream << instrCnt << " " << i->getOpcodeName() << " ";
//                 outs() << instrCnt << " " << i->getOpcodeName() << " ";
//                 if (isa<CallInst>(&*i)) {
//                     CallInst* op = dyn_cast<CallInst>(&*i);
//                     if (!op->getCalledFunction()) {
//                         mystream << "indirect " << op->getNumArgOperands() << '\n';
//                         outs() << "indirect " << op->getNumArgOperands() << '\n';
//                     } else {
//                         mystream << op->getCalledFunction()->getName().str() << ' ' << op->getNumArgOperands() << '\n';
//                         outs() << op->getCalledFunction()->getName() << ' ' << op->getNumArgOperands() << '\n';
//                     }
//                 } else if (isa<GetElementPtrInst>(&*i)) {
//                     GetElementPtrInst* op = dyn_cast<GetElementPtrInst>(&*i);
//                     mystream << op->getNumIndices() << '\n';
//                     outs() << op->getNumIndices() << '\n';
//                 } else if (isa<CmpInst>(&*i)) {
//                     CmpInst* op = dyn_cast<CmpInst>(&*i);
//                     mystream << op->getPredicateName(op->getPredicate()).str() << '\n';
//                     outs() << op->getPredicateName(op->getPredicate()) << '\n';
//                 } else if (isa<BinaryOperator>(&*i)) {
//                     BinaryOperator* op = dyn_cast<BinaryOperator>(&*i);
//                     std::string flags = std::string();
//                     if (op->hasNoUnsignedWrap()) {
//                         flags += "nuw ";
//                     }
//                     if (op->hasNoSignedWrap()) {
//                         flags += "nsw ";
//                     }
//                     if (op->isExact()) {
//                         flags += "exact ";
//                     }
//                     if (op->isFast()) {
//                         flags += "fast ";
//                     }
//                     mystream << flags << '\n';
//                     outs() << flags << '\n';
//                 } else {
//                     mystream << '\n';
//                     outs() << '\n';
//                 }

//                 ++instrCnt;
//             }
//             mystream << "\n";
//             outs() << "\n";
//         }
//         return isModified;
//     }

//     // bool runOnFunction(Function &F) {
//     //   bool bModified = false;
     
//     //   for (auto &B : F) {
        
//     //     for (auto& I : B) {

	   
//     //       this->mystream<< I.getOpcodeName() << " " << instrCnt << "\n"; //Stay as outs
//     //      // outs()<< I.getOpcodeName() << " " << instrCnt << "\n"; //Stay as outs

	  

//     //       instrCnt++;
//     //       // if (auto *op = dyn_cast<CallInst>(&I)) { 
//     //       //   errs()<< "instr #: " << (instrCnt) << " opcode: " << I.getOpcodeName() << "\n";
//     //       // }
        
//     //     }
//     //   }
//     //   return bModified;
//     // }
//     static bool isRequired() { return true; }
//   };
// }


// namespace {
// struct MutatePass : public PassInfoMixin<MutatePass> {
//     PreservedAnalyses run(Module& M, ModuleAnalysisManager &) {
//         bool Changed = runOnModule(M);
//         return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
//     }

//     Instruction* getRequestedLoadOp(Instruction* I) {
//         if (MutationOp == "loadint8") {
//             auto *op = dyn_cast<LoadInst>(I);
//             IRBuilder<> builder(op);
//             LLVMContext &context = I->getFunction()->getContext();
//             Type* type = Type::getInt8Ty(context); //Note this only should be done if the type is larger than 8. ie you mutate a 16 byte load to an 8 byte load.

//             return dyn_cast<Instruction>(builder.CreateLoad(type, op->getOperand(0)));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedCallOp(Instruction* I, Module& M) {
//         CallInst* callInst = dyn_cast<CallInst>(I);
//         IRBuilder<> builder(callInst);
//         // dont attempt to do any modifications with llvm intrinsic functions
//         if (callInst->getCalledFunction() && callInst->getCalledFunction()->getName().startswith("llvm.")) {
//             return nullptr;
//         }

//         if (MutationOp == "swapFuncParam") { //TODO - make this function more robust for type-implicit conversions
//             std::stringstream paramIndices(SwapParamNums); // populates stringstream with two indices values
//             std::vector<Value*> newArgs;

//             // extract the two indices
//             int firstIndex, secondIndex;
//             paramIndices >> firstIndex;
//             paramIndices >> secondIndex;

//             for (uint32_t i = 0; i < callInst->getNumArgOperands(); ++i) {
//                 newArgs.push_back(callInst->getArgOperand(i));
//             }

//             llvm::Value* firstParam = newArgs[firstIndex];
//             llvm::Value* secondParam = newArgs[secondIndex];

//             if (firstParam->getType() == secondParam->getType()) {
//                 std::swap(newArgs[firstIndex], newArgs[secondIndex]);
//             } else {
//                 auto firstCast = getTypeCast(builder, firstParam, secondParam);
//                 // if a type conversion is not supported, do not replace any instructions
//                 if (firstCast == nullptr) {
//                     return nullptr;
//                 }
//                 auto secondCast = getTypeCast(builder, secondParam, firstParam);
//                 std::swap(newArgs[firstIndex], secondCast);
//                 std::swap(newArgs[secondIndex], firstCast);
//             } 
//             return CallInst::Create(callInst->getFunctionType(), callInst->getCalledOperand(), newArgs);
//         } else if (MutationOp == "swapFuncCall") {
//             outs() << "Begin" << "\n";
//             std::vector<Value*> argList;

//             for(uint32_t i = 0; i < callInst->getCalledFunction()->arg_size(); ++i){
//                 argList.push_back(callInst->getOperand(i));
//             }

//             Instruction *inst = builder.CreateCall(stringToFunc[FunctionName], argList);
//             //erase from parent here
//             instToDelete.push_back(I);
//             outs() << "End" << "\n";
//             return inst;
//         } else if (MutationOp == "funcConstParam") {
//             std::vector<Value*> argList;
        

//             for (uint32_t i = 0; i < callInst->getCalledFunction()->arg_size(); ++i){
//                 argList.push_back(callInst->getOperand(i));
//             }
//             //Mutate here
//             // https://stackoverflow.com/questions/16246920/how-to-create-a-constantint-in-llvm
//             LLVMContext &context = callInst->getCalledFunction()->getContext();
//             llvm::Type *i32_type = llvm::IntegerType::getInt32Ty(context);
//             llvm::Constant *i32_val = llvm::ConstantInt::get(i32_type, stoi(MutationVal) /*value*/, true);
//             argList[ParameterLocation] = i32_val;

//             builder.CreateCall(callInst->getCalledFunction(), argList);
//             //erase from parent here
//             instToDelete.push_back(I);

//             //return inst;
//         } else if (MutationOp == "changeHash") {
//             // assumes the user is using the EPV_DIGEST interface of openssl
//             // Inject a call to EVP_md5
//             auto& ctx = M.getContext();
//             auto structTypes = M.getIdentifiedStructTypes();
//             bool foundType = false;
//             FunctionType* hashType = nullptr;

//             for (auto i: structTypes) {
//                 if (i->getName() == StringRef("struct.evp_md_st")) {
//                     foundType = true;
//                     hashType = FunctionType::get(PointerType::getUnqual(i), false);
//                     break;
//                 }
//             }

//             if (!foundType) {
//                 hashType = FunctionType::get(PointerType::getUnqual(StructType::create(ctx, StringRef("struct.epv_md_st"))), false);
//                 return nullptr;
//             }
//             FunctionCallee EPV_hash = M.getOrInsertFunction(StringRef("EVP_" + HashAlgorithm), hashType);

//             return dyn_cast<Instruction>(builder.CreateCall(EPV_hash));
//         } else if (MutationOp == "removeVoidCall") {
//             // eliminates a call to a void function, which changes state
//             if (callInst->getCalledFunction() && !callInst->getCalledFunction()->getReturnType()->isVoidTy()) {
//                 return nullptr;
//             }

//             Function* nop = Intrinsic::getDeclaration(&M, Intrinsic::donothing);
//             return CallInst::Create(nop->getFunctionType(), nop, NoneType::None);
//         }

//         return nullptr;
//     }
    
//     bool getRequestedBranchOp(Instruction* I) {
//         BranchInst* branchInst = dyn_cast<BranchInst>(I);
//         IRBuilder<> builder(branchInst);

//         if (MutationOp == "removeElseBlock") {
//             // check to make sure the current branch inst is actually an if-else block
//             if (!branchInst->isConditional() || branchInst->getNumSuccessors() != 2) {
//                 return false;
//             }

//             Value* condValue = branchInst->getCondition();
//             BasicBlock* thenBlock = branchInst->getSuccessor(0);
//             BasicBlock* elseBlock = branchInst->getSuccessor(1);
//             BasicBlock* thenBlockSuccessor = thenBlock->getUniqueSuccessor();
//             BasicBlock* elseBlockSuccessor = elseBlock->getUniqueSuccessor();
//             BasicBlock* thenBlockPredecessor = thenBlock->getUniquePredecessor();
//             BasicBlock* elseBlockPredecessor = elseBlock->getUniquePredecessor();

//             // check to make sure the if-else block has no nested blocks
//             if (thenBlockSuccessor != elseBlockSuccessor || thenBlockPredecessor != elseBlockPredecessor ||
//                 thenBlockSuccessor == nullptr || thenBlockPredecessor == nullptr) {
//                 return false;
//             }

//             builder.CreateCondBr(condValue, thenBlock, thenBlockSuccessor);
//             I->eraseFromParent();
//             DeleteDeadBlock(elseBlock);
//             return true;
//         }
//         return false;
//     }

//     Value* getTypeCast(IRBuilder<>& builder, llvm::Value* firstParam, llvm::Value* secondParam) {
//         // begin a series of statements to determine the types of the parameters to generate the correct IRBuilder cast call
//         // TODO, all references to integers are assumed to be unsigned, have to distinguish since llvm makes no explicit distinctions between signed and unsigned ints
//         if (firstParam->getType()->isIntegerTy() && secondParam->getType()->isFloatingPointTy()) {
//             return builder.CreateUIToFP(firstParam, secondParam->getType());
//         } else if (firstParam->getType()->isFloatingPointTy() && secondParam->getType()->isIntegerTy()) {
//             return builder.CreateFPToUI(firstParam, secondParam->getType());
//         } else if (firstParam->getType()->isPointerTy() && secondParam->getType()->isIntegerTy()) {
//             return builder.CreatePtrToInt(firstParam, secondParam->getType());
//         } else if (firstParam->getType()->isIntegerTy() && secondParam->getType()->isPointerTy()) {
//             return builder.CreateIntToPtr(firstParam, secondParam->getType());
//         } else if (firstParam->getType()->isIntegerTy() && secondParam->getType()->isIntegerTy()) {
//             return builder.CreateSExtOrTrunc(firstParam, secondParam->getType());
//         } else if (firstParam->getType()->isPointerTy() && secondParam->getType()->isPointerTy()) {
//             return builder.CreateBitOrPointerCast(firstParam, secondParam->getType());
//         } else {
//             return nullptr;
//         }
//     }

//     Instruction* getRequestedMutantBinaryIntegerOp(Instruction* binop) {
//         if (MutationOp == "add"){
//             return binop->getOpcode() == Instruction::BinaryOps::Add ? nullptr : BinaryOperator::CreateAdd(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "sub") {
//             return binop->getOpcode() == Instruction::BinaryOps::Sub ? nullptr : BinaryOperator::CreateSub(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "mul") {
//             return binop->getOpcode() == Instruction::BinaryOps::Mul ? nullptr : BinaryOperator::CreateMul(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "udiv") {
//             return binop->getOpcode() == Instruction::BinaryOps::UDiv ? nullptr : BinaryOperator::CreateUDiv(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "sdiv") {
//             return binop->getOpcode() == Instruction::BinaryOps::SDiv ? nullptr : BinaryOperator::CreateSDiv(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "urem") {
//             return binop->getOpcode() == Instruction::BinaryOps::URem ? nullptr : BinaryOperator::CreateURem(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "srem") {
//             return binop->getOpcode() == Instruction::BinaryOps::SRem ? nullptr : BinaryOperator::CreateSRem(binop->getOperand(0), binop->getOperand(1));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantBinaryFloatingOp(Instruction* binop) {
//         if (MutationOp == "fadd") {
//             return binop->getOpcode() == Instruction::BinaryOps::FAdd ? nullptr : BinaryOperator::CreateFAdd(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "fsub") {
//             return binop->getOpcode() == Instruction::BinaryOps::FSub ? nullptr : BinaryOperator::CreateFSub(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "fmul") {
//             return binop->getOpcode() == Instruction::BinaryOps::FMul ? nullptr : BinaryOperator::CreateFMul(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "fdiv") {
//             return binop->getOpcode() == Instruction::BinaryOps::FDiv ? nullptr :  BinaryOperator::CreateFDiv(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "frem") {
//             return binop->getOpcode() == Instruction::BinaryOps::FRem ? nullptr : BinaryOperator::CreateFRem(binop->getOperand(0), binop->getOperand(1));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantBinaryLogicalOp(Instruction* binop) {
//         if (MutationOp == "and") {
//             return binop->getOpcode() == Instruction::BinaryOps::And ? nullptr : BinaryOperator::CreateAnd(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "or") {
//             return binop->getOpcode() == Instruction::BinaryOps::Or ? nullptr : BinaryOperator::CreateOr(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "xor") {
//             return binop->getOpcode() == Instruction::BinaryOps::Xor ? nullptr : BinaryOperator::CreateXor(binop->getOperand(0), binop->getOperand(1));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantBinaryShiftOp(Instruction* binop) {
//         if (MutationOp == "shl") {
//             return binop->getOpcode() == Instruction::BinaryOps::Shl ? nullptr : BinaryOperator::CreateShl(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "lshr") {
//             return binop->getOpcode() == Instruction::BinaryOps::LShr ? nullptr : BinaryOperator::CreateLShr(binop->getOperand(0), binop->getOperand(1));
//         } else if (MutationOp == "ashr") {
//             return binop->getOpcode() == Instruction::BinaryOps::AShr ? nullptr : BinaryOperator::CreateAShr(binop->getOperand(0), binop->getOperand(1));
//         }
//         return nullptr;
//     }
//     // encompases bitwise shifts, bitwise logical operations, and mathematical operations
//     Instruction* getRequestedMutationBinaryOp(Instruction* I) {
//         BinaryOperator* binop = dyn_cast<BinaryOperator>(I);
//         auto opcode = binop->getOpcode();
//         auto opname = binop->getOpcodeName(opcode);

//         if (opcode < Instruction::BinaryOps::Shl && opname[0] != 'f') {
//             return getRequestedMutantBinaryIntegerOp(binop);
//         } else if (opcode < Instruction::BinaryOps::Shl) {
//             return getRequestedMutantBinaryFloatingOp(binop);
//         } else if (opcode < Instruction::BinaryOps::And) {
//             return getRequestedMutantBinaryShiftOp(binop);
//         } else {
//             return getRequestedMutantBinaryLogicalOp(binop);
//         } 
//     }

//     Instruction* getRequestedMutantIcmpUnsignedInst(ICmpInst* cmpInst)
//     {
//         if (MutationOp == "icmpUgt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_UGT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_UGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if(MutationOp == "icmpUge") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_UGE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_UGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if(MutationOp == "icmpUlt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_ULT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_ULT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if(MutationOp == "icmpUle") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_ULE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_ULE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         }

//         return nullptr;
//     }

//     Instruction* getRequestedMutantIcmpSignedInst(ICmpInst* cmpInst)
//     {
//         if(MutationOp == "icmpSgt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGT ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if(MutationOp == "icmpSge") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGE ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SGE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if(MutationOp == "icmpSlt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLT ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLT ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SLT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if(MutationOp == "icmpSle") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLE ? nullptr : cmpInst->getPredicate() == CmpInst::Predicate::ICMP_SLE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_SLE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantIcmpInst(Instruction* I) {
//         ICmpInst* cmpInst = dyn_cast<ICmpInst>(I);
//         IRBuilder<> builder(cmpInst);

//         if (MutationOp == "icmpEq") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_EQ ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_EQ, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "icmpNe") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::ICMP_NE ? nullptr : CmpInst::Create(Instruction::OtherOps::ICmp, CmpInst::Predicate::ICMP_NE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (cmpInst->getPredicate() < CmpInst::Predicate::ICMP_SGT) {
//             return getRequestedMutantIcmpUnsignedInst(cmpInst);
//         } else {
//             return getRequestedMutantIcmpSignedInst(cmpInst);
//         }
//     }

//     Instruction* getRequestedMutantFcmpOrderedInst(FCmpInst* cmpInst)
//     {
//         if (MutationOp == "fcmpOeq") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OEQ ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OEQ, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpOne") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ONE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ONE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpOgt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OGT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpOge") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OGE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpOlt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OLT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OLT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpOle") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_OLE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_OLE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpOrd") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ORD ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ORD, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantFcmpUnorderedInst(FCmpInst* cmpInst)
//     {
//         if (MutationOp == "fcmpUeq") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UEQ ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UEQ, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpUne") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UNE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UNE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpUgt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UGT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UGT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpUge") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UGE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UGE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpUlt") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ULT ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ULT, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpUle") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_ULE ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_ULE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpUno") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_UNO ? nullptr : CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_UNO, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantFcmpInst(Instruction* I) {
//         FCmpInst* cmpInst = dyn_cast<FCmpInst>(I);

//         if (MutationOp == "fcmpTrue") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_TRUE ? nullptr : cmpInst-> CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_TRUE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (MutationOp == "fcmpFalse") {
//             return cmpInst->getPredicate() == CmpInst::Predicate::FCMP_FALSE ? nullptr : cmpInst-> CmpInst::Create(Instruction::OtherOps::FCmp, CmpInst::Predicate::FCMP_FALSE, cmpInst->getOperand(0), cmpInst->getOperand(1));
//         } else if (!(cmpInst->getPredicate() & 0x8)) {
//             return getRequestedMutantFcmpOrderedInst(cmpInst);
//         } else {
//             return getRequestedMutantFcmpUnorderedInst(cmpInst);
//         }

//         return nullptr;
//     }

//     Instruction* getRequestedMutantIntegerSignCastInst(Instruction* I) {
//         CastInst* castInst = dyn_cast<CastInst>(I);
//         if (MutationOp == "zext") {
//             return castInst->getOpcode() == Instruction::CastOps::SExt ? CastInst::Create(Instruction::CastOps::ZExt, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
//         } else if (MutationOp == "sext") {
//             return castInst->getOpcode() == Instruction::CastOps::ZExt ? CastInst::Create(Instruction::CastOps::SExt, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
//         } else if (MutationOp == "fptosi") {
//             return castInst->getOpcode() == Instruction::CastOps::FPToUI ? CastInst::Create(Instruction::CastOps::FPToSI, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
//         } else if (MutationOp == "fptoui") {
//             return castInst->getOpcode() == Instruction::CastOps::FPToSI ? CastInst::Create(Instruction::CastOps::FPToUI, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
//          } else if (MutationOp == "sitofp") {
//             return castInst->getOpcode() == Instruction::CastOps::UIToFP ? CastInst::Create(Instruction::CastOps::SIToFP, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
//         } else if (MutationOp == "uitofp") {
//             return castInst->getOpcode() == Instruction::CastOps::SIToFP ? CastInst::Create(Instruction::CastOps::UIToFP, castInst->getOperand(0), castInst->getDestTy()) : nullptr;
//         }
//         return nullptr;
//     }

//     Instruction* getRequestedMutantGEPInst(Instruction* I, Module& M)
//     {
//       return nullptr;
//     }

//     bool runOnModule(Module &M) {
//         bool bModified = false;
//         uint64_t instrCnt = 0;

//         for (auto& F: M) {
//             if (F.isDeclaration()) 
//                 continue;

//             for (inst_iterator i = inst_begin(F); i != inst_end(F) && !bModified; ++i) {
//                 if (instrCnt == MutationLocation) {
//                     Instruction* I = &*i;
//                     Instruction* altI = nullptr;

//                     if (MutationOp == "null") {
//                         outs() << "nothing modified\n";
//                         return false;
//                     }

//                     if (isa<LoadInst>(I)) {
//                         outs() << "Replacing Load Instruction: ";
//                         altI = getRequestedLoadOp(I);
//                     } else if (isa<ICmpInst>(I)) {
//                         outs() << "Replacing Immediate Comparison Instruction: ";
//                         altI = getRequestedMutantIcmpInst(I);
//                     } else if (isa<FCmpInst>(I)) {
//                         outs() << "Replacing Floating-point Comparison Instruction: ";
//                         altI = getRequestedMutantFcmpInst(I);
//                     } else if (isa<BinaryOperator>(I)) {
//                         outs() << "Replacing Binary Instruction: ";
//                         altI = getRequestedMutationBinaryOp(I);
//                     } else if (isa<CallInst>(I)) {
//                         outs() << "Replacing Call Instruction: ";
//                         altI = getRequestedCallOp(I, M);
//                     } else if (isa<BranchInst>(I)) {
//                         outs() << "Replacing Branch Instruction: ";
//                         if (!getRequestedBranchOp(I)) {
//                             outs() << "no else block detected\n";
//                         } else {
//                              outs() << "else block successfully eliminated\n";
//                             bModified = true;
//                         }
//                         break;
//                     } else if (isa<CastInst>(I)) {
//                         outs() << "Replacing Cast Instruction: ";
//                         altI = getRequestedMutantIntegerSignCastInst(I);
//                     } else if (isa<GetElementPtrInst>(I)) {
//                         outs() << "Replacing GetElementPtr Instruction: ";
//                         altI = getRequestedMutantGEPInst(I, M);
//                     }

//                     if (altI == nullptr) {
//                         outs() << "nothing modified\n";
//                         return bModified;
//                     } else {
//                         outs() << "\n";
//                     }

//                     ReplaceInstWithInst(I, altI);
//                     bModified = true;
//                     break;
//                 }
//                 ++instrCnt;
//             }
//         }

//         for (unsigned i = 0; i < instToDelete.size(); ++i) {
//             outs() << "Deleted\n";
//             instToDelete[i]->eraseFromParent();
//         }

//         for (auto &F : M) {
//             stringToFunc[std::string(F.getName())] = &F;
//             outs() << F.getName() << "\n";
//         }
//         return bModified;
//     }

//     static bool isRequired() { return true; }
// };
// }

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

// namespace {
// struct RegisterInfoPass : public PassInfoMixin<RegisterInfoPass> {
//     PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
//         bool Changed = runOnModule(M);
//         return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
//     }
    
//     void insertGlobals(Module& M) {
//         IntegerType* int32ty = IntegerType::getInt32Ty(M.getContext());
//         IntegerType* int64ty = IntegerType::getInt64Ty(M.getContext());
//         IntegerType* int8ty = IntegerType::getInt8Ty(M.getContext());
//         PointerType* int8ptr = IntegerType::getInt8PtrTy(M.getContext());

//         ArrayType* arrayTy = ArrayType::get(IntegerType::getInt32Ty(M.getContext()), 2);
//         ConstantAggregateZero* zeroInit = ConstantAggregateZero::get(arrayTy);
//         Constant* timeptr = M.getOrInsertGlobal("time", arrayTy);
//         dyn_cast<GlobalVariable>(timeptr)->setInitializer(zeroInit);
//         dyn_cast<GlobalVariable>(timeptr)->setAlignment(MaybeAlign(4));
//         dyn_cast<GlobalVariable>(timeptr)->setDSOLocal(true);

//         arrayTy = ArrayType::get(IntegerType::getInt64Ty(M.getContext()), 16);
//         zeroInit = ConstantAggregateZero::get(arrayTy);
//         Constant* regsptr = M.getOrInsertGlobal("regs", arrayTy);
//         dyn_cast<GlobalVariable>(regsptr)->setInitializer(zeroInit);
//         dyn_cast<GlobalVariable>(regsptr)->setAlignment(MaybeAlign(16));
//         dyn_cast<GlobalVariable>(regsptr)->setDSOLocal(true);

//         arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 3276700);
//         Constant* stringptr = M.getOrInsertGlobal("regstring", arrayTy);
//         dyn_cast<GlobalVariable>(stringptr)->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
//         dyn_cast<GlobalVariable>(stringptr)->setAlignment(MaybeAlign(16));
//         dyn_cast<GlobalVariable>(stringptr)->setDSOLocal(true);

//         arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 30);
//         zeroInit = ConstantAggregateZero::get(arrayTy);
//         Constant* tempstrptr = M.getOrInsertGlobal("tempstring", arrayTy);
//         dyn_cast<GlobalVariable>(tempstrptr)->setInitializer(zeroInit);
//         dyn_cast<GlobalVariable>(tempstrptr)->setAlignment(MaybeAlign(16));
//         dyn_cast<GlobalVariable>(tempstrptr)->setDSOLocal(true);
//     }

//     void insertFunctions(Module& M) {
//         IntegerType* int32ty = IntegerType::getInt32Ty(M.getContext());
//         IntegerType* int64ty = IntegerType::getInt64Ty(M.getContext());
//         IntegerType* int8ty = IntegerType::getInt8Ty(M.getContext());
//         PointerType* int8ptr = IntegerType::getInt8PtrTy(M.getContext());

//         FunctionType* snsprintfTy = FunctionType::get(int32ty, {int8ptr, int64ty, int8ptr}, true);
//         M.getOrInsertFunction("snsprintf", snsprintfTy);

//         FunctionType* strcatTy = FunctionType::get(int8ptr, {int8ptr, int8ptr}, false);
//         M.getOrInsertFunction("strcat", strcatTy);
//     }

//     void insertConstants(Module& M) {
//         ArrayType* arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 14);
//         Constant* initStr = ConstantDataArray::getString(M.getContext(), StringRef("time: %#x%8x\n"), true);
//         Constant* strptr = M.getOrInsertGlobal("str.time", arrayTy);
//         dyn_cast<GlobalVariable>(strptr)->setInitializer(initStr);
//         dyn_cast<GlobalVariable>(strptr)->setConstant(true);
//         dyn_cast<GlobalVariable>(strptr)->setAlignment(MaybeAlign(1));
//         dyn_cast<GlobalVariable>(strptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
//         dyn_cast<GlobalVariable>(strptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
        
//         std::vector<int> strSize = {11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11, 11};
//         std::vector<std::string> registerNames = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14"};
//         for (int i = 0; i < strSize.size(); ++i) {
//             arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), strSize[i]);
//             initStr = ConstantDataArray::getString(M.getContext(), StringRef(registerNames[i] + ": %#zx\n"), true);
//             strptr = M.getOrInsertGlobal("str." + registerNames[i], arrayTy);
//             dyn_cast<GlobalVariable>(strptr)->setInitializer(initStr);
//             dyn_cast<GlobalVariable>(strptr)->setConstant(true);
//             dyn_cast<GlobalVariable>(strptr)->setAlignment(MaybeAlign(1));
//             dyn_cast<GlobalVariable>(strptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
//             dyn_cast<GlobalVariable>(strptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
//         }

//         arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 12);
//         initStr = ConstantDataArray::getString(M.getContext(), StringRef("r15: %#zx\n\n"), true);
//         strptr = M.getOrInsertGlobal("str.r15", arrayTy);
//         dyn_cast<GlobalVariable>(strptr)->setInitializer(initStr);
//         dyn_cast<GlobalVariable>(strptr)->setConstant(true);
//         dyn_cast<GlobalVariable>(strptr)->setAlignment(MaybeAlign(1));
//         dyn_cast<GlobalVariable>(strptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
//         dyn_cast<GlobalVariable>(strptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

//     }

//     Constant* getInBoundsGEP(Type* ty, std::string name, Module& M, uint64_t index) {
//         std::vector<Constant*> indices;
//         indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 0));
//         indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), index));
//         return ConstantExpr::getInBoundsGetElementPtr(ty, M.getOrInsertGlobal(name, ty), indices);
//     }

//     void injectRegisterInsts(Instruction* inst, Module& M) {
//         IRBuilder<> builder(inst);
//         std::vector<int> strSize = {11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11, 11, 12};
//         std::vector<std::string> registerNames = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

//         ArrayType* timeTy = ArrayType::get(IntegerType::getInt32Ty(M.getContext()), 2);
//         ArrayType* regsTy = ArrayType::get(IntegerType::getInt64Ty(M.getContext()), 16);
//         ArrayType* regstringTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 3276700);
//         ArrayType* tempstrTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 30);
//         PointerType* int32ptr = IntegerType::getInt32PtrTy(M.getContext());
//         PointerType* int64ptr = IntegerType::getInt64PtrTy(M.getContext());
//         PointerType* int8ptr = IntegerType::getInt8PtrTy(M.getContext());
//         IntegerType* int8ty = IntegerType::getInt8Ty(M.getContext());
//         IntegerType* int32ty = IntegerType::getInt32Ty(M.getContext());
//         IntegerType* int64ty = IntegerType::getInt64Ty(M.getContext());
        
//         CallInst* callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
//                            "pushq %rdx", "~{dirflag},~{fpsr},~{flags}", true));
//         callInst->setTailCall(true);

//         callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
//                            "pushq %rax", "~{dirflag},~{fpsr},~{flags}", true));
//         callInst->setTailCall(true);

//         callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
//                            "rdtsc", "~{dirflag},~{fpsr},~{flags}", true));
//         callInst->setTailCall(true);
        
//         callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), {int32ptr}, false),
//                            "movl %edx, $0", "=*rm,~{dirflag},~{fpsr},~{flags}", false), getInBoundsGEP(timeTy, "time", M, 0));
//         callInst->setTailCall(true);
        
//         callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), {int32ptr}, false),
//                            "movl %eax, $0", "=*rm,~{dirflag},~{fpsr},~{flags}", false), getInBoundsGEP(timeTy, "time", M, 1));
//         callInst->setTailCall(true);

//         callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
//                            "popq %rax", "~{dirflag},~{fpsr},~{flags}", true));
//         callInst->setTailCall(true);

//         callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
//                            "popq %rdx", "~{dirflag},~{fpsr},~{flags}", true));
//         callInst->setTailCall(true);

//         for (size_t i = 0; i < registerNames.size(); ++i) {
//             callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), {int64ptr}, false),
//                            "movq %" + registerNames[i] + ", $0", "=*rm,~{dirflag},~{fpsr},~{flags}", false), getInBoundsGEP(regsTy, "regs", M, i));
//             callInst->setTailCall(true);
//         }
        

//         // Time Store Sequence
//         FunctionType* snsprintfTy = FunctionType::get(int32ty, {int8ptr, int64ty, int8ptr}, true);
//         FunctionType* strcatTy = FunctionType::get(int8ptr, {int8ptr, int8ptr}, false);
//         FunctionCallee snprintfCallee = M.getOrInsertFunction("snprintf", snsprintfTy);
//         FunctionCallee strcatCallee = M.getOrInsertFunction("strcat", strcatTy);

//         ArrayType* arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 14);
//         Value* firstLoad = builder.CreateLoad(int32ty, getInBoundsGEP(timeTy, "time", M, 0));
//         Value* secondLoad = builder.CreateLoad(int32ty, getInBoundsGEP(timeTy, "time", M, 1));
//         CallInst* callsnsprintf = builder.CreateCall(snprintfCallee, {getInBoundsGEP(tempstrTy, "tempstring", M, 0),
//                                                                    ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 30),
//                                                                    getInBoundsGEP(arrayTy, "str.time", M, 0), firstLoad, secondLoad });
//         callsnsprintf->setTailCall(true);
//         CallInst* callstrcat = builder.CreateCall(strcatCallee, {getInBoundsGEP(regstringTy, "regstring", M, 0),
//                                                               getInBoundsGEP(tempstrTy, "tempstring", M, 0) });
//         callstrcat->setTailCall(true);
//         builder.CreateStore(ConstantInt::get(IntegerType::getInt8Ty(M.getContext()), 0), getInBoundsGEP(tempstrTy, "tempstring", M, 0));

//         for (int i = 0; i < strSize.size(); ++i) {
//             arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), strSize[i]);
//             firstLoad = builder.CreateLoad(int64ty, getInBoundsGEP(regsTy, "regs", M, i));
//             CallInst* callsnsprintf = builder.CreateCall(snprintfCallee, {getInBoundsGEP(tempstrTy, "tempstring", M, 0),
//                                                                        ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 30),
//                                                                        getInBoundsGEP(arrayTy, "str." + registerNames[i], M, 0), firstLoad});
//             callsnsprintf->setTailCall(true);
//             CallInst* callstrcat = builder.CreateCall(strcatCallee, {getInBoundsGEP(regstringTy, "regstring", M, 0),
//                                                                   getInBoundsGEP(tempstrTy, "tempstring", M, 0) });
//             callstrcat->setTailCall(true);
//             builder.CreateStore(ConstantInt::get(IntegerType::getInt8Ty(M.getContext()), 0), getInBoundsGEP(tempstrTy, "tempstring", M, 0));
//         }
//     }

//     bool runOnModule(Module& M) {
//         // injection of global string constants and arrays
//         insertGlobals(M);
//         insertConstants(M);
//         insertFunctions(M);
//         bool isModified = false;
//         uint64_t instrCnt = 0;
//         for (auto& F: M) {
//             if (F.isDeclaration()) 
//                 continue;

//             for (inst_iterator i = inst_begin(F); i != inst_end(F) && !isModified; ++i) {
//                 if (instrCnt == MutationLocation) {
//                     injectRegisterInsts(&*i, M);
//                     isModified = true;
//                 }
//                 ++instrCnt;
//             }
//         }
//         return isModified;
//     }

//     static bool isRequired() { return true; }
// };
// }

// namespace {
// struct RegisterExitPass : public PassInfoMixin<RegisterExitPass> {
//     PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
//         bool Changed = runOnModule(M);
//         return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
//     }

//     void insertGlobals(Module& M) {
//         auto& ctx = M.getContext();
//         IntegerType* int32ty = IntegerType::getInt32Ty(ctx);
//         IntegerType* int64ty = IntegerType::getInt64Ty(ctx);
//         IntegerType* int8ty = IntegerType::getInt8Ty(ctx);
//         PointerType* int8ptr = IntegerType::getInt8PtrTy(ctx);

//         ArrayType* arrayTy = ArrayType::get(int8ty, 3276700);
//         ConstantAggregateZero* zeroInit = ConstantAggregateZero::get(arrayTy);
//         Constant* initStr = ConstantDataArray::getString(ctx, "", true);
//         Constant* stringptr = M.getOrInsertGlobal("regstring", arrayTy);
//         dyn_cast<GlobalVariable>(stringptr)->setAlignment(MaybeAlign(16));
//         dyn_cast<GlobalVariable>(stringptr)->setInitializer(zeroInit);

//         arrayTy = ArrayType::get(int8ty, OutputFile.length() + 1);
//         initStr = ConstantDataArray::getString(ctx, static_cast<const char*>(OutputFile.data()), true);
//         Constant* outputFilePtr = M.getOrInsertGlobal("outputFile", arrayTy);
//         dyn_cast<GlobalVariable>(outputFilePtr)->setAlignment(MaybeAlign(1));
//         dyn_cast<GlobalVariable>(outputFilePtr)->setInitializer(initStr);
//         dyn_cast<GlobalVariable>(outputFilePtr)->setConstant(true);
//         dyn_cast<GlobalVariable>(outputFilePtr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
//         dyn_cast<GlobalVariable>(outputFilePtr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

//         arrayTy = ArrayType::get(int8ty, 2);
//         initStr = ConstantDataArray::getString(ctx, "a", true);
//         Constant* filemodeptr = M.getOrInsertGlobal("filemode", arrayTy);
//         dyn_cast<GlobalVariable>(filemodeptr)->setAlignment(MaybeAlign(1));
//         dyn_cast<GlobalVariable>(filemodeptr)->setInitializer(initStr);
//         dyn_cast<GlobalVariable>(filemodeptr)->setConstant(true);
//         dyn_cast<GlobalVariable>(filemodeptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
//         dyn_cast<GlobalVariable>(filemodeptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
//     }

//     Constant* getInBoundsGEP(Type* ty, std::string name, Module& M, uint64_t index) {
//         std::vector<Constant*> indices;
//         indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 0));
//         indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), index));
//         return ConstantExpr::getInBoundsGetElementPtr(ty, M.getOrInsertGlobal(name, ty), indices);
//     }

//     FunctionCallee insertHandler(Module& M) {
//         auto& ctx = M.getContext();
//         IntegerType* int32ty = IntegerType::getInt32Ty(ctx);
//         IntegerType* int64ty = IntegerType::getInt64Ty(ctx);
//         IntegerType* int8ty = IntegerType::getInt8Ty(ctx);
//         IntegerType* int16ty = IntegerType::getInt16Ty(ctx);
//         PointerType* int8ptr = IntegerType::getInt8PtrTy(ctx);
//         auto structTypes = M.getIdentifiedStructTypes();
//         bool foundType = false;

//         StructType* file = nullptr;
//         for (auto i: structTypes) {
//             if (i->hasName() && i->getName() == "struct._IO_FILE") {
//                 foundType = true;
//                 file = i;
//                 break;
//             }
//         }

//         // have to create the structs necessary for a FILE struct
//         if (!foundType) {
//             StructType* ioMarker = StructType::create(ctx, StringRef("struct._IO_marker"));
//             StructType* ioCodecvt = StructType::create(ctx, "struct._IO_codecvt");
//             StructType* ioWideData = StructType::create(ctx, "struct._IO_wide_data");
//             // M.getOrInsertGlobal(ioMarker);
//             // M.getOrInsertGlobal(ioCodecvt);
//             // M.getOrInsertGlobal(ioWideData);

//             std::vector<Type*> ioFileTy;
//             file = StructType::create(ctx, "struct._IO_FILE");
//             ioFileTy.push_back(int32ty);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(PointerType::getUnqual(ioMarker));
//             ioFileTy.push_back(PointerType::getUnqual(file));
//             ioFileTy.push_back(int32ty);
//             ioFileTy.push_back(int32ty);
//             ioFileTy.push_back(int64ty);
//             ioFileTy.push_back(int16ty);
//             ioFileTy.push_back(int8ty);
//             ioFileTy.push_back(ArrayType::get(int8ty, 1));
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int64ty);
//             ioFileTy.push_back(PointerType::getUnqual(ioCodecvt));
//             ioFileTy.push_back(PointerType::getUnqual(ioWideData));
//             ioFileTy.push_back(PointerType::getUnqual(file));
//             ioFileTy.push_back(int8ptr);
//             ioFileTy.push_back(int64ty);
//             ioFileTy.push_back(int32ty);
//             ioFileTy.push_back(ArrayType::get(int8ty, 20));

//             file->setBody(ioFileTy, false);
//         }
//         PointerType* fileptr = PointerType::getUnqual(file);

//         // insert the handler
//         FunctionType* voidTy = FunctionType::get(Type::getVoidTy(ctx), false);
//         FunctionType* fopenTy = FunctionType::get(fileptr, {int8ptr, int8ptr}, false);
//         FunctionType* fputsTy = FunctionType::get(int32ty, {int8ptr, fileptr}, false);
//         FunctionType* fcloseTy = FunctionType::get(int32ty, fileptr, false);
//         FunctionCallee handler = M.getOrInsertFunction("regInfoNormalExitHandler", voidTy);
//         FunctionCallee fopenFunc = M.getOrInsertFunction("fopen", fopenTy);
//         FunctionCallee fputsFunc = M.getOrInsertFunction("fputs", fputsTy);
//         FunctionCallee fcloseFunc = M.getOrInsertFunction("fclose", fcloseTy);

//         ArrayType* regstringty = ArrayType::get(int8ty, 3276700);
//         ArrayType* filenamety = ArrayType::get(int8ty, OutputFile.length() + 1);
//         ArrayType* modety = ArrayType::get(int8ty, 2);
//         Constant* filename = getInBoundsGEP(filenamety, "outputFile", M, 0);
//         Constant* mode = getInBoundsGEP(modety, "filemode", M, 0);
//         Constant* regstring = getInBoundsGEP(regstringty, "regstring", M, 0);

//         // insert instructions
//         BasicBlock* handlerBlock = BasicBlock::Create(ctx, "", dyn_cast<Function>(handler.getCallee()));
//         IRBuilder<> builder(handlerBlock);

//         CallInst* fileStructPtr = builder.CreateCall(fopenFunc, {filename, mode});
//         fileStructPtr->setTailCall();
//         CallInst* writeToFile = builder.CreateCall(fputsFunc, {regstring, fileStructPtr});
//         writeToFile->setTailCall();
//         builder.CreateCall(fcloseFunc, fileStructPtr)->setTailCall();
//         builder.CreateRetVoid();
//         return handler;
//     }

//     bool runOnModule(Module& M) {
//         // injection of global string constants and arrays
//         auto& ctx = M.getContext();
//         insertGlobals(M);
//         FunctionCallee handle = insertHandler(M);

//         // insert atexit and final dump call
//         for (auto& F: M) {
//             if (F.getName() != "main") {
//                 continue;
//             }
//             IntegerType* int32ty = IntegerType::getInt32Ty(ctx);
//             PointerType* funcPtr = PointerType::getUnqual(FunctionType::get(Type::getVoidTy(ctx), false));
//             FunctionType* atexitTy = FunctionType::get(int32ty, funcPtr, false);
//             FunctionCallee atexitFunc = M.getOrInsertFunction("atexit", atexitTy);
//             IRBuilder<> builder(&*F.getEntryBlock().getFirstInsertionPt());
//             builder.CreateCall(atexitFunc, handle.getCallee());
//             break;
//         }
//         return true;
//     //     insertGlobals(M);
//     //     insertFunctions(M);
//     //     bool isModified = false;
//     //     uint64_t instrCnt = 0;
//     //     for (auto& F: M) {
//     //         if (F.isDeclaration()) 
//     //             continue;

//     //         for (inst_iterator i = inst_begin(F); i != inst_end(F) && !isModified; ++i) {
//     //             if (instrCnt == MutationLocation) {
//     //                 isModified = true;
//     //             }
//     //         ++instrCnt;
//     //         }
//     //     }
//     //     return isModified;
//     }

//     static bool isRequired() { return true; }
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
                        MPM.addPass(MutatePass(MutationOp, MutationLocation, SwapParamNums, HashAlgorithm));
                        return true;
                   } else if (Name == "label-pass") {
                        MPM.addPass(LabelPass(OutputFile));
                        return true;
                   } else if (Name == "register-info-pass") {
                        MPM.addPass(RegisterInfoPass(MutationLocation));
                        return true;
                   } else if (Name == "register-exit-pass") {
                        MPM.addPass(RegisterExitPass(OutputFile));
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
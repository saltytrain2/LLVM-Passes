#include "RegisterInfoPass.h"

#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;

RegisterInfoPass::RegisterInfoPass(uint32_t mutationLoc)
    : mMutationLoc(mutationLoc)
{}

PreservedAnalyses RegisterInfoPass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

void RegisterInfoPass::insertGlobals(Module& M) 
{
    IntegerType* int32ty = IntegerType::getInt32Ty(M.getContext());
    IntegerType* int64ty = IntegerType::getInt64Ty(M.getContext());
    IntegerType* int8ty = IntegerType::getInt8Ty(M.getContext());
    PointerType* int8ptr = IntegerType::getInt8PtrTy(M.getContext());

    ArrayType* arrayTy = ArrayType::get(IntegerType::getInt32Ty(M.getContext()), 2);
    ConstantAggregateZero* zeroInit = ConstantAggregateZero::get(arrayTy);
    Constant* timeptr = M.getOrInsertGlobal("time", arrayTy);
    dyn_cast<GlobalVariable>(timeptr)->setInitializer(zeroInit);
    dyn_cast<GlobalVariable>(timeptr)->setAlignment(MaybeAlign(4));
    dyn_cast<GlobalVariable>(timeptr)->setDSOLocal(true);

    arrayTy = ArrayType::get(IntegerType::getInt64Ty(M.getContext()), 16);
    zeroInit = ConstantAggregateZero::get(arrayTy);
    Constant* regsptr = M.getOrInsertGlobal("regs", arrayTy);
    dyn_cast<GlobalVariable>(regsptr)->setInitializer(zeroInit);
    dyn_cast<GlobalVariable>(regsptr)->setAlignment(MaybeAlign(16));
    dyn_cast<GlobalVariable>(regsptr)->setDSOLocal(true);

    arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 3276700);
    Constant* stringptr = M.getOrInsertGlobal("regstring", arrayTy);
    dyn_cast<GlobalVariable>(stringptr)->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
    dyn_cast<GlobalVariable>(stringptr)->setAlignment(MaybeAlign(16));
    dyn_cast<GlobalVariable>(stringptr)->setDSOLocal(true);

    arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 30);
    zeroInit = ConstantAggregateZero::get(arrayTy);
    Constant* tempstrptr = M.getOrInsertGlobal("tempstring", arrayTy);
    dyn_cast<GlobalVariable>(tempstrptr)->setInitializer(zeroInit);
    dyn_cast<GlobalVariable>(tempstrptr)->setAlignment(MaybeAlign(16));
    dyn_cast<GlobalVariable>(tempstrptr)->setDSOLocal(true);
}

void RegisterInfoPass::insertFunctions(Module& M) 
{
    IntegerType* int32ty = IntegerType::getInt32Ty(M.getContext());
    IntegerType* int64ty = IntegerType::getInt64Ty(M.getContext());
    IntegerType* int8ty = IntegerType::getInt8Ty(M.getContext());
    PointerType* int8ptr = IntegerType::getInt8PtrTy(M.getContext());

    FunctionType* snsprintfTy = FunctionType::get(int32ty, {int8ptr, int64ty, int8ptr}, true);
    M.getOrInsertFunction("snsprintf", snsprintfTy);

    FunctionType* strcatTy = FunctionType::get(int8ptr, {int8ptr, int8ptr}, false);
    M.getOrInsertFunction("strcat", strcatTy);
}

void RegisterInfoPass::insertConstants(Module& M) 
{
    ArrayType* arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 14);
    Constant* initStr = ConstantDataArray::getString(M.getContext(), StringRef("time: %#x%8x\n"), true);
    Constant* strptr = M.getOrInsertGlobal("str.time", arrayTy);
    dyn_cast<GlobalVariable>(strptr)->setInitializer(initStr);
    dyn_cast<GlobalVariable>(strptr)->setConstant(true);
    dyn_cast<GlobalVariable>(strptr)->setAlignment(MaybeAlign(1));
    dyn_cast<GlobalVariable>(strptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
    dyn_cast<GlobalVariable>(strptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    
    std::vector<int> strSize = {11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11, 11};
    std::vector<std::string> registerNames = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14"};
    for (int i = 0; i < strSize.size(); ++i) {
        arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), strSize[i]);
        initStr = ConstantDataArray::getString(M.getContext(), StringRef(registerNames[i] + ": %#zx\n"), true);
        strptr = M.getOrInsertGlobal("str." + registerNames[i], arrayTy);
        dyn_cast<GlobalVariable>(strptr)->setInitializer(initStr);
        dyn_cast<GlobalVariable>(strptr)->setConstant(true);
        dyn_cast<GlobalVariable>(strptr)->setAlignment(MaybeAlign(1));
        dyn_cast<GlobalVariable>(strptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
        dyn_cast<GlobalVariable>(strptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    }

    arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 12);
    initStr = ConstantDataArray::getString(M.getContext(), StringRef("r15: %#zx\n\n"), true);
    strptr = M.getOrInsertGlobal("str.r15", arrayTy);
    dyn_cast<GlobalVariable>(strptr)->setInitializer(initStr);
    dyn_cast<GlobalVariable>(strptr)->setConstant(true);
    dyn_cast<GlobalVariable>(strptr)->setAlignment(MaybeAlign(1));
    dyn_cast<GlobalVariable>(strptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
    dyn_cast<GlobalVariable>(strptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
}

Constant* RegisterInfoPass::getInBoundsGEP(Type* ty, std::string name, Module& M, uint64_t index) 
{
    std::vector<Constant*> indices;
    indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 0));
    indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), index));
    return ConstantExpr::getInBoundsGetElementPtr(ty, M.getOrInsertGlobal(name, ty), indices);
}

void RegisterInfoPass::injectRegisterInsts(Instruction* inst, Module& M) 
{
    IRBuilder<> builder(inst);
    std::vector<int> strSize = {11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11, 11, 12};
    std::vector<std::string> registerNames = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

    ArrayType* timeTy = ArrayType::get(IntegerType::getInt32Ty(M.getContext()), 2);
    ArrayType* regsTy = ArrayType::get(IntegerType::getInt64Ty(M.getContext()), 16);
    ArrayType* regstringTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 3276700);
    ArrayType* tempstrTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 30);
    PointerType* int32ptr = IntegerType::getInt32PtrTy(M.getContext());
    PointerType* int64ptr = IntegerType::getInt64PtrTy(M.getContext());
    PointerType* int8ptr = IntegerType::getInt8PtrTy(M.getContext());
    IntegerType* int8ty = IntegerType::getInt8Ty(M.getContext());
    IntegerType* int32ty = IntegerType::getInt32Ty(M.getContext());
    IntegerType* int64ty = IntegerType::getInt64Ty(M.getContext());
    
    CallInst* callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
                        "pushq %rdx", "~{dirflag},~{fpsr},~{flags}", true));
    callInst->setTailCall(true);

    callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
                        "pushq %rax", "~{dirflag},~{fpsr},~{flags}", true));
    callInst->setTailCall(true);

    callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
                        "rdtsc", "~{dirflag},~{fpsr},~{flags}", true));
    callInst->setTailCall(true);
    
    callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), {int32ptr}, false),
                        "movl %edx, $0", "=*rm,~{dirflag},~{fpsr},~{flags}", false), getInBoundsGEP(timeTy, "time", M, 0));
    callInst->setTailCall(true);
    
    callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), {int32ptr}, false),
                        "movl %eax, $0", "=*rm,~{dirflag},~{fpsr},~{flags}", false), getInBoundsGEP(timeTy, "time", M, 1));
    callInst->setTailCall(true);

    callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
                        "popq %rax", "~{dirflag},~{fpsr},~{flags}", true));
    callInst->setTailCall(true);

    callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), false),
                        "popq %rdx", "~{dirflag},~{fpsr},~{flags}", true));
    callInst->setTailCall(true);

    for (size_t i = 0; i < registerNames.size(); ++i) {
        callInst = builder.CreateCall(InlineAsm::get(FunctionType::get(Type::getVoidTy(M.getContext()), {int64ptr}, false),
                        "movq %" + registerNames[i] + ", $0", "=*rm,~{dirflag},~{fpsr},~{flags}", false), getInBoundsGEP(regsTy, "regs", M, i));
        callInst->setTailCall(true);
    }
    

    // Time Store Sequence
    FunctionType* snsprintfTy = FunctionType::get(int32ty, {int8ptr, int64ty, int8ptr}, true);
    FunctionType* strcatTy = FunctionType::get(int8ptr, {int8ptr, int8ptr}, false);
    FunctionCallee snprintfCallee = M.getOrInsertFunction("snprintf", snsprintfTy);
    FunctionCallee strcatCallee = M.getOrInsertFunction("strcat", strcatTy);

    ArrayType* arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), 14);
    Value* firstLoad = builder.CreateLoad(int32ty, getInBoundsGEP(timeTy, "time", M, 0));
    Value* secondLoad = builder.CreateLoad(int32ty, getInBoundsGEP(timeTy, "time", M, 1));
    CallInst* callsnsprintf = builder.CreateCall(snprintfCallee, {getInBoundsGEP(tempstrTy, "tempstring", M, 0),
                                                                ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 30),
                                                                getInBoundsGEP(arrayTy, "str.time", M, 0), firstLoad, secondLoad });
    callsnsprintf->setTailCall(true);
    CallInst* callstrcat = builder.CreateCall(strcatCallee, {getInBoundsGEP(regstringTy, "regstring", M, 0),
                                                            getInBoundsGEP(tempstrTy, "tempstring", M, 0) });
    callstrcat->setTailCall(true);
    builder.CreateStore(ConstantInt::get(IntegerType::getInt8Ty(M.getContext()), 0), getInBoundsGEP(tempstrTy, "tempstring", M, 0));

    for (int i = 0; i < strSize.size(); ++i) {
        arrayTy = ArrayType::get(IntegerType::getInt8Ty(M.getContext()), strSize[i]);
        firstLoad = builder.CreateLoad(int64ty, getInBoundsGEP(regsTy, "regs", M, i));
        CallInst* callsnsprintf = builder.CreateCall(snprintfCallee, {getInBoundsGEP(tempstrTy, "tempstring", M, 0),
                                                                    ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 30),
                                                                    getInBoundsGEP(arrayTy, "str." + registerNames[i], M, 0), firstLoad});
        callsnsprintf->setTailCall(true);
        CallInst* callstrcat = builder.CreateCall(strcatCallee, {getInBoundsGEP(regstringTy, "regstring", M, 0),
                                                                getInBoundsGEP(tempstrTy, "tempstring", M, 0) });
        callstrcat->setTailCall(true);
        builder.CreateStore(ConstantInt::get(IntegerType::getInt8Ty(M.getContext()), 0), getInBoundsGEP(tempstrTy, "tempstring", M, 0));
    }
}

bool RegisterInfoPass::runOnModule(Module& M) 
{
    // injection of global string constants and arrays
    insertGlobals(M);
    insertConstants(M);
    insertFunctions(M);
    bool isModified = false;
    uint64_t instrCnt = 0;
    for (auto& F: M) {
        if (F.isDeclaration()) 
            continue;

        for (inst_iterator i = inst_begin(F); i != inst_end(F) && !isModified; ++i) {
            if (instrCnt == mMutationLoc) {
                injectRegisterInsts(&*i, M);
                isModified = true;
            }
            ++instrCnt;
        }
    }
    return isModified;
}
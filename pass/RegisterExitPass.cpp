#include "RegisterExitPass.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;

RegisterExitPass::RegisterExitPass(std::string outputFile)
    : mOutputFile(outputFile)
{}

PreservedAnalyses RegisterExitPass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

void RegisterExitPass::insertGlobals(Module& M) 
{
    auto& ctx = M.getContext();
    IntegerType* int32ty = IntegerType::getInt32Ty(ctx);
    IntegerType* int64ty = IntegerType::getInt64Ty(ctx);
    IntegerType* int8ty = IntegerType::getInt8Ty(ctx);
    PointerType* int8ptr = IntegerType::getInt8PtrTy(ctx);

    ArrayType* arrayTy = ArrayType::get(int8ty, 3276700);
    ConstantAggregateZero* zeroInit = ConstantAggregateZero::get(arrayTy);
    Constant* initStr = ConstantDataArray::getString(ctx, "", true);
    Constant* stringptr = M.getOrInsertGlobal("regstring", arrayTy);
    dyn_cast<GlobalVariable>(stringptr)->setAlignment(MaybeAlign(16));
    dyn_cast<GlobalVariable>(stringptr)->setInitializer(zeroInit);

    arrayTy = ArrayType::get(int8ty, mOutputFile.length() + 1);
    initStr = ConstantDataArray::getString(ctx, static_cast<const char*>(mOutputFile.data()), true);
    Constant* outputFilePtr = M.getOrInsertGlobal("outputFile", arrayTy);
    dyn_cast<GlobalVariable>(outputFilePtr)->setAlignment(MaybeAlign(1));
    dyn_cast<GlobalVariable>(outputFilePtr)->setInitializer(initStr);
    dyn_cast<GlobalVariable>(outputFilePtr)->setConstant(true);
    dyn_cast<GlobalVariable>(outputFilePtr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
    dyn_cast<GlobalVariable>(outputFilePtr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    arrayTy = ArrayType::get(int8ty, 2);
    initStr = ConstantDataArray::getString(ctx, "a", true);
    Constant* filemodeptr = M.getOrInsertGlobal("filemode", arrayTy);
    dyn_cast<GlobalVariable>(filemodeptr)->setAlignment(MaybeAlign(1));
    dyn_cast<GlobalVariable>(filemodeptr)->setInitializer(initStr);
    dyn_cast<GlobalVariable>(filemodeptr)->setConstant(true);
    dyn_cast<GlobalVariable>(filemodeptr)->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
    dyn_cast<GlobalVariable>(filemodeptr)->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
}

Constant* RegisterExitPass::getInBoundsGEP(Type* ty, std::string name, Module& M, uint64_t index) 
{
    std::vector<Constant*> indices;
    indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 0));
    indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), index));
    return ConstantExpr::getInBoundsGetElementPtr(ty, M.getOrInsertGlobal(name, ty), indices);
}

FunctionCallee RegisterExitPass::insertHandler(Module& M) 
{
    auto& ctx = M.getContext();
    IntegerType* int32ty = IntegerType::getInt32Ty(ctx);
    IntegerType* int64ty = IntegerType::getInt64Ty(ctx);
    IntegerType* int8ty = IntegerType::getInt8Ty(ctx);
    IntegerType* int16ty = IntegerType::getInt16Ty(ctx);
    PointerType* int8ptr = IntegerType::getInt8PtrTy(ctx);
    auto structTypes = M.getIdentifiedStructTypes();
    bool foundType = false;

    StructType* file = nullptr;
    for (auto i: structTypes) {
        if (i->hasName() && i->getName() == "struct._IO_FILE") {
            foundType = true;
            file = i;
            break;
        }
    }

    // have to create the structs necessary for a FILE struct
    if (!foundType) {
        StructType* ioMarker = StructType::create(ctx, StringRef("struct._IO_marker"));
        StructType* ioCodecvt = StructType::create(ctx, "struct._IO_codecvt");
        StructType* ioWideData = StructType::create(ctx, "struct._IO_wide_data");
        // M.getOrInsertGlobal(ioMarker);
        // M.getOrInsertGlobal(ioCodecvt);
        // M.getOrInsertGlobal(ioWideData);

        std::vector<Type*> ioFileTy;
        file = StructType::create(ctx, "struct._IO_FILE");
        ioFileTy.push_back(int32ty);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(PointerType::getUnqual(ioMarker));
        ioFileTy.push_back(PointerType::getUnqual(file));
        ioFileTy.push_back(int32ty);
        ioFileTy.push_back(int32ty);
        ioFileTy.push_back(int64ty);
        ioFileTy.push_back(int16ty);
        ioFileTy.push_back(int8ty);
        ioFileTy.push_back(ArrayType::get(int8ty, 1));
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int64ty);
        ioFileTy.push_back(PointerType::getUnqual(ioCodecvt));
        ioFileTy.push_back(PointerType::getUnqual(ioWideData));
        ioFileTy.push_back(PointerType::getUnqual(file));
        ioFileTy.push_back(int8ptr);
        ioFileTy.push_back(int64ty);
        ioFileTy.push_back(int32ty);
        ioFileTy.push_back(ArrayType::get(int8ty, 20));

        file->setBody(ioFileTy, false);
    }
    PointerType* fileptr = PointerType::getUnqual(file);

    // insert the handler
    FunctionType* voidTy = FunctionType::get(Type::getVoidTy(ctx), false);
    FunctionType* fopenTy = FunctionType::get(fileptr, {int8ptr, int8ptr}, false);
    FunctionType* fputsTy = FunctionType::get(int32ty, {int8ptr, fileptr}, false);
    FunctionType* fcloseTy = FunctionType::get(int32ty, fileptr, false);
    FunctionCallee handler = M.getOrInsertFunction("regInfoNormalExitHandler", voidTy);
    FunctionCallee fopenFunc = M.getOrInsertFunction("fopen", fopenTy);
    FunctionCallee fputsFunc = M.getOrInsertFunction("fputs", fputsTy);
    FunctionCallee fcloseFunc = M.getOrInsertFunction("fclose", fcloseTy);

    ArrayType* regstringty = ArrayType::get(int8ty, 3276700);
    ArrayType* filenamety = ArrayType::get(int8ty, mOutputFile.length() + 1);
    ArrayType* modety = ArrayType::get(int8ty, 2);
    Constant* filename = getInBoundsGEP(filenamety, "outputFile", M, 0);
    Constant* mode = getInBoundsGEP(modety, "filemode", M, 0);
    Constant* regstring = getInBoundsGEP(regstringty, "regstring", M, 0);

    // insert instructions
    BasicBlock* handlerBlock = BasicBlock::Create(ctx, "", dyn_cast<Function>(handler.getCallee()));
    IRBuilder<> builder(handlerBlock);

    CallInst* fileStructPtr = builder.CreateCall(fopenFunc, {filename, mode});
    fileStructPtr->setTailCall();
    CallInst* writeToFile = builder.CreateCall(fputsFunc, {regstring, fileStructPtr});
    writeToFile->setTailCall();
    builder.CreateCall(fcloseFunc, fileStructPtr)->setTailCall();
    builder.CreateRetVoid();
    return handler;
}

FunctionCallee RegisterExitPass::insertSignalHandler(Module& M)
{
    auto& ctx = M.getContext();

    FunctionType* handleTy = FunctionType::get(Type::getVoidTy(ctx), IntegerType::getInt32Ty(ctx), false);
    FunctionCallee handlerFunc = M.getOrInsertFunction("signalExitHandler", handleTy);
    FunctionCallee exitFunc = M.getOrInsertFunction("exit", handleTy);

    BasicBlock* handlerBlock = BasicBlock::Create(ctx, "", dyn_cast<Function>(handlerFunc.getCallee()));
    IRBuilder<> builder(handlerBlock);

    builder.CreateCall(exitFunc, dyn_cast<Function>(handlerFunc.getCallee())->getArg(0))->setTailCall();
    builder.CreateUnreachable();
    return handlerFunc;
}

bool RegisterExitPass::runOnModule(Module& M) 
{
    // injection of global string constants and arrays
    auto& ctx = M.getContext();
    insertGlobals(M);
    FunctionCallee exitHandle = insertHandler(M);
    FunctionCallee signalHandle = insertSignalHandler(M);


    // insert atexit and final dump call
    for (auto& F: M) {
        if (F.getName() != "main") {
            continue;
        }
        IntegerType* int32ty = IntegerType::getInt32Ty(ctx);
        PointerType* funcPtr = PointerType::getUnqual(FunctionType::get(Type::getVoidTy(ctx), false));
        FunctionType* atexitTy = FunctionType::get(int32ty, funcPtr, false);
        FunctionCallee atexitFunc = M.getOrInsertFunction("atexit", atexitTy);
        IRBuilder<> builder(&*F.getEntryBlock().getFirstInsertionPt());
        builder.CreateCall(atexitFunc, exitHandle.getCallee());

        funcPtr = PointerType::getUnqual(FunctionType::get(Type::getVoidTy(ctx), int32ty, false));
        FunctionType* signalTy = FunctionType::get(funcPtr, {int32ty, funcPtr}, false);
        FunctionCallee signalFunc = M.getOrInsertFunction("signal", signalTy);
        builder.CreateCall(signalFunc, {ConstantInt::get(IntegerType::getInt32Ty(ctx), 2), signalHandle.getCallee()})->setTailCall();
        builder.CreateCall(signalFunc, {ConstantInt::get(IntegerType::getInt32Ty(ctx), 3), signalHandle.getCallee()})->setTailCall();
        builder.CreateCall(signalFunc, {ConstantInt::get(IntegerType::getInt32Ty(ctx), 6), signalHandle.getCallee()})->setTailCall();
        builder.CreateCall(signalFunc, {ConstantInt::get(IntegerType::getInt32Ty(ctx), 11), signalHandle.getCallee()})->setTailCall();
        builder.CreateCall(signalFunc, {ConstantInt::get(IntegerType::getInt32Ty(ctx), 8), signalHandle.getCallee()})->setTailCall();
        builder.CreateCall(signalFunc, {ConstantInt::get(IntegerType::getInt32Ty(ctx), 15), signalHandle.getCallee()})->setTailCall();
        break;
    }
    return true;
}
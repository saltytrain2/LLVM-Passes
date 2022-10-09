#ifndef _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H
#define _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H

#include <string>
#include <cstdint>

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class RegisterExitPass : public PassInfoMixin<RegisterExitPass>
{
public:
    RegisterExitPass(std::string outputFile);
    inline static bool isRequired() { return true; }
    PreservedAnalyses run(Module& M, ModuleAnalysisManager&);

private:
    std::string mOutputFile;

    void insertGlobals(Module& M);
    Constant* getInBoundsGEP(Type* ty, std::string anem, Module& M, uint64_t index);
    FunctionCallee insertHandler(Module& M);
    FunctionCallee insertSignalHandler(Module& M);
    bool runOnModule(Module& M);
}; 
} // namespace llvm

#endif // _LLVMPASSES__INCLUDE__REGISTEREXITPASS_H
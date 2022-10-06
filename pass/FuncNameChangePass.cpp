#include "FuncNameChangePass.h"

#include <string>
#include <algorithm>
#include <fstream>
#include <memory>

#include "llvm/IR/Function.h"

using namespace llvm;

WindowPass::WindowPass()
{}

PreservedAnalyses WindowPass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool WindowPass::runOnModule(Module& M)
{
    int counter = 0;
    for (auto& F: M) {
        if (F.isDeclaration())
            continue;
        
        std::string name = "@@" + "func" + std::to_string(counter) + "@@"
        F.setName(name)

        // if (!F.getName().contains(mFunctionName))
        //     continue;

        // for (inst_iterator i = inst_begin(F); i != inst_end(F); ++i) {
        //     createWindows(i, inst_begin(F), inst_end(F));
        // }
    }
    return false;
}

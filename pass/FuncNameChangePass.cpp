#include "FuncNameChangePass.h"

#include <string>
#include <algorithm>
#include <fstream>
#include <memory>
#include <json/json.h>

#include "llvm/IR/Function.h"

using namespace llvm;

FuncNameChangePass::FuncNameChangePass(std::string file)
    : mOutputFile(file)
{}

PreservedAnalyses FuncNameChangePass::run(Module& M, ModuleAnalysisManager&)
{
    return runOnModule(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool FuncNameChangePass::runOnModule(Module& M)
{
    Json::Value root;

    int counter = 0;
    bool bModified = false;
    for (auto& F: M) {
        if (F.isDeclaration())
            continue;
        
        std::string oldname = std::string(F.getName());
        std::string newname = "COMBOTESTFUN" + std::to_string(counter);
        F.setName(newname);
        bModified = true;

        root[newname] = oldname;
        ++counter;
    }
    
    mOutputFile << root;
    return bModified;
}

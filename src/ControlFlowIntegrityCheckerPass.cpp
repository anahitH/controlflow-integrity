#include "ControlFlowIntegrityCheckerPass.h"
#include "FunctionsAnnotatorPass.h"

#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"

namespace cf_integrity {
char ControlFlowIntegrityCheckerPass::ID = 0;

void ControlFlowIntegrityCheckerPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.setPreservesAll();
    AU.addRequired<FunctionsAnnotatorPass>();
}

bool ControlFlowIntegrityCheckerPass::runOnFunction(llvm::Function &F)
{
    functions_annotator = &getAnalysis<FunctionsAnnotatorPass>();
    if (!functions_annotator->is_function_annotated(&F)) {
        return false;
    }

    // Get the function to call from our runtime library.
    llvm::LLVMContext &Ctx = F.getContext();
    llvm::Constant *checkFunc = F.getParent()->getOrInsertFunction(
            "check", llvm::Type::getVoidTy(Ctx), NULL);

    llvm::BasicBlock &bb = F.getEntryBlock();
    llvm::IRBuilder<> builder(&bb);
    builder.SetInsertPoint(&bb, --builder.GetInsertPoint());

    llvm::Value* args[] = {};
    builder.CreateCall(checkFunc);

    return true;
}

static llvm::RegisterPass<ControlFlowIntegrityCheckerPass> X("cf-integrity","Control flow integrity checkers inserter");
}



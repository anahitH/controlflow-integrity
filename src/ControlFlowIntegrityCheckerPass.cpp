#include "ControlFlowIntegrityCheckerPass.h"
#include "FunctionsAnnotatorPass.h"
#include "CallPathsAnalysisPass.h"

#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"

namespace cf_integrity {
char ControlFlowIntegrityCheckerPass::ID = 0;

void ControlFlowIntegrityCheckerPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.setPreservesAll();
    AU.addRequired<CallPathsAnalysisPass>();
    AU.addRequired<FunctionsAnnotatorPass>();
}

bool ControlFlowIntegrityCheckerPass::runOnFunction(llvm::Function &F)
{
    const auto& functions_annotator = getAnalysis<FunctionsAnnotatorPass>();
    if (!functions_annotator.is_function_annotated(&F)) {
        return false;
    }
    if (F.isDeclaration()) {
        return false;
    }
    const auto& call_paths_analysis = getAnalysis<CallPathsAnalysisPass>();

    // Get the function to call from our runtime library.
    llvm::LLVMContext &Ctx = F.getContext();
    llvm::ArrayRef<llvm::Type*> params{llvm::Type::getInt8PtrTy(Ctx), llvm::Type::getInt32Ty(Ctx)};
    llvm::FunctionType* function_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(Ctx), params, true);
    llvm::Constant *checkFunc = F.getParent()->getOrInsertFunction(
            "check", function_type);

    llvm::BasicBlock &bb = F.getEntryBlock();
    llvm::IRBuilder<> builder(&bb);
    builder.SetInsertPoint(&bb, --builder.GetInsertPoint());

    std::vector<llvm::Value*> arg_values;
    const auto& hashes = call_paths_analysis.get_function_call_path_hashes(&F);
    llvm::Value *strPtr = builder.CreateGlobalStringPtr(F.getParent()->getName(), "module_name");
    arg_values.push_back(strPtr);
    arg_values.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), hashes.size()));
    for (const auto& hash : hashes) {
        arg_values.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(Ctx), hash));
    }
    llvm::ArrayRef<llvm::Value*> args(arg_values);
    builder.CreateCall(checkFunc, args);

    return true;
}

static llvm::RegisterPass<ControlFlowIntegrityCheckerPass> X("cf-integrity","Control flow integrity checkers inserter");
}



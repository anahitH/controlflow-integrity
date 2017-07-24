#include "FunctionsAnnotatorPass.h"

#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"

namespace cf_integrity {

char FunctionsAnnotatorPass::ID = 0;

bool FunctionsAnnotatorPass::runOnModule(llvm::Module &M)
{
    llvm::dbgs() << "PASS\n";
    annotate(M);
    for (auto& F : M) {
        if (F.hasFnAttribute("protected")) {
            llvm::dbgs() << "Add sensitive function " << F.getName() << "\n";
            annotated_functions.insert(&F);
        } else {
            //llvm::errs() << "Not Protected\n";
        }
    }

    return false;
}

bool FunctionsAnnotatorPass::is_function_annotated(llvm::Function* F) const
{
    return annotated_functions.find(F) != annotated_functions.end();
}

bool FunctionsAnnotatorPass::is_function_annotated(const llvm::Function* F) const
{
    return is_function_annotated(const_cast<llvm::Function*>(F));
}

void FunctionsAnnotatorPass::annotate(llvm::Module &M)
{
    // Setup all the function annotations
    auto global_annos = M.getNamedGlobal("llvm.global.annotations");
    if (!global_annos) {
        return;
    }
    auto annot = llvm::cast<llvm::ConstantArray>(global_annos->getOperand(0));
    for (unsigned i = 0; i < annot->getNumOperands(); ++i) {
        auto e = llvm::cast<llvm::ConstantStruct>(annot->getOperand(i));

        if (auto fn = llvm::dyn_cast<llvm::Function>(e->getOperand(0)->getOperand(0))) {
            auto anno = llvm::cast<llvm::ConstantDataArray>(llvm::cast<llvm::GlobalVariable>(
                                    e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
            fn->addFnAttr(anno); // <-- add function annotation here
        }
    }
}

static llvm::RegisterPass<FunctionsAnnotatorPass> X("annotate-functions","Control flow integrity checkers inserter");

}



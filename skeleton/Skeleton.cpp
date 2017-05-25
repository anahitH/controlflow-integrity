#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool doInitialization(Module &M) {
      // Setup all the function annotations
      auto global_annos = M.getNamedGlobal("llvm.global.annotations");
      if (global_annos) {
        auto a = cast<ConstantArray>(global_annos->getOperand(0));
        for (int i=0; i<a->getNumOperands(); i++) {
          auto e = cast<ConstantStruct>(a->getOperand(i));

          if (auto fn = dyn_cast<Function>(e->getOperand(0)->getOperand(0))) {
            auto anno = cast<ConstantDataArray>(cast<GlobalVariable>(e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
            fn->addFnAttr(anno); // <-- add function annotation here
          }
        }
      }
    }

    virtual bool runOnFunction(Function &F) {
      // Check if function is protected
      if (!F.hasFnAttribute("protected")) {
        errs() << "Not Protected\n";
        return false;
      }
      errs() << "Protected\n";

      // Get the function to call from our runtime library.
      LLVMContext &Ctx = F.getContext();
      Constant *checkFunc = F.getParent()->getOrInsertFunction(
        "check", Type::getVoidTy(Ctx), NULL
      );

      BasicBlock &bb = F.getEntryBlock();
      IRBuilder<> builder(&bb);
      builder.SetInsertPoint(&bb, --builder.GetInsertPoint());

      Value* args[] = {};
      builder.CreateCall(checkFunc);

      return true;
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);

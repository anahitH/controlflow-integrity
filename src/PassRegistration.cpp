
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "CallPathsAnalysisPass.h"
#include "ControlFlowIntegrityCheckerPass.h"
#include "FunctionsAnnotatorPass.h"

namespace cf_integrity {

static void registerPathsAnalysisPass(const llvm::PassManagerBuilder &,
                         	      llvm::legacy::PassManagerBase &PM) {
  //PM.add(new CallPathsAnalysisPass());
  PM.add(new ControlFlowIntegrityCheckerPass());
  PM.add(new FunctionsAnnotatorPass());
}

static llvm::RegisterStandardPasses RegisterMyPass(llvm::PassManagerBuilder::EP_EarlyAsPossible, registerPathsAnalysisPass);

}



#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "CallPathsAnalysisPass.h"

namespace cf_integrity {

static void registerPathsAnalysisPass(const llvm::PassManagerBuilder &,
                         	      llvm::legacy::PassManagerBase &PM) {
  CallPathsAnalysisPass* pathsAnalysisPass = new CallPathsAnalysisPass();
  pathsAnalysisPass->set_sensitive_functions(std::vector<std::string>{"dummy", "test"});
  PM.add(pathsAnalysisPass);
}

static llvm::RegisterStandardPasses RegisterMyPass(llvm::PassManagerBuilder::EP_EarlyAsPossible, registerPathsAnalysisPass);

}


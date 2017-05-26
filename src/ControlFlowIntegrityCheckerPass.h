#include "llvm/Pass.h"

namespace llvm {
class Function;
class Module;
}

namespace cf_integrity {
class FunctionsAnnotatorPass;

class ControlFlowIntegrityCheckerPass : public llvm::FunctionPass
{
public:
    static char ID;

    ControlFlowIntegrityCheckerPass()
        : llvm::FunctionPass(ID)
    {
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;
    virtual bool runOnFunction(llvm::Function &F) override;

private:
    FunctionsAnnotatorPass* functions_annotator;
};

}


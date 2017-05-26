#include "llvm/Pass.h"

#include <unordered_set>

namespace llvm {
class Module;
}

namespace cf_integrity {

class FunctionsAnnotatorPass : public llvm::ModulePass
{
public:
    static char ID;

    FunctionsAnnotatorPass()
        : llvm::ModulePass(ID)
    {
    }

    virtual bool runOnModule(llvm::Module &M) override;

public:
    bool is_function_annotated(llvm::Function* F) const;
    bool is_function_annotated(const llvm::Function* F) const;

private:
    void annotate(llvm::Module& M);

private:
    std::unordered_set<llvm::Function*> annotated_functions;
};

}


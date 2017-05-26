#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraphSCCPass.h"

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <vector>
#include <unordered_set>


class TestModuleSCCPass : public llvm::ModulePass
{
public:
    static char ID;

    TestModuleSCCPass()
        : llvm::ModulePass(ID)
    {
    }

public:
    bool runOnModule(llvm::Module& M) override
    {
        llvm::CallGraph &CG = getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();
        CG.dump();

        llvm::scc_iterator<llvm::CallGraph*> CGI = llvm::scc_begin(&CG);
        llvm::CallGraphSCC CurSCC(CG, &CGI);
        while (!CGI.isAtEnd()) {
            // Copy the current SCC and increment past it so that the pass can hack
            // on the SCC if it wants to without invalidating our iterator.
            const std::vector<llvm::CallGraphNode *> &NodeVec = *CGI;
            CurSCC.initialize(NodeVec.data(), NodeVec.data() + NodeVec.size());

            for (llvm::CallGraphNode* node : CurSCC) {
                llvm::Function* F = node->getFunction();
                if (F == nullptr || F->isDeclaration()) {
                    continue;
                }
                llvm::dbgs() << "Function " << F->getName() << "\n";
            }

            ++CGI;
        }
        return false;
    }

    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override
    {
        AU.addRequired<llvm::CallGraphWrapperPass>();
        AU.setPreservesCFG();
        AU.setPreservesAll();
    }
};

char TestModuleSCCPass::ID = 0;

static llvm::RegisterPass<TestModuleSCCPass> X("test-modulescc","TEST");


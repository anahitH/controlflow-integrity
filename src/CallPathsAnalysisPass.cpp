#include "CallPathsAnalysisPass.h"
#include "FunctionsAnnotatorPass.h"

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <cxxabi.h>

namespace cf_integrity {

namespace {

std::string demangle(const std::string& mangled_name)
{
    int status = -1;
    char* demangled = abi::__cxa_demangle(mangled_name.c_str(), NULL, NULL, &status);
    if (status == 0) {
        return std::string(demangled);
    } else {
        llvm::dbgs() << "Failed to demangle the name " << mangled_name << "\n";
    }
    return std::string();
}

}

CallPathsAnalysisPass::node::node(llvm::Function* f)
    : node_f(f)
{
}

void CallPathsAnalysisPass::node::add_parent(const node_type& parent)
{
    parents.push_back(parent);
}

const CallPathsAnalysisPass::node::node_parents& CallPathsAnalysisPass::node::get_parents() const
{
    return parents;
}

CallPathsAnalysisPass::CallPaths CallPathsAnalysisPass::node::get_flattened_paths()
{
    if (!flattened_paths.empty()) {
        return flattened_paths;
    }
    if (parents.empty()) {
        flattened_paths.push_back(CallPath{node_f});
        return flattened_paths;
    }
    for (auto& parent : parents) {
        auto parent_call_paths = parent->get_flattened_paths();
        for (auto& path : parent_call_paths) {
            path.push_front(node_f);
            flattened_paths.push_back(path);
        }
    }
    return flattened_paths;
}

char CallPathsAnalysisPass::ID = 0;


CallPathsAnalysisPass::CallPathsAnalysisPass()
    : llvm::ModulePass(ID)
{
}

bool CallPathsAnalysisPass::doFinalization(llvm::Module& M)
{
    llvm::dbgs() << "doFinalization\n";
    //compute_hashes();
    dump();
    intermediate_sensitive_functions.clear();
    return false;
}

bool CallPathsAnalysisPass::runOnModule(llvm::Module& M)
{
    collect_sensitive_functions(M);
    llvm::CallGraph& CG = getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();
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
            if (is_sensitive(F)) {
                assert(sensitive_functions.find(F) != sensitive_functions.end());
                assert(intermediate_sensitive_functions.find(F) != intermediate_sensitive_functions.end());
            }
            auto F_pos = intermediate_sensitive_functions.find(F);
            for (auto& callrecord : *node) {
                // callrecord - <callInst, CallGraphNode>
                llvm::Function* calledF = callrecord.second->getFunction();
                auto pos = intermediate_sensitive_functions.find(calledF);
                if (pos == intermediate_sensitive_functions.end()) {
                    continue;
                }
                if (F_pos == intermediate_sensitive_functions.end()) {
                    node_type new_sensitive(new CallPathsAnalysisPass::node(F));
                    intermediate_sensitive_functions.insert(std::make_pair(F, new_sensitive));
                    pos->second->add_parent(new_sensitive);
                    F_pos = intermediate_sensitive_functions.find(F);
                } else {
                    pos->second->add_parent(F_pos->second);
                }
            }
        }
        ++CGI;
    }
    compute_hashes();
    return false;
}

void CallPathsAnalysisPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.setPreservesCFG();
    AU.setPreservesAll();
    AU.addRequired<FunctionsAnnotatorPass>();
    AU.addRequired<llvm::CallGraphWrapperPass>();
    AU.addPreserved<llvm::CallGraphWrapperPass>();
}

void CallPathsAnalysisPass::dump()
{
    //flatten_paths();
    for (auto& item : sensitive_functions) {
        CallPaths paths;
        const auto& pos = flattened_paths.find(item.first);
        if (pos == flattened_paths.end()) {
            paths = item.second->get_flattened_paths();
            flattened_paths[item.first] = paths;
        } else {
            paths = pos->second;
        }
        llvm::dbgs() << "Function " << item.first->getName() << "\n";
        dump(paths);
        llvm::dbgs() << "Hashes:\n";
        for (const auto& hash : path_hashes[item.first]) {
            llvm::dbgs() << hash << "   ";
        }
        llvm::dbgs() << "\n";
    }
}

void CallPathsAnalysisPass::collect_sensitive_functions(llvm::Module& M)
{
    const auto& Fannotator = getAnalysis<FunctionsAnnotatorPass>();
    for (auto& F : M) {
        if (Fannotator.is_function_annotated(&F)) {
            node_type f_node(new node(&F));
            sensitive_functions.insert(std::make_pair(&F, f_node));
            intermediate_sensitive_functions.insert(std::make_pair(&F, f_node));
        }
    }
    llvm::dbgs() << "doInitialization\n";
}

bool CallPathsAnalysisPass::is_sensitive(llvm::Function* F) const
{
    return sensitive_functions.find(F) != sensitive_functions.end();
}

void CallPathsAnalysisPass::flatten_paths()
{
    if (!flattened_paths.empty()) {
        return;
    }

    for (auto& item : sensitive_functions) {
        CallPaths paths;
        const auto& pos = flattened_paths.find(item.first);
        if (pos == flattened_paths.end()) {
            paths = item.second->get_flattened_paths();
            flattened_paths[item.first] = paths;
        } else {
            paths = pos->second;
        }
    }
}

void CallPathsAnalysisPass::compute_hashes()
{
    flatten_paths();
    for (const auto& function_paths : flattened_paths) {
        auto& function_hashes = path_hashes[function_paths.first];
        for (const auto& path : function_paths.second) {
            function_hashes.insert(compute_hash(path));
        }
    }
}

size_t CallPathsAnalysisPass::compute_hash(const CallPath& path) const
{
    size_t hash = 0;
    for (const auto& f : path) {
        const std::string& fname = f->getName();
        hash ^= std::hash<std::string>()(fname);
    }
    return hash;
}

void CallPathsAnalysisPass::dump(CallPaths paths) const
{
    llvm::dbgs() << "*****\n";
    for (const auto& path : paths) {
        for (const auto& f : path) {
            llvm::dbgs() << f->getName() << "   ";
        }
        llvm::dbgs() << "\n";
    }
    llvm::dbgs() << "*****\n";
}

static llvm::RegisterPass<CallPathsAnalysisPass> X("call-paths","Collects call paths for the given functions");
}


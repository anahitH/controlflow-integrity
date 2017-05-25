#include "CallPathsAnalysisPass.h"

#include "llvm/ADT/SCCIterator.h"
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
    : llvm::CallGraphSCCPass(ID)
{
}

void CallPathsAnalysisPass::set_sensitive_functions(const std::vector<std::string>& functions)
{
    sensitive_function_names.clear();
    sensitive_function_names.insert(functions.begin(), functions.end());
}

bool CallPathsAnalysisPass::doInitialization(llvm::CallGraph &CG)
{
    sensitive_function_names.insert("licence_check()");
    llvm::dbgs() << "doInitialization\n";
}

bool CallPathsAnalysisPass::doFinalization(llvm::CallGraph &CG)
{
    llvm::dbgs() << "doFinalization\n";
    dump();
    intermediate_sensitive_functions.clear();
}

bool CallPathsAnalysisPass::runOnSCC(llvm::CallGraphSCC& SCC)
{
    for (llvm::CallGraphNode* node : SCC) {
        llvm::Function* F = node->getFunction();
        if (F == nullptr) {
            continue;
        }
        if (is_sensitive(F)) {
            add_new_sensitive_functions(F);
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
            } else {
                pos->second->add_parent(F_pos->second);
            }
        }
    }
    return false;
}

void CallPathsAnalysisPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.setPreservesCFG();
    AU.setPreservesAll();
    llvm::CallGraphSCCPass::getAnalysisUsage(AU);
    AU.addPreserved<llvm::CallGraphWrapperPass>();
}

void CallPathsAnalysisPass::dump()
{
    for (auto& item : sensitive_functions) {
        CallPaths paths;
        const auto& pos = flattened_paths.find(item.first);
        if (pos == flattened_paths.end()) {
            paths = item.second->get_flattened_paths();
            flattened_paths[item.first] = paths;
        } else {
            paths = pos->second;
        }
        dump(paths);
    }
}

bool CallPathsAnalysisPass::is_sensitive(llvm::Function* F) const
{
    const auto& demangled_name = demangle(F->getName());
    return sensitive_function_names.find(demangled_name) != sensitive_function_names.end();
}

void CallPathsAnalysisPass::add_new_sensitive_functions(llvm::Function* F)
{
    // sensitive function is just hit in call graph, thus it should not be added
    assert(sensitive_functions.find(F) == sensitive_functions.end());
    // If F was called from another sensitive function, it should have been already processed.
    assert(intermediate_sensitive_functions.find(F) == intermediate_sensitive_functions.end());
    node_type f_node(new node(F));
    sensitive_functions.insert(std::make_pair(F, f_node));
    intermediate_sensitive_functions.insert(std::make_pair(F, f_node));
}

void CallPathsAnalysisPass::flatten_path(llvm::Function* F)
{
    if (!flattened_paths.empty()) {
        return;
    }
    auto pos = sensitive_functions.find(F);
    assert(pos != sensitive_functions.end());
    flattened_paths[F] = pos->second->get_flattened_paths();
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


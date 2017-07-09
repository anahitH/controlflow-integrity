#include "llvm/Pass.h"

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace llvm
{
class CallGraphSCC;
class CallGraphSCCPass;
class Function;
}

namespace cf_integrity
{
class CallPathsAnalysisPass : public llvm::ModulePass
{
public:
    using CallPath = std::list<llvm::Function*>;
    using CallPaths = std::vector<CallPath>;
    using hash_set = std::unordered_set<size_t>;

public:
    class node;
    using node_type = std::shared_ptr<node>;
    
    class node
    {
    public:
        using node_parents = std::vector<node_type>;

    public:
        node(llvm::Function* f);

        llvm::Function* get_function() const;

        void add_parent(node_type parent);
        const node_parents& get_parents() const;
        bool has_parent(llvm::Function* F) const;

        CallPaths get_flattened_paths();

    private:
        llvm::Function* node_f;
        node_parents parents;
        CallPaths flattened_paths; // cach
    };
        
public:
    static char ID;

    CallPathsAnalysisPass();

public:
    bool doFinalization(llvm::Module &M) override;
    bool runOnModule(llvm::Module& M) override;
    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;


public:
    const hash_set& get_function_call_path_hashes(llvm::Function* F) const;
    void dump();

private:
    void collect_sensitive_functions(llvm::Module& M);
    bool is_sensitive(llvm::Function* F) const;
    void flatten_paths();
    void compute_hashes();
    size_t compute_hash(const CallPath& path) const;
    void dump(CallPaths paths) const;

private:
    std::unordered_map<llvm::Function*, node_type> sensitive_functions;
    std::unordered_map<llvm::Function*, CallPaths> flattened_paths;

    std::unordered_map<llvm::Function*, node_type> intermediate_sensitive_functions;
    std::unordered_map<llvm::Function*, hash_set> path_hashes;

};
} // cf_integrity


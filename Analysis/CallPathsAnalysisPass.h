#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraphSCCPass.h"

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
class CallPathsAnalysisPass : public llvm::CallGraphSCCPass
{
public:
    using CallPath = std::list<llvm::Function*>;
    using CallPaths = std::vector<CallPath>;

public:
    class node;
    using node_type = std::shared_ptr<node>;
    
    class node
    {
    public:
        using node_parents = std::vector<node_type>;

    public:
        node(llvm::Function* f);

        void add_parent(const node_type& parent);
        const node_parents& get_parents() const;

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
    void set_sensitive_functions(const std::vector<std::string>& function_names);

public:
    bool doInitialization(llvm::CallGraph &CG)override;
    bool doFinalization(llvm::CallGraph &CG) override;
    bool runOnSCC(llvm::CallGraphSCC& SCC) override;
    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;

    void dump();

private:
    bool is_sensitive(llvm::Function* F) const;
    void add_new_sensitive_functions(llvm::Function* F);
    void flatten_path(llvm::Function* F);
    void dump(CallPaths paths) const;

private:
    std::unordered_set<std::string> sensitive_function_names;
    std::unordered_map<llvm::Function*, node_type> sensitive_functions;
    std::unordered_map<llvm::Function*, CallPaths> flattened_paths;

    std::unordered_map<llvm::Function*, node_type> intermediate_sensitive_functions;

};
} // cf_integrity


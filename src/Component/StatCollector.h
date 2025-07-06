// StatCollector.h
#include <mutex>
#include <vector>
struct StatRecord {
    int bf_before, bf_after;
    long long expanded_nodes;
    long long simulations;
    float root_value_prune;
    float root_value_noprune;
};

class StatCollector {
public:
    static StatCollector& Instance();

    void recordBranching(int before, int after);
    void incrementExpandedNodes(int delta);
    void incrementSimulations(int delta);
    void setRootValues(float prune_val, float noprune_val);
    void dumpToCSV(const std::string& path) const;

private:
    std::mutex _mtx;
    std::vector<StatRecord> _records;
    // 以下可根据需要存一行临时汇总，用完后 push_back
    StatRecord _current{};
};

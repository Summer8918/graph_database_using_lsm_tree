#include <iostream>
#include <map>
#include <vector>

using namespace std;

class Memtable {
    public:
        Memtable() {}
        ~Memtable() {}

        void insert_edge(uint key, uint edge_end) {
            auto search = hash_table.find(key);
            if (search != hash_table.end()) {
                hash_table[key].push_back(edge_end);
            } else {
                std::vector<uint> v = {edge_end};
                hash_table[key] = v;
            }
        }

        std::vector<uint> get_adjacent(uint key) {
            auto search = hash_table.find(key);
            if (search != hash_table.end()) {
                return hash_table[key];
            } else {
                return std::vector<uint>();
            }
        }

    private:
        std::map<uint, std::vector<uint>> hash_table;
};

int main() {
    Memtable* mem = new Memtable();
    mem->insert_edge(1,2);
    mem->insert_edge(1,3);
    mem->insert_edge(2,3);
    std::vector<uint> adjacent_1 = mem->get_adjacent(1);
    std::vector<uint> adjacent_2 = mem->get_adjacent(2);

    std::cout << "Adjacent to 1:" << std::endl;
    for (auto a_1 : adjacent_1) {
        std::cout << a_1 << " ";
    }
    std::cout << std::endl;

    std::cout << "Adjacent to 2:" << std::endl;
    for (auto a_2 : adjacent_2) {
        std::cout << a_2 << " ";
    }
    std::cout << std::endl;

    return 0;
}
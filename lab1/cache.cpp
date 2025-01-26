#include <bits/stdc++.h>

using namespace std;

char get_hex(int i) {
    return (i > 9) ? ('A' + (i - 10)) : ('0' + i); // Adjusted i - 10 for proper hex calculation
}

string get_tag(string address) {
    return address.substr(0, 3); // Use substr() to get the first 3 characters
}

int get_offset(string address) {
    return stoi(address.substr(3, 1), nullptr, 16); // Use stoi() with base 16 to convert hex to int
}

class Block {
public:
    string tag;
    int size = 16;
    vector<string> words = vector<string>(size);

    Block() {
        this->tag = "";
    }

    Block(string tag) {
        this->tag = tag;
        for (int i = 0; i < this->size; i++) {
            char last_bit = get_hex(i);
            this->words[i] = tag + last_bit; // Concatenate to form word address
        }
    }

    void print() {
        cout << "Tag: " << this->tag << endl;
        for (const auto& word : this->words) {
            cout << word << " ";
        }
        cout << endl;
    }
};

class RAM {
private:
    map<string, Block> block_map; // Map tags to blocks

public:
    // Constructor: Assign address for blocks from 0000H to FFFFH
    RAM() {
        for (int i = 0; i < 16; i++) {
            char first_bit = get_hex(i);
            for (int j = 0; j < 16; j++) {
                char second_bit = get_hex(j);
                for (int k = 0; k < 16; k++) {
                    char third_bit = get_hex(k);
                    string tag = string(1, first_bit) + second_bit + third_bit; // Proper string concatenation
                    block_map.emplace(tag, Block(tag)); // Use emplace to avoid unnecessary copy
                }
            }
        }
    }

    void print_all() {
        for (const auto& [tag, block] : block_map) {
            //print tag
            cout << "Tag: " << tag << endl;
        }
    }

    const Block& get_block(const string& tag) {
        return block_map.at(tag); // Use at() for bounds-checked access
    }
};

RAM ram;

class CacheBlock {
    private:
        Block block;
        bool valid;
        bool dirty;
        int last_used;

    public:
        CacheBlock() {
            
            this->valid = false;
            this->dirty = false;
            this->last_used = 0;
        }

        void print() {
            cout << this->valid << " " << this->dirty << " " << this->last_used << endl;
        }

        void set_block(const Block& block) {
            this->block = block;
            this->valid = true;
            this->dirty = false;
            this->last_used = 0;
        }

        void set_invalid() {
            this->valid = false;
        }

        friend class Cache;
};

class Cache {
    private:
        int size = 128;
        vector<CacheBlock> cache_blocks;

    public:
        Cache() {
            this->cache_blocks = vector<CacheBlock>(this->size);
        }

        // void print_all() {
        //     for (const auto& cache_block : cache_blocks) {
        //         cache_block.print();
        //     }
        // }

        void fetch_RAM(string address) {
            string tag = get_tag(address);
            Block block = ram.get_block(tag);
            
            for (int i = 0; i < this->size; i++) {
                if (!cache_blocks[i].valid) {
                    cache_blocks[i].set_block(block);
                    break;
                }
            }
        }

        void search(string address) {
            string tag = get_tag(address);
            bool miss = true;
            for (int i = 0; i < this->size; i++) {
                if (cache_blocks[i].valid && cache_blocks[i].block.tag == tag) {
                    miss = false;
                    cout << "Cache hit" << endl;
                    return;
                }
            }
            if(miss) {
                cout << "Cache miss" << endl;
                fetch_RAM(address);
                
            }
        }

        void evict_block();
};

int main() {
    Cache cache;

    string address = "F23BH";
    cout << get_tag(address) << " " << get_offset(address) << endl;
    return 0;
}

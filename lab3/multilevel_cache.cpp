//CS22B1060 Rohan Shenoy
//CS22B1083 Dharun Thota

#include <bits/stdc++.h>
using namespace std;

#define WORD_SIZE 8 //bytes
#define L1_MAX_BLOCKS 128 //blocks
#define BLOCK_SIZE 16 //words
#define RAM_SIZE 65536 //words

//searches mean going through every valid block in the cache
int searches=0, hits=0, misses=0;
int l1_hits=0, l1_misses=0, l2_hits=0, l2_misses=0;

class SetAssociativeCache;

char get_hex(int i) {
    return (i > 9) ? ('A' + (i - 10)) : ('0' + i); // Adjusted i - 10 for proper hex calculation
}

string get_bits(string address, int start, int end) {
    return address.substr(start, end - start + 1);
}

string get_binary(string address){
    string binary = "";
    for(int i=0;i<address.length();i++){
        switch(address[i]){
            case '0':
                binary += "0000";
                break;
            case '1':
                binary += "0001";
                break;
            case '2':
                binary += "0010";
                break;
            case '3':
                binary += "0011";
                break;
            case '4':
                binary += "0100";
                break;
            case '5':
                binary += "0101";
                break;
            case '6':
                binary += "0110";
                break;
            case '7':
                binary += "0111";
                break;
            case '8':
                binary += "1000";
                break;
            case '9':
                binary += "1001";
                break;
            case 'A':
                binary += "1010";
                break;
            case 'B':
                binary += "1011";
                break;
            case 'C':
                binary += "1100";
                break;
            case 'D':
                binary += "1101";
                break;
            case 'E':
                binary += "1110";
                break;
            case 'F':
                binary += "1111";
                break;
        }
    }
    
    return binary;
}

int get_offset(string address) {
    return stoi(address.substr(3, 1), nullptr, 16); // Use stoi() with base 16 to convert hex to int
}

class Block {
public:
    string tag;
    int size = BLOCK_SIZE;
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
    int ram_blocks = RAM_SIZE / BLOCK_SIZE; //4096

public:
    // Constructor: Assign address for blocks from 0000H to FFFFH
    RAM() {
        for (int i = 0; i < 16; i++) {
            // each bit is actually a nibble
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
            //this->last_used = 0;
        }

        void set_invalid() {
            this->valid = false;
        }

        friend class Cache;
        friend class DirectMappedCache;
        friend class SetBlock;
        friend class SetAssociativeCache;
};

// class Cache {
//     private:

//         int size = CACHE_MAX_SIZE;
//         vector<CacheBlock> cache_blocks;

//         void evict_block(int index) {
//             //just remove the index block
//             cache_blocks[index].set_invalid();
//         }

//         void update_LRU(int index) {
//             cache_blocks[index].last_used = 0;
//             for (int i = 0; i < this->size; i++) {
//                 if (i != index && cache_blocks[i].valid) {
//                     cache_blocks[i].last_used++;
//                 }
//             }
//         }

//     public:
//         Cache() {
//             this->cache_blocks = vector<CacheBlock>(this->size);
//         }

//         // void print_all() {
//         //     for (const auto& cache_block : cache_blocks) {
//         //         cache_block.print();
//         //     }
//         // }

//         //fetches the block from RAM and stores it in the cache
//         //if the cache is full, evict the LRU block
//         //if the block is already in the cache, update the LRU
//         void fetch_RAM(string address) {
//             string tag = get_tag(address);
//             Block block = ram.get_block(tag);
            
//             for (int i = 0; i < this->size; i++) {
//                 if (!cache_blocks[i].valid) {
//                     cache_blocks[i].set_block(block);
//                     //printf("Block %s fetched from RAM\n", tag.c_str());
//                     update_LRU(i);
//                     return;
//                 }
//             }
//             //LRU
//             int remove_index = LRU();
//             evict_block(remove_index);
//             cache_blocks[remove_index].set_block(block);
//             update_LRU(remove_index);
//             //printf("Cache block %d evicted, Block %s fetched from RAM\n", remove_index, tag.c_str());
//         }

//         void search(string address) {
//             string tag = get_tag(address);
//             bool miss = true;
//             for (int i = 0; i < this->size; i++) {
//                 if (cache_blocks[i].valid) {
//                     searches++;
//                     if(cache_blocks[i].block.tag == tag){
//                         miss = false;
//                         //cout << "Cache hit, updating " << i <<"th block" << endl;
//                         hits++;
//                         update_LRU(i);
//                         return;
//                     }
//                 }
//             }
//             if(miss) {
//                 //cout << "Cache miss" << endl;
//                 misses++;
//                 fetch_RAM(address);
                
//             }
//         }

//         //returns the oldest block
//         int LRU() {
//             int max = 0;
//             int index = 0;
//             for (int i = 0; i < this->size; i++) {
//                 if (cache_blocks[i].valid && cache_blocks[i].last_used > max) {
//                     max = cache_blocks[i].last_used;
//                     index = i;
//                 }
//             }
//             return index;
//         }

//         void clear() {
//             for (int i = 0; i < this->size; i++) {
//                 cache_blocks[i].set_invalid();
//             }
//         }
// };
// Cache cache;


class SetBlock{
    int size;
    vector<CacheBlock> blocks;

    string get_tag(string address){
        return address.substr(0, 3);
    }

    public:
        SetBlock(int size=4){
            this->size = size;
            this->blocks = vector<CacheBlock>(size);
        }

        Block fetch_RAM(string address){
            string tag = get_tag(address);
            Block block = ram.get_block(tag);
            cout << "Block fetched from RAM" << endl;
            for(int i=0;i<this->size;i++){
                if(!blocks[i].valid){
                    blocks[i].set_block(block);
                    update_LRU(i);
                    return block;
                }
            }
            int remove_index = LRU();
            evict_block(remove_index);
            blocks[remove_index].set_block(block);
            update_LRU(remove_index);
            return blocks[remove_index].block;
        }

        void update_LRU(int index){
            blocks[index].last_used = 0;
            for(int i=0;i<this->size;i++){
                if(i != index && blocks[i].valid){
                    blocks[i].last_used++;
                }
            }
        }

        void evict_block(int index){
            blocks[index].set_invalid();
        }

        int LRU(){
            int max = 0;
            int index = 0;
            for(int i=0;i<this->size;i++){
                if(blocks[i].valid && blocks[i].last_used > max){
                    max = blocks[i].last_used;
                    index = i;
                }
            }
            return index;
        }

        friend class SetAssociativeCache;
};

class SetAssociativeCache{
    int set_block_size = 4;
    int size = 256;
    vector<SetBlock> set_blocks;

    string get_tag(string address){
        return get_bits(address, 0, 3);
    }

    int get_index(string address){
        return stoi(get_bits(address, 4, 11), nullptr, 2);
    }

    public:
        SetAssociativeCache(){
            this->set_blocks = vector<SetBlock>(this->size);
        }

        void test(){
            string address = "0F0F";
            string bin = get_binary(address);
            string tag = this->get_tag(bin);
            int index = get_index(bin);
            cout << "Tag: " << tag << endl;
            cout << "Index: " << index << endl;
        }

        Block search(string nib_addr){
            string address = get_binary(nib_addr);
            string tag = get_tag(nib_addr);
            int index = get_index(address);
            //search for the block in the cache
            for(int i=0;i<set_block_size;i++){
                string tag_bin = get_tag(get_binary(set_blocks[index].blocks[i].block.tag));
                if(set_blocks[index].blocks[i].valid && tag_bin == tag){
                    //if the block is found, update the LRU
                    cout << "L2 Cache hit" << endl;
                    l2_hits++;
                    set_blocks[index].update_LRU(i);
                    return set_blocks[index].blocks[i].block;
                }
            }
            //if the block is not found, fetch the block from RAM
            cout << "L2 Cache miss" << endl;
            l2_misses++;
            return set_blocks[index].fetch_RAM(nib_addr);
            //if the cache is full, evict the LRU block
        }
};
SetAssociativeCache L2;

class VictimCache{
    vector<Block> blocks;
    int size = 4;
    int fifo_index; 

    string get_tag(string address){
        return address.substr(0, 3);
    }

    public:
        VictimCache(){
            this->blocks = vector<Block>(this->size);
            this->fifo_index = 0;
        }

        void add_block(Block block){
            blocks[fifo_index] = block;
            fifo_index = (fifo_index + 1) % size;
        }

        Block search(string nib_addr){
            string address = get_binary(nib_addr);
            string tag = get_tag(nib_addr);
            for(int i=0;i<this->size;i++){
                cout << "extracted tag: " << tag << " actual tag: " << blocks[i].tag << endl; 
                if(blocks[i].tag == tag){
                    cout << "Victim Cache hit" << endl;
                    return blocks[i];
                }
            }
            cout << "Victim Cache miss" << endl;
            //fetch the block from L2 cache
            return L2.search(nib_addr);
        }

        friend class DirectMappedCache;
};
VictimCache VC;

class DirectMappedCache {
    int size;
    vector<CacheBlock> cache_blocks;

    // l1 address structure  [tag 0:3][block index 5:11][word offset 12:15]

    string get_tag(string address){
        return get_bits(address, 0, 4);
    }
    int get_block(string address){
        return stoi(get_bits(address, 5, 11), nullptr, 2);
    }

    /*
        gets the i+1th block address 
        cur_address - 4 nibble address
        next_address - 4 nibble address with +1 to the 3rd nibble
        for ex: prefetch based on block tag
        0F0F -> 0F1F
          ^
          0 + 1 = 1

        edge case: if the block is the last block in the cache
        0FFF -> 100F
          ^ 
          F + 1 = 0 [ +1 carry ]
    */
    Block prefetch_after_address(string cur_address){
            //get first 3 bytes of the address
        string tag = cur_address.substr(0, 3);
        cout << "Tag: " << tag << endl;
        int nextaddr_int = hexaddr_to_int(tag) + 1;
        string next_address = "";
        string next_address_tag = int_to_hexaddr(nextaddr_int);
        if(nextaddr_int == 4096)
            next_address = "0000";
        else
            next_address = next_address_tag + "0";

        return ram.get_block(next_address);
    }

    public:
        //constructor
        DirectMappedCache(int size = L1_MAX_BLOCKS){
            this->size = size;
            this->cache_blocks = vector<CacheBlock>(this->size);
        }
        
        //we need a function to search the cache
        void search(string nib_addr){
            //convert address to binary
            string address = get_binary(nib_addr);
            cout << "Address: " << address << endl;
            int block_index = get_block(address);
            cout << "Block Index: " << block_index << endl;
            string tag = get_tag(address); 
            cout << "Tag: " << tag << endl;
            
            /* 
                now we need to search for the block in the cache
                direct mapping to access block using block_index
                if the block is valid and the tag matches, then it is a hit
                else it is a miss
                for misses we fetch from L2 cache
                NOTE: prefetch and stuff to be added later 
            */
           // ram has 12 bit tag but we need 4 bit tag
            string tag_bin = get_tag(get_binary(cache_blocks[block_index].block.tag));
            cout << "Tag in cache: " << tag_bin << endl;
            if(cache_blocks[block_index].valid &&  tag_bin == tag){
                cout << "L1 Cache hit" << endl;
                l1_hits++;
                return;
            } else{
                cout << "L1 Cache miss" << endl;
                l1_misses++;
                if(cache_blocks[block_index].valid && tag_bin != tag){
                    //if the block is valid but the tag doesn't match, then it is a conflict miss
                    //put this block in the victim cache
                    VC.add_block(cache_blocks[block_index].block);
                }
                
                //fetch 'Block' from L2 cache
                Block fetched_BL2 = VC.search(nib_addr);
                cache_blocks[block_index].set_block(fetched_BL2);
                cout << cache_blocks[block_index].valid << endl;
            }
            
        }
};


/*
    COMMON CLASS - StreamBuffer
    will be used for i) Instruction Stream Buffer ii) Data Stream Buffer
    
    i)  instruction stream buffer - for L1 cache
        will be used to prefetch instructions from RAM
        Prefetching technique - when the ith block is accessed, 
            the (i+1)th block is prefetched into ISB
    
    ii) data stream buffer - for L1 cache
        will be used to prefetch data from RAM
        Prefetching technique - when the ith block is accessed,
            the (i+1)th block is prefetched into DSB
            
    Both the ISB and DSB will have a size of 4 blocks
    eviction policy - FIFO (fifo_index)

*/
class StreamBuffer {
    private:
        vector<Block> blocks;
        int size;
        int fifo_index; 
        
    public:
        StreamBuffer(int size = 4){
            this->size = size;
            this->blocks = vector<Block>(size);
            this->fifo_index = 0;
        }
        
        void fetch_block(string address){
            //fetch the block from RAM
            Block block = ram.get_block(address);
            //store the block in the stream buffer
            blocks[fifo_index] = block;
            //update the fifo index
            fifo_index = (fifo_index + 1) % size;
        }

};

//prints the statistics
// void stats(){
//     double hit_rate = (double)hits / (hits + misses) * 100;
//     double miss_rate = (double)misses / (hits + misses) * 100;

//     cout << "Hits: " << hits << endl;
//     cout << "Misses: " << misses << endl;
//     cout << "Searches: " << searches << endl;
//     cout << "Hit rate: " << hit_rate << "%" << endl;
//     cout << "Miss rate: " << miss_rate << "%\n" << endl;

//     hits = 0;
//     misses = 0;
//     searches = 0;

//     cache.clear();
// }

// --tests--

/*
Random Test
This test is to show that the cache is able to handle random addresses
and fetch the blocks from RAM when needed
Number misses should be high as the addresses are random
*/
// void random_test(){
//     //test for random addresses

//     cout << "Random Test" << endl;

//     for (int i = 0; i < 256; i++) {
//         string address = "";
//         address += get_hex(rand() % 16);
//         address += get_hex(rand() % 16);
//         address += get_hex(rand() % 16);
//         address += get_hex(rand() % 16);

//         cache.search(address);
//     }

//     stats();
// }
/*
Spatial Locality Test

This test is to show that the cache is able to handle spatial locality
Number of misses should be low as the addresses are close to each other
Each miss should fetch a new block from RAM
*/
// void spatial_test(int prog_size=2048){
//     //test for showing spatial locality
//     int num_blocks = prog_size / (WORD_SIZE * BLOCK_SIZE);
//     //generate random tag

//     cout << "Spatial Locality Test" << endl;

//     for(int b=0;b<num_blocks;b++){
//         string rand_tag = "";
//         rand_tag += get_hex(rand() % 16);
//         rand_tag += get_hex(rand() % 16);
//         rand_tag += get_hex(rand() % 16);

//         for(int i=0;i<16;i++){
//             string address = rand_tag;
//             address += get_hex(i);

//             cache.search(address);
//         }
//     }

//     stats();
// }

/*
Temporal Locality Test

This test loops through the same addresses multiple times,
it first gets 3 random blocks from RAM and then loops through each word in the block 5 times
Number of misses should be low as the addresses are close to each other, and repeatedly accessed
*/

// void temporal_test(int num_loops=5){
//     //generate 3 random tags
//     string tags[3];
//     for(int i=0;i<3;i++){
//         tags[i] += get_hex(rand() % 16);
//         tags[i] += get_hex(rand() % 16);
//         tags[i] += get_hex(rand() % 16);
//     }

//     //generate 3 random addresses
//     string addresses[3];
//     for(int i=0;i<3;i++){
//         addresses[i] = tags[i];
//         addresses[i] += get_hex(rand() % 16);
//     }

//     //search for the 3 addresses
//     for(int i=0;i<3;i++){
//         cache.search(addresses[i]);
//     }

//     cout << "Temporal Localilty Test" << endl;

//     //loop through each word in the tags and search for them 5 times
//     for(int i=0;i<num_loops;i++){
//         for(int j=0;j<3;j++){
//             for(int k=0;k<16;k++){
//                 string address = tags[j];
//                 address += get_hex(k);

//                 cache.search(address);
//             }
//         }
//     }

//     stats();
// }

int main() {

    //use case example: word - 0F0FH
    //cache.search("0F0FH");

    // random_test();

    // spatial_test();

    // temporal_test();
    
    DirectMappedCache L1;
    //test to cjeck if an address is correctly fetched from yhe L1 cache, goes to l2 if not in l1
   L1.search("0000"); // 0000 0000 0000 0000
   L1.search("0001"); // 0000 0000 0000 0001
   L2.search("0000"); // 0000 0000 0000 0000
   L1.search("1000"); 
   L1.search("0000");

    return 0;
}

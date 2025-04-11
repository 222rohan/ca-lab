//CS22B1060 Rohan Shenoy
//CS22B1083 Dharun Thota

#include <bits/stdc++.h>
using namespace std;

#define WORD_SIZE 8 //bytes
#define L1_MAX_BLOCKS 128 //blocks
#define BLOCK_SIZE 16 //words
#define RAM_SIZE 65536 //words

/*

    STATISTICS
    variables to store the statistics of the entire memory system

    1) L1 Cache (direct mapped, hence no searches)
        - hits
        - misses
        - evicts (= conflict misses)
        - write buffer evicts
    2) L2 Cache
        - hits
        - misses
        - searches (number of valid blocks' words in the cache, as its a set associative cache)
    3) Victim Cache
        - hits
        - misses
    4) Instruction Stream Buffer
        - hits
        - misses
    5) Data Stream Buffer
        - hits
        - misses
    6) Write Buffer
        - flushes

*/
int l1_hits=0, l1_misses=0, l2_hits=0, l2_misses=0, l2_searches=0, vc_hits=0, vc_misses=0, isb_hits=0, isb_misses=0, dsb_hits=0, dsb_misses=0;
int l1_evicts=0, write_buffer_evicts=0;
int reads=0, writes=0, flushes=0;

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

int hexaddr_to_int(string hexaddr) {
    int intaddr = 0;
    for (int i = 0; i < hexaddr.length(); i++) {
        intaddr = intaddr * 16;
        if (hexaddr[i] >= '0' && hexaddr[i] <= '9') {
            intaddr += hexaddr[i] - '0';
        } else {
            intaddr += hexaddr[i] - 'A' + 10;
        }
    }
    return intaddr;
}

string int_to_hexaddr(int intaddr) {
    string hexaddr = "";
    while (intaddr > 0) {
        int rem = intaddr % 16;
        if (rem < 10) {
            hexaddr = (char)('0' + rem) + hexaddr;
        } else {
            hexaddr = (char)('A' + rem - 10) + hexaddr;
        }
        intaddr /= 16;
    }
    while (hexaddr.length() < 3) {
        hexaddr = '0' + hexaddr;
    }
    return hexaddr;
}

int get_offset(string address) {
    return stoi(address.substr(3, 1), nullptr, 16); // Use stoi() with base 16 to convert hex to int
}

/*

                                    +-------------+
                                    |  Processor  |
                                    +------+------+  
                                        │ - REQUEST ADDRESS
                                        ▼
                                    +-------------+
                                    |   L1 Cache  |   > DIRECT MAPPED 
                                    +------+------+  
                                        │
            EVICT TO ▼                  |          
            +----------------+          |                                                  
            |   Victim       |<---------|        
            |   Cache        │          |           ^PREFETCH FROM▼
            +----------------+          |────────► +-------------------+
                                        |          | Instruction Stream|
                                        |          | Buffer (ISB)      |
                                        |          +-------------------+  
                                        │              
                                        ├-───────► +-------------------+
           WRITE THROUGH▼               │          | Data Stream Buffer|
          +----------------+            │          | (DSB)             |
          |  Write Buffer  |<-----------│          +-------------------+
          |                |            │
          +----------------+            │
                                        ├
                                        │
                                        ▼
                                    +-------------+
                                    |   L2 Cache  |   > SET ASSOCIATIVE (4 WAY)
                                    +------+------+  
                                        │
                                        ▼
                                    +-------------+
                                    |    RAM      |   > BLOCK SIZE 16 WORDS
                                    +-------------+


*/

class Word {
    public: 
    string address;
    // string content;
    //int type; // 0 - instruction, 1 - data
};

class Block {
public:
    string tag;
    int size = BLOCK_SIZE;
    vector<Word> words = vector<Word>(size);
    int type;

    Block() {
        this->tag = "";
        this->type = 0;
    }

    Block(string tag) {
        this->tag = tag;
        for (int i = 0; i < this->size; i++) {
            char last_bit = get_hex(i);
            string addr = tag + last_bit; // Concatenate to form word address
            // Word word(addr, instr_type); // Initialize as instruction
            // this->words[i] = word;
            this->words[i].address = addr;
        }
        int instr_type = rand() % 2; // Randomly assign instruction or data
        this->type = instr_type;
    }

    void print() {
        cout << "Tag: " << this->tag << endl;
        for (const auto& word : this->words) {
            cout << word.address << " ";
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

    int get_block_type(const string& tag) {
        return block_map.at(tag).type;
    }

    void write_block(const Block& block) {
        block_map[block.tag] = block;
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
            // cout << "Block fetched from RAM" << endl;
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

        Block search(string nib_addr){
            string address = get_binary(nib_addr);
            string tag = get_tag(nib_addr);
            int index = get_index(address);
            //search for the block in the cache
            for(int i=0;i<set_block_size;i++){
                string tag_bin = get_tag(get_binary(set_blocks[index].blocks[i].block.tag));
                if(set_blocks[index].blocks[i].valid){
                    l2_searches++;
                    //if the block is found, update the LRU
                    if(tag_bin == tag){
                        // cout << "L2 Cache hit" << endl;
                        l2_hits++;
                        set_blocks[index].update_LRU(i);
                        return set_blocks[index].blocks[i].block;
                    }
                }
            }
            //if the block is not found, fetch the block from RAM
            // cout << "L2 Cache miss" << endl;
            l2_misses++;
            Block block = set_blocks[index].fetch_RAM(nib_addr);
            //if the cache is full, evict the LRU block

            return block;
        }
};
SetAssociativeCache L2;

/*

    Write Buffer
    we use Write back policy for the cache
    Store evicted dirty blocks of L1 in the write buffer
    and flush the entire write buffer to RAM (When the write buffer is full) 
*/
class WriteBuffer{
    vector<Block> blocks;
    int size;
    int fifo_index;

    public:
        WriteBuffer(int size = 4){
            this->size = size;
            this->blocks = vector<Block>(this->size);
            this->fifo_index = 0;
        }

        void add_block(Block block){
            blocks[fifo_index] = block;
            if(fifo_index == size-1){
                // cout << "Write Buffer full, updating in RAM" << endl;
                write_to_RAM();
                flushes++;
            }
            fifo_index = (fifo_index + 1) % size;
        }

        void write_to_RAM(){
            for(int i=0;i<this->size;i++){
                //write the block to RAM
                ram.write_block(blocks[i]);
                write_buffer_evicts++;
            }
        }

        friend class DirectMappedCache;
};
WriteBuffer WB;


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

        string get_tag(string address){
            return address.substr(0, 3);
        }

        StreamBuffer(int size = 4){
            this->size = size;
            this->blocks = vector<Block>(size);
            this->fifo_index = 0;
        }
        
        void add_block(Block block){
            blocks[fifo_index] = block;
            fifo_index = (fifo_index + 1) % size;
        }

        Block search(string nib_addr, int type){
            string address = get_binary(nib_addr);
            string tag = get_tag(nib_addr);
            for(int i=0;i<this->size;i++){
                if(blocks[i].tag == tag){
                    // cout << "Stream Buffer hit" << endl;
                    type == 1 ? dsb_hits++ : isb_hits++;
                    return blocks[i];
                }
            }
            // cout << "Stream Buffer miss" << endl;
            type == 1 ? dsb_misses++ : isb_misses++;
            //---------------fetch the block from L2 cache
            return L2.search(nib_addr);
        }

};
StreamBuffer ISB, DSB;

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
                if(blocks[i].tag == tag){
                    // cout << "Victim Cache hit" << endl;
                    vc_hits++;
                    return blocks[i];
                }
            }
            // cout << "Victim Cache miss" << endl;
            vc_misses++;
            //fetch the block from L2` cache
            
            //--------get block type,
            //-----if the block is an instruction block, prefetch the next block into ISB
            //-----if the block is a data block, prefetch the next block into DSB
            int type = ram.get_block_type(tag);
            if(type){
                return DSB.search(nib_addr, type);
            } 
            else {
                return ISB.search(nib_addr, type);
            }
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
        // cout << "Tag: " << tag << endl;
        int nextaddr_int = hexaddr_to_int(tag) + 1;
        string next_address_tag = int_to_hexaddr(nextaddr_int);

        if(nextaddr_int == 4096)
            next_address_tag = "000";
             
        //cout << "prefetched address prefetch: " << next_address << endl;

        return ram.get_block(next_address_tag);
    }

    void add_to_stream_buffer(Block block){
        //add the block to the stream buffer
        if(block.type == 0){
            ISB.add_block(block);
        } else {
            DSB.add_block(block);
        }
        
    }

    public:
        //constructor
        DirectMappedCache(int size = L1_MAX_BLOCKS){
            this->size = size;
            this->cache_blocks = vector<CacheBlock>(this->size);
        }
        
        //function to search the cache address, and the type of instruction (READ or WRITE)
        void search(string nib_addr, int instr_type=0){
            //convert address to binary
            string address = get_binary(nib_addr);
            int block_index = get_block(address);
            string tag = get_tag(address); 
            
            string tag_bin = get_tag(get_binary(cache_blocks[block_index].block.tag));
            if(cache_blocks[block_index].valid &&  tag_bin == tag){
                // cout << "L1 Cache hit" << endl;
                l1_hits++;
                if(instr_type && cache_blocks[block_index].block.type == 1){
                    cache_blocks[block_index].dirty = true;
                    // writes++;
                }
                return;
            } else {        
                /*
                
                Miss Order : 
                0. L1 Cache - if miss, go to step 1  -> PREFETCH parallelly
                    1. Victim Cache - if miss call ISB
                        2. ISB - if miss call DSB
                            3. DSB - if miss call L2 Cache
                                4. L2 Cache - if miss call RAM
                
                */
                // cout << "L1 Cache miss" << endl;
                l1_misses++;

                //-----------prefetch the next block
                Block next_block = prefetch_after_address(nib_addr);
                add_to_stream_buffer(next_block);

                //-----------search in the victim cache
                Block fetched_block = VC.search(nib_addr);

                //--------handle conflict miss
                if(cache_blocks[block_index].valid && tag_bin != tag){
                    //if the block is valid but the tag doesn't match, then it is a conflict miss
                    //put this block in the victim cache
                    VC.add_block(cache_blocks[block_index].block);
                    l1_evicts++;
                    
                    // if block is dirty, write to write buffer
                    if(cache_blocks[block_index].dirty){
                        WB.add_block(cache_blocks[block_index].block);
                    }
                }
                
                //-----------final step - update the cache.
                cache_blocks[block_index].set_block(fetched_block);
                if(instr_type && fetched_block.type == 1){
                    cache_blocks[block_index].dirty = true;
                    // writes++;
                }
            }
            
        }
};
DirectMappedCache L1;

void proc_load(string address){
    L1.search(address, 0);
}

void proc_store(string address){
    L1.search(address, 1);
}

//prints the statistics update pls 
void stats(){
    double l1_hit_rate = (double)l1_hits / (l1_hits + l1_misses) * 100;
    double l1_miss_rate = (double)l1_misses / (l1_hits + l1_misses) * 100;

    double l2_hit_rate = (double)l2_hits / (l2_hits + l2_misses) * 100;
    double l2_miss_rate = (double)l2_misses / (l2_hits + l2_misses) * 100;

    double vc_hit_rate = (double)vc_hits / (vc_hits + vc_misses) * 100;
    double vc_miss_rate = (double)vc_misses / (vc_hits + vc_misses) * 100;

    double isb_hit_rate = (double)isb_hits / (isb_hits + isb_misses) * 100;
    double isb_miss_rate = (double)isb_misses / (isb_hits + isb_misses) * 100;

    double dsb_hit_rate = (double)dsb_hits / (dsb_hits + dsb_misses) * 100;
    double dsb_miss_rate = (double)dsb_misses / (dsb_hits + dsb_misses) * 100;

    double global_hit_rate = (double)(l1_hits + l2_hits + vc_hits + isb_hits + dsb_hits) / (l1_hits + l2_hits + vc_hits + isb_hits + dsb_hits + l2_misses) * 100;

    cout << "Global Misses: " << l2_misses << endl;
    cout << "Global Hits: " << l1_hits + l2_hits + vc_hits + isb_hits + dsb_hits << endl;
    cout << "Global Hit Rate: " << global_hit_rate << "%" << endl;
    cout << "Global Miss Rate: " << (double)100 - global_hit_rate << endl << endl;


    cout << "L1 Cache" << endl;
    cout << "Hits: " << l1_hits << endl;
    cout << "Misses: " << l1_misses << endl;
    cout << "Hit Rate: " << l1_hit_rate << "%" << endl;
    cout << "Miss Rate: " << l1_miss_rate << "%" << endl;
    cout << "Evicts: " << l1_evicts << endl;
    cout << "Write Buffer Evicts: " << write_buffer_evicts << endl << endl;

    cout << "L2 Cache" << endl;
    cout << "Hits: " << l2_hits << endl;
    cout << "Misses: " << l2_misses << endl;
    cout << "Searches: " << l2_searches << endl;
    cout << "Hit Rate: " << l2_hit_rate << "%" << endl;
    cout << "Miss Rate: " << l2_miss_rate << "%" << endl << endl;

    cout << "Victim Cache" << endl;
    cout << "Hits: " << vc_hits << endl;
    cout << "Misses: " << vc_misses << endl;
    cout << "Hit Rate: " << vc_hit_rate << "%" << endl;
    cout << "Miss Rate: " << vc_miss_rate << "%" << endl << endl;

    cout << "Instruction Stream Buffer" << endl;
    cout << "Hits: " << isb_hits << endl;
    cout << "Misses: " << isb_misses << endl;
    cout << "Hit Rate: " << isb_hit_rate << "%" << endl;
    cout << "Miss Rate: " << isb_miss_rate << "%" << endl << endl;

    cout << "Data Stream Buffer" << endl;
    cout << "Hits: " << dsb_hits << endl;
    cout << "Misses: " << dsb_misses << endl;
    cout << "Hit Rate: " << dsb_hit_rate << "%" << endl;
    cout << "Miss Rate: " << dsb_miss_rate << "%" << endl << endl;

    cout << "Write Buffer" << endl;
    cout << "Flushes: " << flushes << endl;

    cout << "Reads: " << reads << endl;
    cout << "Writes: " << writes << endl;
}

// --tests--

/*
Spatial Locality Test

This test is to show that the cache is able to handle spatial locality
Number of misses should be low as the addresses are close to each other
Each miss should fetch a new block from RAM
*/
void spatial_test(int prog_size = 10000) {
    // Test for showing spatial locality
    int num_blocks = prog_size / (WORD_SIZE * BLOCK_SIZE);
    
    cout << "Spatial Locality Test" << endl;

    string base_tag = "";  
    base_tag += get_hex(rand() % 16);
    base_tag += get_hex(rand() % 16);

    for (int b = 0; b < num_blocks; b++) {
        for (int i = 0; i < 16; i++) {  // Search across multiple blocks
            string address = base_tag;
            address += get_hex(b);  // Increment block number in address
            address += get_hex(i);  // Word offset within block

            int instr_type = rand() % 2;  // Randomly assign load or store
            if(instr_type) {
                proc_store(address);
                writes++;
            } else {
                proc_load(address);
                reads++;
            }
            // L1.search(address);
        }
    }

    stats();
}


/*
Temporal Locality Test

This test loops through the same addresses multiple times,
it first gets 3 random blocks from RAM and then loops through each word in the block 5 times
Number of misses should be low as the addresses are close to each other, and repeatedly accessed
*/

void temporal_test(int num_loops=5){
    //generate 3 random tags
    string tags[5000];
    for(int i=0;i<5000;i++){
        tags[i] += get_hex(rand() % 16);
        tags[i] += get_hex(rand() % 16);
        tags[i] += get_hex(rand() % 16);
    }

    //generate 3 random addresses
    string addresses[5000];
    for(int i=0;i<5000;i++){
        addresses[i] = tags[i];
        addresses[i] += get_hex(rand() % 16);
    }

    //search for the 1000 addresses
    for(int i=0;i<5000;i++){
        L1.search(addresses[i]);
    }

    cout << "Temporal Localilty Test" << endl;

    //loop through each word in the tags and search for them 5 times
    for(int i=0;i<num_loops;i++){
        for(int j=0;j<5000;j++){
            for(int k=0;k<16;k++){
                string address = tags[j];
                address += get_hex(k);

                L1.search(address);
            }
        }
    }

    stats();
}

int main() {

    //use case example: word - 0F0FH
    //cache.search("0F0FH");

    // spatial_test();

    temporal_test();

    return 0;
}

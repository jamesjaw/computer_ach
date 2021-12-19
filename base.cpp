#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#define maxx 100

using namespace std;

struct cach{
    int NRU_bit = 1;
    int tag = 0;
};

int miss = 0;
int p_count = 0;

int address_bits = 0;
int block_size = 0;
int cache_sets = 0;
int associativity = 0;

string str[30];

int offset_bit_count = 0;
int indexing_bit_count = 0;
int indexing_bit[maxx];

int p_add[maxx];
bool hitornot[maxx];

int bit_to_size[11];


int NRU(int way,cach* recode){
    int pick = 0;
    bool find = false;
    
    while(find == false){
        for(int i=0;i<way;i++){
            if(recode[i].NRU_bit == 1){
                pick = i;
                recode[i].NRU_bit = 0;
                find = true;
                break;
            }
        }
        if(find == false){
            for(int i=0;i<way;i++) recode[i].NRU_bit = 1;
        }
    }
    return pick;
}

bool hit(int way,cach* set,int add){
    for(int i=0;i<way;i++){
        if(set[i].tag == add){
            return true;
        }
    }
    
    int pick = NRU(way, set);
    set[pick].tag = add;
    return false;
}


int main(int argc,char* argv[]){
    //init
    for(int i=0;i<maxx;i++){
        indexing_bit[i] = 0;
        p_add[i] = 0;
        hitornot[i] = false;
    }
    bit_to_size[0] = 0;
    bit_to_size[1] = 2;
    bit_to_size[2] = 4;
    bit_to_size[3] = 8;
    bit_to_size[4] = 16;
    bit_to_size[5] = 32;
    bit_to_size[6] = 64;
    bit_to_size[7] = 128;
    bit_to_size[8] = 256;
    bit_to_size[9] = 512;
    bit_to_size[10] = 1024;

    //input data
    fstream in,out;
    in.open(argv[1],ios::in);
    
    in>>str[0]>>address_bits>>str[1]>>block_size>>str[2]>>cache_sets>>str[3]>>associativity;
    in.flush();
    in.close();
    
    in.open(argv[2],ios::in);
    while(in>>p_add[p_count]){
        p_count++;
    }
    in.flush();
    in.close();
    
    //init
    for(int i=0;i<11;i++){
        if(block_size<=bit_to_size[i]){
            offset_bit_count = i;
            break;
        }
    }
    for(int i=0;i<11;i++){
        if(cache_sets<=bit_to_size[i]){
            indexing_bit_count = i;
            break;
        }
    }
    int temp = offset_bit_count;
    //baseline
    for(int i=0;i<indexing_bit_count;i++){
        indexing_bit[i] = temp++;
    }
    //creat cache
    cach** my_cach = new cach*[cache_sets];
    for(int i=0;i<cache_sets;i++){
        my_cach[i] = new cach[associativity];
    }
    //process
    for(int i=0;i<p_count;i++){
        int bit[address_bits+1];
        int add = p_add[i];
        int tag = 0;
        for(int j=0;j<address_bits;j++){
            bit[j] = add % 10;
            add = add / 10;
            if(j == offset_bit_count -1) tag = add;
        }
        
        int set = 0;
        int z = 1;
        for(int j=0;j<indexing_bit_count;j++){
            set += bit[indexing_bit[j]] * z;
            z *= 2;
        }
        
        if(hit(associativity, my_cach[set], tag)){
            hitornot[i] = true;
        }
        else{
            hitornot[i] = false;
            miss++;
        }
    }

    //output file
    out.open(argv[3],ios::out);
    out<<"Address bits: "<<str[0]<<"\n";
    out<<"Block size: "<<str[1]<<"\n";
    out<<"Cache sets: "<<str[2]<<"\n";
    out<<"Associativity: "<<str[3]<<"\n\n";
    //==================================================
    out<<"Offset bit count: "<<offset_bit_count<<"\n";
    out<<"Indexing bit count: "<<indexing_bit_count<<"\n";
    out<<"Indexing bits:";
    for(int i=indexing_bit_count-1;i>=0;i--) out<<" "<<indexing_bit[i];
    out<<"\n\n";
    //==================================================
    out<<".benchmark testcase1\n";
    for(int i=0;i<p_count;i++){
        out<<p_add<<" ";
        if(hitornot[i] == false)
            out<<"miss\n";
        else
            out<<"hit\n";
    }
    out<<".end\n\n";
    //=================================================
    out<<"Total cache miss count: "<<miss;
    out.close();
    
    //delete malloc
    for(int i=0;i<cache_sets;i++){
        delete [] my_cach[i];
    }
    delete [] my_cach;
    
    return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#define maxx 100

using namespace std;

vector<string> v_str;

struct cach{
    int NRU_bit = 1;
    string tag;
};

int miss = 0;
int p_count = 0;

int address_bits = 0;
int block_size = 0;
int cache_sets = 0;
int associativity = 0;


int offset_bit_count = 0;
int indexing_bit_count = 0;
int indexing_bit[maxx];

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

bool hit(int way,cach* set,string add){
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
    string s;
    in>>s>>s>>address_bits>>s>>s>>block_size>>s>>s>>cache_sets>>s>>associativity;
    in.flush();
    in.close();
    
    in.open(argv[2],ios::in);
    string s1;
    in>>s1;
    while(in>>s){
        v_str.push_back(s);
    }
    p_count = v_str.size() - 1;
    
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
    
    //baseline
    int temp = offset_bit_count;
    
    for(int i=0;i<indexing_bit_count;i++){
        indexing_bit[i] = temp++;
    }
    //creat cache
    cach** my_cach = new cach*[cache_sets];
    for(int i=0;i<cache_sets;i++){
        my_cach[i] = new cach[associativity];
    }
    //process
    for(int i=1;i<=p_count;i++){
        string tag;
        tag.assign(v_str[i],offset_bit_count,address_bits - offset_bit_count);
        
        int set = 0;
        int z = 1;
        for(int j=0;j<indexing_bit_count;j++){
            if(v_str[i][indexing_bit[j]] == '1'){
                set += z;
            }
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
    out<<"Address bits: "<<address_bits<<"\n";
    out<<"Block size: "<<block_size<<"\n";
    out<<"Cache sets: "<<cache_sets<<"\n";
    out<<"Associativity: "<<associativity<<"\n\n";
    //==================================================
    out<<"Offset bit count: "<<offset_bit_count<<"\n";
    out<<"Indexing bit count: "<<indexing_bit_count<<"\n";
    out<<"Indexing bits:";
    for(int i=indexing_bit_count-1;i>=0;i--) out<<" "<<indexing_bit[i];
    out<<"\n\n";
    //==================================================
    out<<s1<<"\n";
    for(int i=0;i<p_count;i++){
        out<<v_str[i]<<" ";
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

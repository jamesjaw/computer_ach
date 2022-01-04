#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

vector<string> v_str;

struct cach{
    int NRU_bit;
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
vector<int> indexing_bit;
vector<bool> hitornot;

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

int size2bit(int size){
    int bit = 0;
    if(size == 0) return 0;
    else if(size >=2){
        while(size !=1){
            size /= 2;
            bit++;
        }
    }
    return bit;
}
vector<vector<int>> index_bit_poss;
void pick_index_bit(double* Q_array,double** C_array,vector<int> index_bit, int indexing_bit_count){
    for(int i=0;i<indexing_bit_count;i++){
        double max = -1;
        int pick = 0;
        for(int j=0;j<address_bits - offset_bit_count;j++){
            if(Q_array[j] > max){
                max = Q_array[j];
                pick = j;
            }
        }
        Q_array[pick] = -2;
        for(int j=0;j<address_bits - offset_bit_count;j++){
            if(Q_array[j] == max){
                pick_index_bit(Q_array, C_array, index_bit, indexing_bit_count - i);
            }
        }
        index_bit.push_back(address_bits -1 - pick);
        
        for(int j=0;j<indexing_bit_count;j++){
            if(j!=pick){
                Q_array[j] *= C_array[j][pick];
            }
        }
    }
    index_bit_poss.push_back(index_bit);
}


int main(int argc,char* argv[]){
    //input data
    fstream in1,in2,out;
    
    in1.open(argv[1],ios::in);
    string s;
    in1>>s>>address_bits>>s>>block_size>>s>>cache_sets>>s>>associativity;
    in1.close();
    
    in2.open(argv[2],ios::in);
    string s1,s2;
    in2>>s1>>s2;
    while(in2>>s){
        v_str.push_back(s);
        p_count++;
    }
    p_count --;
    in2.close();
    
    //init
    offset_bit_count = size2bit(block_size);
    indexing_bit_count = size2bit(cache_sets);

    //bonus
    double** C_array = new double*[address_bits];
    for(int i=0;i<address_bits;i++){
        C_array[i] = new double[address_bits];
    }
    
    double* Q_array = new double[address_bits];
    
    for(int i=0;i<address_bits - offset_bit_count - 1;i++){
        for(int j=i+1;j<address_bits - offset_bit_count;j++){
            double E = 0;
            double D = 0;
            double C = 0;
            for(int k=0;k<p_count;k++){
                if(i != j){
                    if(v_str[k][i] == v_str[k][j]) E++;
                    else D++;
                }
            }
            if(E > D) C = D/E;
            else C = E/D;
            C_array[i][j] = C_array[j][i] = C;
        }
    }
    
    for(int i=0;i<address_bits - offset_bit_count;i++){
        double Z = 0;
        double O = 0;
        double Q = 0;
        for(int j=0;j<p_count;j++){
            if(v_str[j][i] == '0') Z++;
            else if(v_str[j][i] == '1') O++;
        }
        if(Z > O) Q = O/Z;
        else Q = Z/O;
        Q_array[i] = Q;
    }
    //pick index bit
    for(int i=0;i<indexing_bit_count;i++){
        double max = -1;
        int pick = 0;
        for(int j=0;j<address_bits - offset_bit_count;j++){
            if(Q_array[j] > max){
                max = Q_array[j];
                pick = j;
            }
        }
        indexing_bit.push_back(address_bits -1 - pick);
        Q_array[pick] = -2;
        for(int j=0;j<indexing_bit_count;j++){
            if(j!=pick){
                Q_array[j] *= C_array[j][pick];
            }
        }
    }
    
    //creat cache
    cach** my_cach = new cach*[cache_sets];
    for(int i=0;i<cache_sets;i++){
        my_cach[i] = new cach[associativity];
    }
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            my_cach[i][j].NRU_bit = 1;
        }
    }
    
    //process
    int lest_miss_count = 99999999;
    int best_index_bit = 0;
    for(int z=0;z<index_bit_poss.size();z++){
        //init cache
        for(int i=0;i<cache_sets;i++){
            for(int j=0;j<associativity;j++){
                my_cach[i][j].NRU_bit = 1;
            }
        }
        //run poss set
        int can_be_better = 0;
        int miss_poss = 0;
        for(int i=0;i<p_count;i++){
            string tag;
            //string index 01234567 not 76543210
            tag.assign(v_str[i],0,address_bits - offset_bit_count);
            
            int set = 0;
            int z = 1;
            for(int j=0;j<indexing_bit_count;j++){
                if(v_str[i][address_bits - 1 - index_bit_poss[z][j]] == '1'){
                    set += z;
                }
                z *= 2;
            }
            if(!hit(associativity, my_cach[set], tag)){
                miss_poss++;
                if(miss >= lest_miss_count){
                    can_be_better = 1;
                }
            }
            if(can_be_better == 1) break;
        }
        if(can_be_better == 0){
            lest_miss_count = miss_poss;
            best_index_bit = z;
        }
    }
    
    //sort index bit
    for(int i=0;i<indexing_bit_count;i++) indexing_bit[i] = index_bit_poss[best_index_bit][i];
    sort(indexing_bit.begin(), indexing_bit.begin() + indexing_bit_count);
    
    //init cache
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            my_cach[i][j].NRU_bit = 1;
        }
    }
    //run officily
    for(int i=0;i<p_count;i++){
        string tag;
        //string index 01234567 not 76543210
        tag.assign(v_str[i],0,address_bits - offset_bit_count);
        
        int set = 0;
        int z = 1;
        for(int j=0;j<indexing_bit_count;j++){
            if(v_str[i][address_bits - 1 - indexing_bit[j]] == '1'){
                set += z;
            }
            z *= 2;
        }
        if(hit(associativity, my_cach[set], tag)){
            hitornot.push_back(true);
        }
        else{
            hitornot.push_back(false);
            miss++;
        }
    }

    //output file
    out.open(argv[3],ios::out);
    out<<"Address bits: "<<address_bits<<"\n";
    out<<"Block size: "<<block_size<<"\n";
    out<<"Cache sets: "<<cache_sets<<"\n";
    out<<"Associativity: "<<associativity<<"\n\n";

    out<<"Offset bit count: "<<offset_bit_count<<"\n";
    out<<"Indexing bit count: "<<indexing_bit_count<<"\n";
    out<<"Indexing bits:";
    for(int i=indexing_bit_count-1;i>=0;i--) out<<" "<<indexing_bit[i];
    out<<"\n\n";

    out<<s1<<" "<<s2<<"\n";
    for(int i=0;i<p_count;i++){
        out<<v_str[i]<<" ";
        if(hitornot[i] == false)
            out<<"miss\n";
        else
            out<<"hit\n";
    }
    out<<".end\n\n";
    
    out<<"Total cache miss count: "<<miss<<"\n";
    out.close();
    
    //delete malloc
    /*
    for(int i=0;i<cache_sets;i++){
        delete [] my_cach[i];
    }
    delete [] my_cach;
    
    for(int i=0;i<p_count;i++){
        delete [] C_array[i];
    }
    delete [] C_array;
    delete [] Q_array;
    */
    return 0;
}

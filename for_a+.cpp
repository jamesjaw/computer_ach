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

double** C_array;
double* Q_array;
cach** my_cach;
vector<int>* set_coll[100];
int set_count = 0;

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

void find_poss_combin(int needed_bit, double* Q_chart, vector<int> bit_set ,int max_set){
    if(max_set == 0){
        return;
    }
    
    if(needed_bit == 0){
        set_coll[set_count] = new vector<int>;
        *set_coll[set_count] = bit_set;
        set_count++;
        
        max_set--;
        return;
    }
    
    for(int i=0;i<needed_bit;i++){
        double max = -1;
        int pick = 0;
        int count = 0;
        vector<int> same_value;
        //find max Q_value
        for(int j=0;j<address_bits - offset_bit_count;j++){
            if(Q_array[j] > max){
                max = Q_array[j];
                pick = j;
                same_value.clear();
                same_value.push_back(j);
                count = 1;
            }
            else if(Q_array[j] == max){
                same_value.push_back(j);
                count++;
            }
        }
        
        for(int j=0;j<count;j++){
            vector<int> new_set = bit_set;
            int pick = same_value[j];
            new_set.push_back(pick);
            double* new_Q_chart = new double[address_bits];
            for(int k=0;k<address_bits - offset_bit_count;k++){
                new_Q_chart[k] = Q_chart[k];
                if(k!=pick){
                    new_Q_chart[k] = new_Q_chart[k]*C_array[pick][k];
                }
            }
            find_poss_combin(needed_bit - 1, new_Q_chart, new_set, max_set);
        }
    }
}

int try_set_miss(int no_set, int mini_miss){
    //init chach
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            my_cach[i][j].NRU_bit = 1;
        }
    }
    int set_miss = 0;
    
    for(int i=0;i<p_count;i++){
        string tag;
        //string index 01234567 not 76543210
        tag.assign(v_str[i],0,address_bits - offset_bit_count);
        
        int set = 0;
        int z = 1;
        for(int j=0;j<indexing_bit_count;j++){
            if(v_str[i][address_bits - 1 - (*set_coll[no_set])[j]] == '1'){
                set += z;
            }
            z *= 2;
        }
        if(!hit(associativity, my_cach[set], tag)){
            set_miss++;
            if(set_miss > mini_miss){
                return -1;
            }
        }
    }
    
    return set_miss;
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

    //creat cache
    my_cach = new cach*[cache_sets];
    for(int i=0;i<cache_sets;i++){
        my_cach[i] = new cach[associativity];
    }
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            my_cach[i][j].NRU_bit = 1;
        }
    }
    //bonus
    //malloc place for C_chart and Q_chart
    C_array = new double*[address_bits];
    for(int i=0;i<address_bits;i++){
        C_array[i] = new double[address_bits];
    }
    
    Q_array = new double[address_bits];
    
    //bulid C_chart
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
    
    //build Q_chart
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
    
    //pick best bit for index
    vector<int> first_set;
    int pick_set = 0;
    find_poss_combin(indexing_bit_count, Q_array, first_set ,50);
    if(set_count == 0){
        cout<<"something wrong\n";
    }
    else if(set_count == 1){
        indexing_bit = *set_coll[0];
    }
    //try which is best index bit set
    else{
        int mini_miss = 21470000;
        for(int i=0;i<set_count;i++){
            int result = try_set_miss(i, mini_miss);
            if(result != -1){
                mini_miss = result;
                pick_set = i;
            }
        }
    }
    indexing_bit = *set_coll[pick_set];
    //sort index bit
    sort(indexing_bit.begin(), indexing_bit.begin() + indexing_bit_count);
    
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            my_cach[i][j].NRU_bit = 1;
        }
    }
    
    //process
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
    /*
    //delete malloc
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

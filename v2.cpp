#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;
//recode input address
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

double** C_array;
//for try main set===============================================
vector<int> coll_set[5000005];
int set_num = 0;

void try_many_set(int index_bit_count ,double* chart, vector<int> set, int* picked){
    if(set_num == 5000000) return;
    if(index_bit_count == 0){
        coll_set[set_num++] = set;
        return;
    }

    double max = -1;
    int same_value_bit[35];
    int same_value_count = 0;
    vector<double> wanna_sort;
    for(int i=0;i<address_bits - offset_bit_count;i++) wanna_sort[i] = chart[i];
    sort(wanna_sort.begin(), wanna_sort.begin() + address_bits - offset_bit_count);
    int min = wanna_sort[address_bits - offset_bit_count - 6];
    for(int j=0;j<address_bits - offset_bit_count;j++){
        if(picked[j] == 0){
            if(chart[j] >= min){
                //max = chart[j];
                //same_value_count = 0;
                same_value_bit[same_value_count++] = j;
            }
            //else if(chart[j] == max){
                //same_value_bit[same_value_count++] = j;
            //}
        }
    }
    
    //cout<<index_bit_count<<"same:"<<same_value_count<<" \n";
    //for(int i=0;i<same_value_count;i++) cout<<" "<<same_value_bit[i];
   // cout<<"\n";
    
    for(int i=0;i<same_value_count;i++){
        int pick = same_value_bit[i];
        double temp_chart[35];
        for(int j=0;j<address_bits - offset_bit_count;j++) temp_chart[j] = chart[j];
        //cout<<"round:"<<i<<"chart:\n";
        //for(int j=0;j<address_bits - offset_bit_count;j++) cout<<chart[j]<<" ";
        //cout<<"\n";
        vector<int> my_set = set;
        
        my_set.push_back(address_bits -1 - pick);
        chart[pick] = -2;
        for(int j=0;j<address_bits - offset_bit_count;j++){
            if(j!=pick){
                chart[j] = chart[j]*C_array[pick][j];
            }
        }
        picked[pick] = 1;
        try_many_set(index_bit_count -1,chart, my_set, picked);
        picked[pick] = 0;
        for(int j=0;j<address_bits - offset_bit_count;j++){
            chart[j] = temp_chart[j];
        }
    }
}
int mini_miss = 214700000;
bool miss_right(int no, cach** cache){
    int temp_miss = 0;
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            cache[i][j].NRU_bit = 1;
            cache[i][j].tag = "-1";
        }
    }
    for(int i=0;i<p_count;i++){
        string tag;
        //string index 01234567 not 76543210 so pick 0~x for tag bit
        tag.assign(v_str[i], 0, address_bits - offset_bit_count);
        
        int set = 0;
        int z = 1;
        for(int j=0;j<indexing_bit_count;j++){
            if(v_str[i][address_bits - 1 - coll_set[no][j]] == '1'){
                set += z;
            }
            z *= 2;
        }
        if(!hit(associativity, cache[set], tag)){
            temp_miss++;
            if(temp_miss > mini_miss) return false;
        }
    }
    mini_miss = temp_miss;
    return true;
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
    
    //creat cache for simulation
    cach** my_cach = new cach*[cache_sets];
    for(int i=0;i<cache_sets;i++){
        my_cach[i] = new cach[associativity];
    }

    //bonus=========================================================
    C_array = new double*[address_bits];
    for(int i=0;i<address_bits;i++){
        C_array[i] = new double[address_bits];
    }
    double* Q_array = new double[address_bits];
    //build C_chart
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
    //cout<<"this is C_chart\n";
    //for(int i=0;i<address_bits - offset_bit_count;i++){
        //for(int j=0;j<address_bits - offset_bit_count;j++){
            //if(i==j) cout<<"x ";
            //else cout<<C_array[i][j]<<" ";
        //}
        //cout<<"\n";
    //}
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
    //find one set==============================================================
    /*
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
        for(int j=0;j<address_bits - offset_bit_count;j++){
            if(j!=pick){
                Q_array[j] = Q_array[j]*C_array[pick][j];
            }
        }
    }
    */
    //try many set================================================================
    int picked[35];
    int set_no = 0;
    for(int i=0;i<35;i++) picked[i] = 0;
    vector<int> set1;
    //cout<<"Q_array:\n";
    //for(int i=0;i<address_bits - offset_bit_count;i++) cout<<Q_array[i]<<" ";
    //cout<<"\n";
    try_many_set(indexing_bit_count, Q_array, set1, picked);
    for(int i=0;i<set_num;i++){
        //cout<<"set "<<i<<":";
        //for(int j=0;j<indexing_bit_count;j++){
            //cout<<coll_set[i][j]<<" ";
        //}
        //cout<<"\n";
        if(miss_right(i, my_cach)){
            set_no = i;
        }
    }
    cout<<"pick set is :"<<set_no<<"\n";
    for(int i=0;i<indexing_bit_count;i++) cout<<" "<<coll_set[set_no][i];
    cout<<"\n";
    cout<<"mini miss is : "<<mini_miss<<"\n";
    
    //============================================================================
    /*
    //baseline LSB ===============================================================
    int temp = offset_bit_count;
    for(int i=0;i<indexing_bit_count;i++){
        indexing_bit.push_back(temp);
        temp++;
    }
    //============================================================================
    */
    //sort indexing bit
    //sort(indexing_bit.begin(), indexing_bit.begin() + indexing_bit_count);
    
    for(int i=0;i<cache_sets;i++){
        for(int j=0;j<associativity;j++){
            my_cach[i][j].NRU_bit = 1;
            my_cach[i][j].tag = "-1";
        }
    }
    /*
    //process
    for(int i=0;i<p_count;i++){
        string tag;
        //string index 01234567 not 76543210 so pick 0~x for tag bit
        tag.assign(v_str[i], 0, address_bits - offset_bit_count);
        
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
    */
    
    /* somehow it make error Q_Q
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

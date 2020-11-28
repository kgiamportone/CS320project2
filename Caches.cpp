#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include <math.h>

using namespace std;

unsigned long long binary_decimal_convert(unsigned long long binaryArray[], int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += binaryArray[i] * pow(2, size - i - 1);
    }
    return sum;
}

vector<int> to_binary(unsigned long index) {
    vector<int> retVal(9);
    int i = 0;
    while (index > 0) {
        retVal[i] = (index % 2);
        index = index / 2;
        i++;
    }
    reverse(retVal.begin(), retVal.end());
    return retVal;
}

void direct_mapped(char * input_file, FILE * output_file, unsigned long long size) {
    // size = 1,4,16, or 32
    unsigned long long cache_size = size * 1024 / 32;

    unsigned long long index_bits = log2(cache_size);
    unsigned long long tag_bits = 32 - index_bits;
    printf("Index bits: %llu, tag bits: %llu\n", index_bits, tag_bits);

    unsigned long long cache[cache_size];

    for (unsigned long long i = 0; i < cache_size; i++) {
        cache[i] = 0;
    }

    FILE * input = fopen(input_file, "r");

    char load_store[5];
    unsigned long long address;

    int hits = 0;
    int total = 0;

    while (fscanf(input, "%5s %llx\n", load_store, &address) != EOF) {
        unsigned long long index = (address / 32) % cache_size; // get last (size) bits
        unsigned long long tag = address >> (index_bits + 5);
        // printf("Tag %llu\n", tag);
        // printf("Address %llu\n", address);


        if (tag == cache[index]) {
            hits++;
        }
        else {
            cache[index] = tag;
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", hits, total);
}

void set_associative(char * input_file, FILE * output_file, unsigned long long ways) {
    // size = 1,4,16, or 32
    unsigned long long cache_size = 512;
    unsigned long long num_sets = cache_size / ways;
    unsigned long long index_bits = log2(num_sets);
    printf("Set associative with %llu ways, %llu sets \n", ways, num_sets);
    // int lru to keep track of index to update
    /*
    True LRU:
    use counter for every instruction
    individual counters for each page table entry
    update indiv counter to total counter each time used
    LRU = entry with lowest indiv counter
    */

    // unsigned long long index_bits = log2(cache_size);
    // unsigned long long tag_bits = 32 - index_bits;
    // printf("Index bits: %llu, tag bits: %llu\n", index_bits, tag_bits);

    // create vector of int vectors for LRU ways
    vector<vector<int>> lru(num_sets, vector<int>(ways));

    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            lru[i][j] = j;
        }
    }

    unsigned long long cache[num_sets][ways];
    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            cache[i][j] = 0;
        }
    }

    FILE * input = fopen(input_file, "r");

    char load_store[5];
    unsigned long long address;

    int hits = 0;
    int total = 0;

    while (fscanf(input, "%5s %llx\n", load_store, &address) != EOF) {
        unsigned long long index = (address / 32) % num_sets;
        unsigned long long tag = address >> (index_bits + 5);
        int flag = 0;

        for (unsigned long long i = 0; i < ways; i++) {
            if (tag == cache[index][i]) { // hit
                hits++;
                flag = 1;
                // update LRU!!!
                for (int j = 0; j < ways; j++) {
                    if (lru[index][j] == i) { // found way
                        lru[index].erase(lru[index].begin() + j);
                        lru[index].push_back(i);
                    }
                }
                break;
            }
        }
        if (flag == 0) { // miss, update cache line and LRU
            int to_update = lru[index][0];
            cache[index][to_update] = tag;
            lru[index].erase(lru[index].begin());
            lru[index].push_back(to_update);
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", hits, total);
}

void fully_associative_hotcold(char * input_file, FILE * output_file) {
    // size = 1,4,16, or 32
    int hot_cold[512]; // starts with element 1 NOT ELEMENT 0
    for (int i = 0; i < 512; i++) {
        hot_cold[i] = 0;
    }
    // HOT COLD array
    // start at index 1, go up to index 511 (511 bits)
    // for hot_cold[n]:
        // left child = hot_cold[2n] (2n <= 511)
        // right child = hot_cold[2n + 1] (2n+1 <= 511)
    // PSEUDO
    /*
    int hc_index = 1;
    int n = 2;
    int cache_index = 0;
    while (hc_index < 512) {
        if (hot_cold[hc_index] == 0) { // left is LRU
            // index stays the same
            hot_cold[hc_index] = 1; // flip bit
            hc_index = hc_index * 2;
        }
        else { // right is LRU
            cache_index += (512 / n);
            hot_cold[hc_index] = 0; // flip bit
            hc_index = (hc_index * 2) + 1;
        }
        n *= 2;
    }*/

    unsigned long long cache[512];
    for (unsigned long long i = 0; i < 512; i++) {
        cache[i] = 0;
    }

    FILE * input = fopen(input_file, "r");

    char load_store[5];
    unsigned long long address;

    int hits = 0;
    int total = 0;

    while (fscanf(input, "%5s %llx\n", load_store, &address) != EOF) {
        unsigned long long tag = address >> (5);
        int flag = 0;

        for (unsigned long long i = 0; i < 512; i++) { // TODO
            if (tag == cache[i]) { // hit
                hits++;
                flag = 1; // mark a hit
                // update LRU!!!
                vector<int> binary = to_binary(i);
                int hc_index = 1;
                for (int i = 0; i < 9; i++) {
                    if (binary[i] == 0) {
                        hot_cold[hc_index] = 1;
                        hc_index = hc_index * 2;
                    }
                    else {
                        hot_cold[hc_index] = 0;
                        hc_index = (hc_index * 2) + 1;
                    }
                }
                break;
            }
        }
        if (flag == 0) { // miss, update cache line and LRU
            int hc_index = 1;
            int n = 2;
            int cache_index = 0;
            while (hc_index < 512) {
                if (hot_cold[hc_index] == 0) { // left is LRU
                    // index stays the same
                    hot_cold[hc_index] = 1; // flip bit
                    hc_index = hc_index * 2;
                }
                else { // right is LRU
                    cache_index += (512 / n);
                    hot_cold[hc_index] = 0; // flip bit
                    hc_index = (hc_index * 2) + 1;
                }
                n *= 2;
            }
            cache[cache_index] = tag;
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", hits, total);
}

void set_associative_no_wm(char * input_file, FILE * output_file, unsigned long long ways) {
    // size = 1,4,16, or 32
    unsigned long long cache_size = 512;
    unsigned long long num_sets = cache_size / ways;
    unsigned long long index_bits = log2(num_sets);
    printf("Set associative with %llu ways, %llu sets \n", ways, num_sets);

    vector<vector<int>> lru(num_sets, vector<int>(ways));

    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            lru[i][j] = j;
        }
    }

    unsigned long long cache[num_sets][ways];
    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            cache[i][j] = 0;
        }
    }

    FILE * input = fopen(input_file, "r");

    char load_store[5];
    unsigned long long address;

    int hits = 0;
    int total = 0;

    while (fscanf(input, "%5s %llx\n", load_store, &address) != EOF) {
        unsigned long long index = (address / 32) % num_sets;
        unsigned long long tag = address >> (index_bits + 5);
        int flag = 0;

        for (unsigned long long i = 0; i < ways; i++) {
            if (tag == cache[index][i]) { // hit
                hits++;
                flag = 1;
                // update LRU!!!
                for (int j = 0; j < ways; j++) {
                    if (lru[index][j] == i) { // found way
                        lru[index].erase(lru[index].begin() + j);
                        lru[index].push_back(i);
                    }
                }
                break;
            }
        }
        if (flag == 0) { // miss, update cache line and LRU
            if (strncmp(load_store, "L", 1) == 0) {
                int to_update = lru[index][0];
                cache[index][to_update] = tag;
                lru[index].erase(lru[index].begin());
                lru[index].push_back(to_update);
            }
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", hits, total);
}

void prefetch(char * input_file, FILE * output_file, unsigned long long ways) {
    // size = 1,4,16, or 32
    unsigned long long cache_size = 512;
    unsigned long long num_sets = cache_size / ways;
    unsigned long long index_bits = log2(num_sets);
    printf("Set associative with %llu ways, %llu sets \n", ways, num_sets);

    vector<vector<int>> lru(num_sets, vector<int>(ways));

    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            lru[i][j] = j;
        }
    }

    unsigned long long cache[num_sets][ways];
    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            cache[i][j] = 0;
        }
    }

    FILE * input = fopen(input_file, "r");

    char load_store[5];
    unsigned long long address;

    int hits = 0;
    int total = 0;

    while (fscanf(input, "%5s %llx\n", load_store, &address) != EOF) {
        unsigned long long index = (address / 32) % num_sets;
        unsigned long long tag = address >> (index_bits + 5);
        int flag = 0;

        for (unsigned long long i = 0; i < ways; i++) {
            if (tag == cache[index][i]) { // hit
                hits++;
                flag = 1;
                // update LRU!!!
                for (int j = 0; j < ways; j++) {
                    if (lru[index][j] == i) { // found way
                        lru[index].erase(lru[index].begin() + j);
                        lru[index].push_back(i);
                    }
                }
                break;
            }
        }
        if (flag == 0) { // miss, update cache line and LRU
            int to_update = lru[index][0];
            cache[index][to_update] = tag;
            lru[index].erase(lru[index].begin());
            lru[index].push_back(to_update);
        }
        // prefetch
        address = address + 32;
        index = (address / 32) % num_sets;
        tag = address >> (index_bits + 5);

        flag = 0;
        for (unsigned long long i = 0; i < ways; i++) {
            if (tag == cache[index][i]) { // hit
                // hits++;
                flag = 1;
                // update LRU!!!
                for (int j = 0; j < ways; j++) {
                    if (lru[index][j] == i) { // found way
                        lru[index].erase(lru[index].begin() + j);
                        lru[index].push_back(i);
                    }
                }
                break;
            }
        }
        if (flag == 0) { // miss, update cache line and LRU
            int to_update = lru[index][0];
            cache[index][to_update] = tag;
            lru[index].erase(lru[index].begin());
            lru[index].push_back(to_update);
        }

        total++;
    }
    fprintf(output_file, "%d,%d; ", hits, total);
}

void prefetch_miss(char * input_file, FILE * output_file, unsigned long long ways) {
    // size = 1,4,16, or 32
    unsigned long long cache_size = 512;
    unsigned long long num_sets = cache_size / ways;
    unsigned long long index_bits = log2(num_sets);
    printf("Set associative with %llu ways, %llu sets \n", ways, num_sets);

    vector<vector<int>> lru(num_sets, vector<int>(ways));

    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            lru[i][j] = j;
        }
    }

    unsigned long long cache[num_sets][ways];
    for (unsigned long long i = 0; i < num_sets; i++) {
        for (unsigned long long j = 0; j < ways; j++) {
            cache[i][j] = 0;
        }
    }

    FILE * input = fopen(input_file, "r");

    char load_store[5];
    unsigned long long address;

    int hits = 0;
    int total = 0;

    while (fscanf(input, "%5s %llx\n", load_store, &address) != EOF) {
        unsigned long long index = (address / 32) % num_sets;
        unsigned long long tag = address >> (index_bits + 5);
        int flag = 0;

        for (unsigned long long i = 0; i < ways; i++) {
            if (tag == cache[index][i]) { // hit
                hits++;
                flag = 1;
                // update LRU!!!
                for (int j = 0; j < ways; j++) {
                    if (lru[index][j] == i) { // found way
                        lru[index].erase(lru[index].begin() + j);
                        lru[index].push_back(i);
                    }
                }
                break;
            }
        }
        if (flag == 0) { // miss, update cache line and LRU
            int to_update = lru[index][0];
            cache[index][to_update] = tag;
            lru[index].erase(lru[index].begin());
            lru[index].push_back(to_update);
            // prefetch
            address = address + 32;
            index = (address / 32) % num_sets;
            tag = address >> (index_bits + 5);

            flag = 0;
            for (unsigned long long i = 0; i < ways; i++) {
                if (tag == cache[index][i]) { // hit
                    // hits++;
                    flag = 1;
                    // update LRU!!!
                    for (int j = 0; j < ways; j++) {
                        if (lru[index][j] == i) { // found way
                            lru[index].erase(lru[index].begin() + j);
                            lru[index].push_back(i);
                        }
                    }
                    break;
                }
            }
            if (flag == 0) { // miss, update cache line and LRU
                int to_update = lru[index][0];
                cache[index][to_update] = tag;
                lru[index].erase(lru[index].begin());
                lru[index].push_back(to_update);
            }
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", hits, total);
}

int main(int argc, char *argv[]) {
    // argv[1] = input file name
    char * input_name = argv[1];
    // argv[2] = output file name
    char * output_name = argv[2];

    // open output FILE
    FILE *output;
    output = fopen(output_name, "w");

    direct_mapped(input_name, output, 1);
    direct_mapped(input_name, output, 4);
    direct_mapped(input_name, output, 16);
    direct_mapped(input_name, output, 32);
    fprintf(output, "\n");

    set_associative(input_name, output, 2);
    set_associative(input_name, output, 4);
    set_associative(input_name, output, 8);
    set_associative(input_name, output, 16);
    fprintf(output, "\n");

    // fully associative tru LRU
    set_associative(input_name, output, 512);
    fprintf(output, "\n");
    // full associative hot cold
    fully_associative_hotcold(input_name, output);
    fprintf(output, "\n");

    set_associative_no_wm(input_name, output, 2);
    set_associative_no_wm(input_name, output, 4);
    set_associative_no_wm(input_name, output, 8);
    set_associative_no_wm(input_name, output, 16);
    fprintf(output, "\n");

    prefetch(input_name, output, 2);
    prefetch(input_name, output, 4);
    prefetch(input_name, output, 8);
    prefetch(input_name, output, 16);
    fprintf(output, "\n");

    prefetch_miss(input_name, output, 2);
    prefetch_miss(input_name, output, 4);
    prefetch_miss(input_name, output, 8);
    prefetch_miss(input_name, output, 16);
    fprintf(output, "\n");

    fclose(output);
    return 0;
}

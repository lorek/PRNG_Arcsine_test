#include "DyckPathGenerator.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>

using namespace std;

/***********************************************************************************
 * Generator of Dyck Paths based on vector of booleans                             *
 *    - an optimized version of the vector container for storing binary sequences  *
 *      (1 bit per each bool)                                                      *
 ***********************************************************************************/
 
/***********************************************************************************
 *  PUBLIC METHODS                                                                 *
 ***********************************************************************************/ 
DyckPathGenerator::DyckPathGenerator() {}
	
DyckPathGenerator::DyckPathGenerator(int64 seed) {
	set_seed(seed);
}
		
void DyckPathGenerator::set_seed(int64 seed) {
	mtGen.seed(seed);
}
		
vector<bool>* DyckPathGenerator::generate_bitsequence(int64 path_n) {
	init_bitseq();
	shuffle_bitseq();
	swap_subpath(first_lowest_lvl());
	bitseq.pop_back();
	
	return &bitseq; 			
}
		
vector<bool>* DyckPathGenerator::generate_bitsequence(int64 path_n, int64 seed) {
	set_seed(seed);
	return generate_bitsequence(path_n);
}

/***********************************************************************************
 *  PRIVATE METHODS                                                                *
 ***********************************************************************************/
void DyckPathGenerator::init_bitseq() {
	bitseq.assign(2*n+1, false); // resize (also shrink) if needed and fill with 0's
	fill(bitseq.begin(), bitseq.begin() + n, true); // set first n bits to 1	
}

void DyckPathGenerator::shuffle_bitseq() {
	shuffle(bitseq.begin(), bitseq.end(), mtGen);
}
			
/* 
 * returns the index idx of the bit s.t. after reading it the path reaches its lowest
 * level for the first time 
 * -1 means that the path is empty; otherwise minimum is reached after reading >= 1 bit
 *  the path starts at level 0 and ends at level -1
 * P1 = b_{0}...b_{idx} and is empty iff the path is empty
 * P2 = b_{idx+1}...b_{2*n} and may be empty 
 */
int64 DyckPathGenerator::first_lowest_lvl() {
	int64 idx = -1;  
	int64 minIdx = -1; // -1 returned iff the path is empty 
	int64 lvl = 0;
	int64 minLvl = 0;
	for(auto b : bitseq) {
		++idx;
		if(b) {  
			++lvl;
		}
		else {
			--lvl;
			if(lvl < minLvl) {
				minLvl = lvl;
				minIdx = idx; 						
			}
		}
	}
	return minIdx;
}

/*
 * Construct a Dyck Path from a given path by splitting it into P1|P2 at the point 
 * at which the first minimum is reached, constructing the path P2|P1 and discarding
 * the last bit.
 *
 * Implementation based on built-in function rotate 
 */
void DyckPathGenerator::swap_subpath(int64 idx) { // P1 -> b_0...b_idx; P2 -> b_{idx+1}...b_2n
	rotate(bitseq.begin(), bitseq.begin() + idx + 1, bitseq.end());
}
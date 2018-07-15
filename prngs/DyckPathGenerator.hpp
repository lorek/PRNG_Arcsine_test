#ifndef _DYCK_PATH_H_
#define _DYCK_PATH_H_

#include <cstdlib>
#include <bitset>
#include <random>

typedef long long int64;

/***********************************************************************************
 * Generator of Dyck Paths based on vector of booleans                             *
 *    - an optimized version of the vector container for storing binary sequences  *
 *      (1 bit per each bool)                                                      *
 ***********************************************************************************/
class DyckPathGenerator {
	public:
		DyckPathGenerator();
	
		DyckPathGenerator(int64 seed);
		
		void set_seed(int64 seed);
		
		std::vector<bool>* generate_bitsequence(int64 path_n);
		
		std::vector<bool>* generate_bitsequence(int64 path_n, int64 seed);

		
	private:
		int64 n;
		std::vector<bool> bitseq; // Sequence of generated bits
		std::mt19937_64 mtGen;    // the underlying PRNG - Mersenne Twister MT19937 (64-bit)
		
		void init_bitseq();
		
		void shuffle_bitseq();
		
		int64 first_lowest_lvl();
		
		void swap_subpath(int64 idx);

};

#endif
#ifndef _FLAWED_PATH_H_
#define _FLAWED_PATH_H_

#include <cstdlib>
#include <bitset>
#include <random>

typedef long long int64;

/*********************************************************************************** 
 * Generator of "flawed" bitsequences based on Dyck Path.                          *
 *                                                                                 *
 * These sequences can be viewed as trajectories of random walks which spends half *
 * of the time above and half of the time below the level 0 (OX axis). [P1]        *
 *                                                                                 *
 * The idea of algorithm is the following:                                         *
 *   - generate a random permutation of 2n bits - n 0's ans n 1's                  *
 *   - split the sequence into disjoint subpaths by joining all consecutive 0s     *
 *     (resp. 1s) into a single subpath                                            *
 *   - for each such subpath (say of length k) generate uniformly at random        *
 *     a Dyck Path of length 2k; Dyck Paths corresponding to subsequences of 0s    *
 *     are reflected over OX                                                       *
 *   - as a result we get a random sequence of bits of length 4n satisfying [P1]   *
 *                                                                                 * 
 * The implementation is based on vector of booleans                               *
 *    - an optimized version of the vector container                               *
 *      for storing binary sequences (1 bit per each bool)                         *
 ***********************************************************************************/
class FlawedPath {
	public:
		FlawedPath();
	
		FlawedPath(int64 seed);
		
		void setSeed(int64 seed);
		
		std::vector<bool>* generateBitSequence(int64 path_n);
		
		std::vector<bool>* generateBitSequence(int64 path_n, int64 seed);
		
		std::vector<bool>* generateBitSequence2(int64 path_n);
		
		std::vector<bool>* generateBitSequence2(int64 path_n, int64 seed);
		
		std::vector<bool>::iterator itPathBegin();
		
		std::vector<bool>::iterator itPathEnd();
		
		void prettyPrintPath();
        
        void printBitsAscii();
        
		
	private:
		int64 n;
		std::vector<bool> flawed_bitseq; // Sequence of generated bits of length 4n
		std::vector<bool> ctrl_bitseq; // Control sequence - random permutation of n 0s and n 1s
		std::mt19937_64 mt_eng; // the underlying PRNG - Mersenne Twister MT19937 (64-bit)
		
		void initSequences();
		
		void initSequences2();
		
		void initCtrlSeq();
		
		void initCtrlSeq2();
		
		void shuffleBitseq(std::vector<bool>::iterator begin, std::vector<bool>::iterator end);
		
		void generateDyckPath(std::vector<bool>::iterator begin, std::vector<bool>::iterator end, bool b);
		
		std::vector<bool>::iterator firstLowestLevelIterator(std::vector<bool>::iterator begin, 
														std::vector<bool>::iterator end);
													
		std::pair<std::vector<bool>::iterator, int64>
			firstLowestLevel(std::vector<bool>::iterator begin, std::vector<bool>::iterator end);
		
		void findSubpathLenghts(std::vector<int64>& dyck_lenghts);
		
		void swapSubpaths(std::vector<bool>::iterator begin, std::vector<bool>::iterator end,
							std::vector<bool>::iterator idx);

};

#endif

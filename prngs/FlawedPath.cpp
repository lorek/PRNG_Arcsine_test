#include "FlawedPath.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>

using namespace std;

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
 
 /***********************************************************************************
 *  PUBLIC METHODS                                                                 *
 ***********************************************************************************/ 
FlawedPath::FlawedPath() {}
	
FlawedPath::FlawedPath(int64 seed) {
	setSeed(seed);
}
		
void FlawedPath::setSeed(int64 seed) {
	mt_eng.seed(seed);
}
		
vector<bool>* FlawedPath::generateBitSequence(int64 path_n) {
	n = path_n;
	initSequences();
	// divide the control sequence into subsequences of equal bits
	// and for each such subsequence generate a dyck path
	vector<bool>::iterator it_begin_dyck = flawed_bitseq.begin();
	vector<bool>::iterator it = ctrl_bitseq.begin();
	while(it != ctrl_bitseq.end()) {
		// find next subsequence of consecutive 0s or 1s
		vector<bool>::iterator it_begin_sub_ctrl = it; // inclusive
		bool b = *it_begin_sub_ctrl;
		while(++it != ctrl_bitseq.end() && *it == b) { /* Intentionally empty */ }
		// get the length of this sequence
		auto sub_ctrl_len = distance(it_begin_sub_ctrl, it);
		// generate next Dyck Paths
		generateDyckPath(it_begin_dyck, it_begin_dyck + 2*sub_ctrl_len + 1, b);
		it_begin_dyck += 2*sub_ctrl_len;
	}
	
	flawed_bitseq.pop_back(); // discard last bit
	return &flawed_bitseq; 			
}

vector<bool>* FlawedPath::generateBitSequence2(int64 path_n) {
	n = path_n;
	initSequences2();
	// copy the control sequence into first 2n bits of generated path
	copy(ctrl_bitseq.begin(), ctrl_bitseq.end(), flawed_bitseq.begin());
	// find lengths of Dyck Paths that will form the 2nd part of flawed path
	vector<int64> dyck_lenghts;
	findSubpathLenghts(dyck_lenghts);
	// shuffle lengths - random ordering of Dyck paths
	//shuffle(dyck_lenghts->begin(), dyck_lenghts->end(), mt_eng);
	// generate corresponding paths
	vector<bool>::iterator it_begin_dyck = flawed_bitseq.begin() + 2*n;
	for(auto len : dyck_lenghts) {
		//cout << "Found subpath of length " << len << endl;
		bool above = len > 0;
		int64 len_pos = above ? len : -len;
		generateDyckPath(it_begin_dyck, it_begin_dyck + len_pos + 1, !above);
		it_begin_dyck += len_pos;
	} 
	
	flawed_bitseq.pop_back(); // discard last bit
	return &flawed_bitseq; 			
}
		
vector<bool>* FlawedPath::generateBitSequence(int64 path_n, int64 seed) {
	setSeed(seed);
	return generateBitSequence(path_n);
}

vector<bool>* FlawedPath::generateBitSequence2(int64 path_n, int64 seed) {
	setSeed(seed);
	return generateBitSequence2(path_n);
}

vector<bool>::iterator FlawedPath::itPathBegin() {
	return flawed_bitseq.begin();
}

vector<bool>::iterator FlawedPath::itPathEnd() {
	return flawed_bitseq.end();
}


void FlawedPath::printBitsAscii() {
	for(auto b : flawed_bitseq) {
		cout << b;
	}
}

void FlawedPath::prettyPrintPath() {
	cout << string(80, '-') << endl;
// 	cout << "Control path of length " << 2*n << endl;
// 	for(auto b : ctrl_bitseq) {
// 		cout << b;
// 	}
	cout << endl << string(40, '-') << endl;
	cout << "Path of length " << 4*n << endl;
	for(auto it = flawed_bitseq.begin(); it != flawed_bitseq.end(); ++it) {
		cout << *it;
		if(it + 1 !=  flawed_bitseq.end()) { 
			cout << ",";
		}
	}
	for(auto b : flawed_bitseq) {
		cout << b << ",";
	}
	cout << endl;
	cout << endl << string(40, '-') << endl;
	int64 min_lvl = - firstLowestLevel(flawed_bitseq.begin(), flawed_bitseq.end()).second;
	int64 lvl = 0;
	for(auto b : flawed_bitseq) {
		cout << b << ' ';
		if(b) {
			if(lvl < 0) {
				cout << string(min_lvl + lvl, ' ') << '\\' << string(-(lvl + 1), ' ') << '|' << endl;
			}
			else {
				cout << string(min_lvl, ' ') << '|' << string(lvl, ' ') << '\\' << endl;
			}
			++lvl;
		}
		else {
			--lvl;
			if(lvl < 0) {
				cout << string(min_lvl + lvl, ' ') << '/' << string(-(lvl + 1), ' ') << '|' << endl;
			}
			else {
				cout << string(min_lvl, ' ') << '|' << string(lvl, ' ')  << '/' << endl;
			}
		}
	}
	cout << string(80, '-') << endl;
}

/***********************************************************************************
 *  PRIVATE METHODS                                                                *
 ***********************************************************************************/		
void FlawedPath::initSequences() {
	// initialize output sequence
	flawed_bitseq.assign(4*n+1, false); // resize container for generated sequence (path) if needed
	// initialize control sequence
	initCtrlSeq();
}

void FlawedPath::initSequences2() {
	// initialize output sequence
	flawed_bitseq.assign(4*n+1, false); // resize container for generated sequence (path) if needed
	// initialize control sequence
	initCtrlSeq2();
}

void FlawedPath::initCtrlSeq() {
	ctrl_bitseq.assign(2*n, false); // resize container for control sequence if necessary
	fill(ctrl_bitseq.begin(), ctrl_bitseq.begin() + n, true);
	shuffleBitseq(ctrl_bitseq.begin(), ctrl_bitseq.end());
}

void FlawedPath::initCtrlSeq2() {
	ctrl_bitseq.assign(2*n, false); // resize container for control sequence if necessary
	//cout << ctrl_bitseq.size();
	// first n bits b_0..b_{n-1} - random
	vector<bool>::iterator it = ctrl_bitseq.begin();
	vector<bool>::iterator end = ctrl_bitseq.begin() + n;
	int i = 0;
	int64 r = mt_eng();
    
	while(it != end) {
		if(i++ >= 64) {
			r = mt_eng();
			i = 0;
		}
		*(it++) = ((r & 1) != 0);
		r >>= 1;
	}
	// next n bits b_n..b_{2n-1} - random permutation of complements of b_0...b-{n-1}
	vector<bool>::iterator begin = ctrl_bitseq.begin();
	end = ctrl_bitseq.end();
	while(it != end) { // complement
		*(it++) = !(*(begin++));
	} // begin -> b_n, end -> b_2n
	shuffleBitseq(begin, end);
}


		
void FlawedPath::shuffleBitseq(vector<bool>::iterator begin, vector<bool>::iterator end) {
	shuffle(begin, end, mt_eng);
}

void FlawedPath::generateDyckPath(vector<bool>::iterator begin, vector<bool>::iterator end, bool b) {
	auto len = distance(begin, end); // 2k + 1
	// init bits - k 1s and k+1 0s
	fill(begin, begin + len/2, true);
	// fill(begin, begin + (b ? len/2 : (len+1)/2), true);
	// permute the (sub)vector of k 1s and k+1 0s
	shuffleBitseq(begin, end);
	// split into P1 and P2 at the path's minimum and swap P1 with P2
	swapSubpaths(begin, end, firstLowestLevelIterator(begin, end));
	// swapSubpaths(begin, end, b ? firstLowestLevelIterator(begin, end) : firstHighestLevel(begin, end));
	// if path corresponds to 0s in control sequence, reflect the path over OX
	if(!b) {
		for(vector<bool>::iterator it = begin; it != end; ++it) {
			*it = !(*it);
		} // or transform(begin, end, begin, myflip, [](const bool& b) {return !b;});
	}
	// discard last bit - no action required
}
	
// returns the iterator pointing to the bit s.t. after reading it the path reaches its lowest level for the first time
// -1 means that the path is empty; otherwise the minimum is reached after reading a least 1 bit
// the path is starting at level 0 and ends at level -1
// P1 is then b_{0}...b_{idx} and is empty iff the subpath is empty, P2 is b_{idx+1}...b_{2*n} and may be empty
vector<bool>::iterator FlawedPath::firstLowestLevelIterator(vector<bool>::iterator begin,
																vector<bool>::iterator end) {
	return firstLowestLevel(begin,end).first;
}

pair<vector<bool>::iterator, int64> FlawedPath::firstLowestLevel(vector<bool>::iterator begin, 
																	vector<bool>::iterator end) {
	auto len = distance(begin, end);
	if(len <= 0) { 
		return make_pair(begin - 1, 0);; // begin - 1 returned iff the subpath is empty
	}
	vector<bool>::iterator it_min = begin; 
	int64 lvl = 0;
	int64 min_lvl = 0;
	for(vector<bool>::iterator it = begin; it != end; ++it) {
		if(*it) {  
			++lvl;
		}
		else {
			--lvl;
			if(lvl < min_lvl) {
				min_lvl = lvl;
				it_min = it; 						
			}
		}
	}
	return make_pair(it_min, min_lvl);
}

void FlawedPath::findSubpathLenghts(vector<int64>& dyck_lenghts) {
	int64 lvl = 0;
	int64 len = 0;
	for(auto it = ctrl_bitseq.begin(); it != ctrl_bitseq.end(); ++it) {
		++len;
		bool above = lvl > 0;
		bool below = lvl < 0;
		(*it ? ++lvl : --lvl);
		if(lvl == 0) {
			if(above && !(*(it+1))) { // path moves from +1 -> 0 -> -1
				dyck_lenghts.push_back(len);
				len = 0;
			}
			else if(below && *(it+1)) { // path moves from -1 -> 0 -> +1
				dyck_lenghts.push_back(-len);
				len = 0;
			}
		}
	}
}
		
/*
 * Construct a Dyck Path from a given path by splitting it into P1|P2 at the point 
 * at which the first minimum is reached, constructing the path P2|P1 and discarding
 * the last bit.
 *
 * Implementation based on built-in function rotate 
 */
void FlawedPath::swapSubpaths(vector<bool>::iterator begin, vector<bool>::iterator end,
								vector<bool>::iterator idx) { // P1 -> b_0...b_idx; P2 -> b_{idx+1}...b_2n
		rotate(begin, idx + 1, end);		
}

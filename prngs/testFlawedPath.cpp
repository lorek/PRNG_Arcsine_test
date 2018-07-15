#include <iostream>
#include "FlawedPath.hpp"

using namespace std;

int main(int argc, char** argv) {
	
	FlawedPath fp;
	
    
	unsigned l1 = 1 << 4;  //length of sequence 
	unsigned seed = 135246; 
	fp.generateBitSequence2(l1, seed);
	fp.prettyPrintPath();
    cout << "\n";
    
    
    //just print bits:
    //fp.printBitsAscii();
    //cout << "\n";
    
	
	return 0;
}

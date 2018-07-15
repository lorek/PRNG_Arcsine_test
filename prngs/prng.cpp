#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <set>
#include <random>
#include <iterator>
#include <numeric>
#include "FlawedPath.hpp"

typedef long long int64;
typedef unsigned long long uint64;
typedef int int32;
typedef unsigned int uint32;

using namespace std;

uint64 pow2[64];   // pow2[i] = 2^i
uint64 pow2m1[65]; // pow2m1[i] = 2^i - 1

void initPow()
{
    pow2[0] = 1;
    pow2m1[0] = 0;
    pow2m1[64] = 1;
    for (int i = 1; i < 64; ++i)
    {
        pow2[i] = 2*pow2[i-1];
        pow2m1[i] = pow2[i] - 1;
        pow2m1[64] += pow2[i];
    }
}

void printPow()
{
    for (int i = 0; i < 64; ++i)
        printf("%llX\n", pow2[i]);
    for (int i = 0; i < 65; ++i)
        printf("%llX\n", pow2m1[i]);
}

class PRNG
{
public:
    virtual void setSeed(uint32 seed) = 0;
    virtual uint64 nextInt() = 0;
    virtual uint32 getNrOfBits() = 0;
};

class OneByte : public PRNG
{
public:
    OneByte(shared_ptr<PRNG> prng_, uint32 byteNr_)
        : prng(prng_)
        , byteNr(byteNr_)
    {
    }
    
    void setSeed(uint32 seed)
    {
        prng->setSeed(seed);
    }
    
    uint64 nextInt()
    {
        return getByte(prng->nextInt());
    }
    
    uint32 getNrOfBits()
    {
        return 8u;
    }
    
private:
    const shared_ptr<PRNG> prng;
    const uint32 byteNr;
    
    uint64 getByte(uint64 n)
    {
        n = n >> (byteNr * 8);
        return n & 255LLu;
    }
};

/*************************************************************
 *                                                           *
 *  A wrapping class for a PRNG which allows outputting      *
 *  only some selected bits from numbers actually produced   * 
 *  by that PRNG.                                            *
 *                                                           *
 *************************************************************/
class SomeBits : public PRNG
{
public:
    SomeBits(shared_ptr<PRNG> prng_, uint32 mostSig_, uint32 leastSig_)
        : prng(prng_)
        , mostSig(mostSig_)
        , leastSig(leastSig_)
        , nrOfBits(mostSig_ + 1 - leastSig_)
    {
    }
    
    void setSeed(uint32 seed)
    {
        prng->setSeed(seed);
    }
    
    uint64 nextInt()
    {
        return getBits(prng->nextInt());
    }
    
    uint32 getNrOfBits()
    {
        return nrOfBits;
    }
    
private:
    const shared_ptr<PRNG> prng;
    const uint32 mostSig;
    const uint32 leastSig;
    const uint32 nrOfBits;
    
    uint64 getBits(uint64 n)
    {
        n = n >> leastSig;
        return n & pow2m1[nrOfBits];
    }
};

shared_ptr<PRNG> getShifted(shared_ptr<PRNG> prng, uint32 shift)
{
    uint32 mg = prng->getNrOfBits() - 1;
    uint32 lg = shift;
    return shared_ptr<PRNG>(new SomeBits(prng, mg, lg));
}

/***********************************************
 *                                             *
 *  Linear congruential generator - LCG        *
 *                                             *
 ***********************************************/
class LCG : public PRNG
{
public:
    const uint64 M;
    const uint64 a, b;
    const uint32 nrOfBits;
    
    LCG(uint64 M_, uint64 a_, uint64 b_, const uint32 nrOfBits_)
        : M(M_)
        , a(a_)
        , b(b_)
        , nrOfBits(nrOfBits_)
    {
    }
    
    void setSeed(uint32 seed)
    {
        this->seed = seed;
    }
    
    uint64 nextInt()
    {
        seed = (a * seed + b) % M;
        return seed & pow2m1[nrOfBits];
    }
    
    uint32 getNrOfBits()
    {
        return nrOfBits;
    }
    
private:
    uint64 seed = 1;
};

/***************************************************
 *                                                 *
 *  Combined Multiple Recursive Generator - CMRG   *
 *                                                 *
 ***************************************************/
class CMRG : public PRNG
{
public:
    CMRG()
    {
        reset();
    }
    
    void setSeed(uint32 seed)
    {
        x[0] = 0;
        x[1] = seed;
        x[2] = 0;
        y[0] = 0;
        y[1] = 0;
        y[2] = seed;
        n = 0;
    }
    
    uint64 nextInt()
    {
        int64 nextx = xa *  x[(n+1)%3]  -  xb * x[n];
        int64 nexty = ya *  y[(n+2)%3]  -  yb * y[n];
        nextx = mymod(nextx, xm);
        nexty = mymod(nexty, ym);
        x[n] = nextx;
        y[n] = nexty;
        n = (n + 1)%3;
        return static_cast<uint64>( (zm + nextx - nexty) % zm );
    }
    
    uint32 getNrOfBits()
    {
        return 31;
    }
    
private:
    int64 x[3], y[3];
    const int64 xa = 63308;
    const int64 xb = 183326;
    const int64 xm = 2147483647LL;
    const int64 ya = 86098;
    const int64 yb = 539608;
    const int64 ym = 2145483479LL;
    const int64 zm = 2147483647LL;
    int n;
    
    void reset()
    {
        setSeed(1);
    }
    
    int64 mymod(int64 a, int64 m)
    {
        if (m == 0)
            return a;
        if (m < 0)
            return mymod(-a, -m);
        int64 res = a % m;
        if (res < 0)
            res += m;
        return res;
    }
};

/*****************************************************
 *                                                   *
 *  C_PRG - GLIBC standard library rand() function   *
 *          wrapped in the PRNG class                *
 *                                                   *
 *****************************************************/
class C_PRG : public PRNG
{
public:
    void setSeed(uint32 seed)
    {
        srand(seed);
    }
    
    uint64 nextInt()
    {
        return static_cast<uint64>(rand());
    }
    
    uint32 getNrOfBits()
    {
        return 31;
    }
};

class BorlandPRNG : public PRNG
{
    uint64 nextInt()
    {
        myseed = myseed * 0x015A4E35 + 1;
        return static_cast<uint64>( (myseed >> 16) & 0x7FFF );
    }
    
    void setSeed(uint32 seed)
    {
        myseed = seed;
        //myrand();
    }
    
    uint32 getNrOfBits()
    {
        return 15;
    }
    
private:
    uint32 myseed = 0x015A4E36;
};


/*******************************************************************************
 *                                                                             *
 *  VisualPRNG - custom implementation of the LCG used in the implementation   *
                 of the Microsoft Visual C++ rand() function                   *
 *                                                                             *
 *******************************************************************************/
class VisualPRNG : public PRNG
{
	
	// 0x343FDu  =   214 013
	// 0x269EC3u = 2 531 011
    uint64 nextInt()
    {
        myseed = myseed * 0x343FDu + 0x269EC3u;
        return static_cast<uint64>( (myseed >> 16) & 0x7FFF ); // 0x7FFF = 2^15 - 1 = 32 767 - 15 MSBs
    }
    
    void setSeed(uint32 seed)
    {
        myseed = seed;
        //myrand();
    }
    
    uint32 getNrOfBits()
    {
        return 15;
    }
    
private:
    uint32 myseed = 1;
};


/**********************************************************************************
 *                                                                                *
 *  Mersenne - C++11 implementation of the Mersenne Twister PRNG MT19937          *
 *             implemented as the class PRNG (for convenience, to be compliant    *
 *             with common interface of all custom implementations of tested      *
 *             PRNGs).                                                            *
 *                                                                                *
 **********************************************************************************/
class Mersenne : public PRNG
{
public:
    void setSeed(uint32 seed)
    {
        eng.seed(seed);
    }
    
    uint64 nextInt()
    {
//         //uint64 r = static_cast<uint64>(eng());
        //fprintf(stderr, "%llu\n", r & pow2m1[63]);
        //return r & pow2m1[63];
        return static_cast<uint64>(eng());
    }
    
    uint32 getNrOfBits()
    {
        return 64;
    }
    
    mt19937_64 eng;
};

/**********************************************************************************
 *                                                                                *
 *  Flawed -  Hypothetical flawed PRNG - for 1 in every 100 seeds it returns the  *
 *            sequence of bits given by 10(0110)*01; otherwise it invokes         *
 *            the generator MT19937-64 and returns its output.                    *
 *                                                                                *
 **********************************************************************************/
class Flawed : public PRNG
{
public:
    void setSeed(uint32 seed)
    {
        seedNr++;
        eng.seed(seed);
    }
    
    uint64 nextInt()
    {
        if (seedNr % 100 == 0)
            //return 0x5555555555555555LLu;
            return 0x9999999999999999LLu;
        else
            return static_cast<uint64>(eng());
    }
    
    uint32 getNrOfBits()
    {
        return 64;
    }
    
    int seedNr = 0;
    mt19937_64 eng;
};

/**********************************************************************************
 *                                                                                *
 *  FlawedDyck - Another version of a hypothetical flawed PRNG based on Dyck Path *
 *               The output of this generator are 64-bit integers constructed     *
 *               from consecutive bits of a randomly sampled Dyck Path of given   *
 *               length (by default 2^26).                                        *
 *               If all bits from a given paths were used, a new random path      *
 *               of given length is generated.                                    *
 *                                                                                *
 * NOTE THAT GENERATING VERY LONG PATHS MAY TAKE A LONG TIME                      *
 *                                                                                *
 **********************************************************************************/
class FlawedDyck : public PRNG
{
public:
	FlawedDyck() : FlawedDyck(26) {}
	
	FlawedDyck(uint32 path_loglen, bool do_init = true) : 
		path_n(1 << (path_loglen < 6 ? 4 : path_loglen - 2)) {
			if(do_init) { initializePath(); }
		}

    void setSeed(uint32 seed) {
		// generate new path using given seed for the underlying prng
        initializePath(seed);
    }
    
    uint64 nextInt() {
        // generate new path if current path contains too few "fresh" bits
		if(distance(it_path, eng.itPathEnd()) < getNrOfBits()) {
			initializePath();
		}
		auto it = it_path;
		it_path += getNrOfBits();
		uint64 r = 0;
		while(it != it_path) {
			r <<= 1;
			if(*(it++)) ++r; // "safer" and "cleaner" than r = 2*r+(*it)
		}
		return r;
    }
    
    uint32 getNrOfBits() {
        return 64;
    }
   
private:
	int64 path_n; // length of the Dyck Path is 4n
    
	FlawedPath eng;
	vector<bool>::iterator it_path;
	
	void initializePath() {
		eng.generateBitSequence2(path_n);
		it_path = eng.itPathBegin();
	}
	
	void initializePath(uint32 seed) {
		eng.generateBitSequence2(path_n, seed);
		it_path = eng.itPathBegin();
	}
};

/**********************************************************************************
 *                                                                                *
 *  FlawedDyckMT - Hypothetical flawed PRNG                                       *
 *                 For 1 in every {step} seeds it returns the bits of randolmy    *
 *                 generated FlawedPath (see FlawedDyck); otherwise it invokes    *
 *                 the generator MT19937-64 and returns its output.               *
 *                                                                                *
 **********************************************************************************/
class FlawedDyckMT : public PRNG
{
public:
	FlawedDyckMT() : FlawedDyckMT(26, 100) {}
	
	FlawedDyckMT(uint32 path_loglen, uint32 step, bool do_init = true) : 
		path_n(1 << (path_loglen < 6 ? 4 : path_loglen - 2)),
		step(step == 0 ? 1 : step) {
			if(do_init) { initializePath(); }
		}

    void setSeed(uint32 seed) {
        if(++seedNr % step == 0) {
        	// generate new path only if required
		    initializePath(seed); // the underlying prng will use given seed
		}
		else {
			mt_eng.seed(seed);
		}
    }
    
    uint64 nextInt() {
        if (seedNr % step == 0) {
            return nextIntFromPath();
		}
        else {
            return static_cast<uint64>(mt_eng());
		}
    }
    
    uint32 getNrOfBits() {
        return 64;
    }
   
private:
    int seedNr = 0;
	uint32 step; 
	int64 path_n; // length of the Dyck Path is 4n
    
	FlawedPath path_eng;
	mt19937_64 mt_eng;

	vector<bool>::iterator it_path;
	
	void initializePath() {
		path_eng.generateBitSequence2(path_n);
		it_path = path_eng.itPathBegin();
	}

	void initializePath(uint32 seed) {
		path_eng.generateBitSequence2(path_n, seed);
		it_path = path_eng.itPathBegin();
	}

	uint64 nextIntFromPath() {
		// generate new path if current path contains too few "fresh" bits
		if(distance(it_path, path_eng.itPathEnd()) < getNrOfBits()) {
			initializePath();
		}
		auto it = it_path;
		it_path += getNrOfBits();
		uint64 r = 0;
		while(it != it_path) {
			r <<= 1;
			if(*(it++)) ++r; // "safer" and "cleaner" than r = 2*r+(*it)
		}
		return r;
	}
	
};

class RandU : public PRNG
{
public:
    void setSeed(uint32 seed)
    {
        s = seed + (seed % 2 == 0 ? 1 : 0);
    }
    
    uint64 nextInt()
    {
        s = (65539llu * s) % pow2[31];
        return s;
    }
    
    uint32 getNrOfBits()
    {
        return 31;
    }
    
    uint64 s;
};

class GeneratorInvoker
{
public:
    GeneratorInvoker() = default;
    
    GeneratorInvoker(shared_ptr<PRNG>& prng_)
        : prng(prng_)
    {
    }
    
    GeneratorInvoker(shared_ptr<PRNG>& prng_, int64 nrOfSeedsToSkip_)
        : prng(prng_)
        , nrOfSeedsToSkip(nrOfSeedsToSkip_)
    {
    }
    
    GeneratorInvoker(const GeneratorInvoker&) = delete;
    
    ~GeneratorInvoker()
    {
        if (seeds)
            fclose(seeds);
    }
    
    GeneratorInvoker& operator=(const GeneratorInvoker&) = delete;
    
    void setPRNG(PRNG& prng)
    {
        this->prng = shared_ptr<PRNG>(&prng);
    }
    
    void setPRNG(shared_ptr<PRNG>& prng)
    {
        this->prng = prng;
    }
    
    void setPathToSeeds(char* pathToFile)
    {
        seeds = fopen(pathToFile, "r");
        if (!seeds)
        {
            printf("Couldn't open %s\n", pathToFile);
            exit(1);
        }
    }
    
    void run(int64 nrOfStrings, int64 length, bool write_data_len)
    {
		fprintf(stderr, "GeneratorInvoker::run(%lld, %lld)\n", nrOfStrings, length);
        freopen (NULL, "wb", stdout);
        
        nrOfStrings -= nrOfSeedsToSkip;
		if(write_data_len) {
			fwrite(&nrOfStrings, sizeof(int64), 1, stdout);
			fwrite(&length, sizeof(int64), 1, stdout);
		}
        
        skipSeeds();
        
        for (int64 i = 1; i <= nrOfStrings; ++i)
        {
            prng->setSeed(nextSeed());
            
            if (i % 100 == 0)
                fprintf(stderr, "Generator: %lld/%lld\n", i, nrOfStrings);
            generateString(length);
        }
        fclose(stdout);
    }
    
    void skipSeeds()
    {
        for (int i = 0; i < nrOfSeedsToSkip; ++i)
            nextSeed();
    }
    
    void run(int64 length, bool write_data_len)
    {
        int nrOfStrings = getNextIntFromFile();
        run(nrOfStrings, length, write_data_len);
    }
    
private:
    shared_ptr<PRNG> prng;
    FILE* seeds = 0;
    int64 nrOfSeedsToSkip = 0;
    uint64 curr;
    int filled;
    
    void generateString(uint64 nrOfBits)
    {
        uint64 nrOfChunks = nrOfBits / 64;
        curr = 0;
        filled = 0;
        for (uint64 i = 0; i < nrOfChunks; ++i)
        {
            uint64 chunk = nextChunk();
            //fprintf(stderr, "filled = %d\n", filled);
            fwrite(&chunk, sizeof(uint64), 1, stdout);
        }
    }
    
    uint64 nextChunk()
    {
        uint64 r = 0;
        int nrOfBits = prng->getNrOfBits();
        while (filled < 64)
        {
            r = prng->nextInt();
            curr += (r << filled);
            filled += nrOfBits;
        }
        int used = nrOfBits + 64 - filled;
        uint64 res = curr;
        curr = used < 64 ? (r >> used) : 0;
        filled = nrOfBits - used;
        return res;
    }
    
    int nextSeed()
    {
        static int def_first_seed = 112358;
        
        if (seeds)
            return getNextIntFromFile() + 1000000001;
        else
            return def_first_seed++;
    }
    
    int getNextIntFromFile()
    {
        int val;
        fscanf(seeds, "%d", &val);
        return val;
    }
};

void wrongArgs(int argc, char** argv)
{
        printf("Usage: %s [prng name] [number of strings | path to seeds] [log2 of length >= 6] [nrOfSeeds to skip] [-nolen] \n", argv[0]);
        exit(1);
}

shared_ptr<PRNG> getPRNG(char* name, uint32 log_len = 26)
{	
    // TODO refactor
	// - remove if-cascade
	// - replace char* with std::string and strcmp(s1, s2) with s1.compare(s2)
    if (strcmp(name, "z_czapy") == 0)
    {
        return shared_ptr<PRNG>(new LCG(1e9, 1234, 3, 8));
    }
    else if (strcmp(name, "Rand") == 0)
    {
        return shared_ptr<PRNG>(new LCG(2147483648LL, 1103515245, 12345, 31));
    }
    else if (strcmp(name, "Rand0") == 0)
    {
        return shared_ptr<PRNG>(new LCG(2147483648LL, 1103515245, 12345, 8));
    }
    else if (strcmp(name, "Rand1") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new LCG(2147483648LL, 1103515245, 12345, 31)),
                15, 8
            ) );
    }
    else if (strcmp(name, "Rand3") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new LCG(2147483648LL, 1103515245, 12345, 31)),
                30, 23
            ) );
    }
    else if (strcmp(name, "Minstd") == 0)
    {
        return shared_ptr<PRNG>(new LCG(2147483647, 16807, 0, 31));
    }
    else if (strcmp(name, "Minstd0") == 0)
    {
        return shared_ptr<PRNG>(new LCG(2147483647, 16807, 0, 8));
    }
    else if (strcmp(name, "Minstd1") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new LCG(2147483647, 16807, 0, 31)),
                15, 8
            ) );
    }
    else if (strcmp(name, "NewMinstd") == 0)
    {
        return shared_ptr<PRNG>(new LCG(2147483647, 48271, 0, 31));
    }
    else if (strcmp(name, "NewMinstd0") == 0)
    {
        return shared_ptr<PRNG>(new LCG(2147483647, 48271, 0, 8));
    }
    else if (strcmp(name, "NewMinstd1") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new LCG(2147483647, 48271, 0, 31)),
                15, 8
            ) );
    }
    else if (strcmp(name, "NewMinstd3") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new LCG(2147483647, 48271, 0, 31)),
                30, 23
            ) );
    }
    else if (strcmp(name, "CMRG") == 0)
    {
        return shared_ptr<PRNG>(new CMRG());
    }
    else if (strcmp(name, "CMRG0") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new CMRG()),
                7, 0
            ) );
    }
    else if (strcmp(name, "CMRG1") == 0)
    {
        return shared_ptr<PRNG>(
            new SomeBits(
                shared_ptr<PRNG>(new CMRG()),
                15, 8
            ) );
    }
    else if (strcmp(name, "SBorland") == 0)
    {
        return getShifted(shared_ptr<PRNG>(new BorlandPRNG()), 7);
    }
    else if (strcmp(name, "C_PRG") == 0)
    {
        return shared_ptr<PRNG>(new C_PRG());
    }
    else if (strcmp(name, "SVIS") == 0)
    {
        return getShifted(shared_ptr<PRNG>(new VisualPRNG()), 7); // Discard 7 LSBs produced by the PRNG
    }
    else if (strcmp(name, "Mersenne") == 0)
    {
        return shared_ptr<PRNG>(new Mersenne());
    }
    else if (strcmp(name, "RANDU") == 0)
    {
        return shared_ptr<PRNG>(new RandU());
    }
    else if (strcmp(name, "zepsuty") == 0)
    {
        return shared_ptr<PRNG>(new Flawed());
    }
	else if(strcmp(name, "FlawedDyck") == 0)
	{
		return shared_ptr<PRNG>(new FlawedDyck(log_len));
	}
	else if(strcmp(name, "FlawedDyckMT") == 0)
	{
		return shared_ptr<PRNG>(new FlawedDyckMT(log_len, 100));
	}
    return shared_ptr<PRNG>();
}

int64 myPow(int64 a, uint32 b)
{
    int64 res = 1;
    for (uint32 i = 0; i < b; ++i)
        res *= a;
    return res;
}

int main(int argc, char** argv)
{
    initPow();
    //printPow();
    
    if (argc < 4)
        wrongArgs(argc, argv);
    
    uint32 logLength = atoi(argv[3]);
    int64 nrOfStrings = atoi(argv[2]);
    if (logLength < 6) {
        wrongArgs(argc, argv); // calls exit()
	}
    
	shared_ptr<PRNG> prng = getPRNG(argv[1], logLength);
    if (!prng)
    {
        printf("Unknown prng: %s\n", argv[1]);
        exit(1);
    }
    int64 length = myPow(2LL, logLength);
    int64 skip = 0;
	bool write_data_len = true;
    if (argc == 5) {
		if(strcmp(argv[4], "-nolen") == 0) {
			write_data_len = false;
		}
		else {
			skip = atoi(argv[4]);
		}
    }
	else if (argc == 6) {
		skip = atoi(argv[4]);
		if(strcmp(argv[5], "-nolen") == 0) {
			write_data_len = false;
		}
	}
    GeneratorInvoker gi(prng, skip);
    if (nrOfStrings <= 0)
    {
        gi.setPathToSeeds(argv[2]);
        gi.run(length, write_data_len);
    }
    else
    {
        gi.run(nrOfStrings, length, write_data_len);
    }
	// cout << "Test FlawedDyck" << endl;
	// shared_ptr<PRNG> fd = shared_ptr<PRNG>(new FlawedDyck(9)); 
	// for(int i = 0; i < 8; ++i) {
		// fd->nextInt();
	// }
    // return 0;
}

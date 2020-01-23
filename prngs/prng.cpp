#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <set>
#include <random>
#include <iterator>
#include <numeric>
#include <iostream>

#include <cln/cln.h>
#include "FlawedPath.hpp"

typedef long long long64;
typedef unsigned long long ulong64;
typedef int int32;
typedef unsigned int uint32;

using namespace std;

ulong64 pow2[64];   // pow2[i] = 2^i
ulong64 pow2m1[65]; // pow2m1[i] = 2^i - 1

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

long64 myPow(long64 a, uint32 b)
{
    long64 res = 1;
    for (uint32 i = 0; i < b; ++i)
        res *= a;
    return res;
}

/**
 * calculates (base^exp) % mod using fast modular exponentiation
 */
ulong64 powerMod(ulong64 base, ulong64 exp, ulong64 mod)
{
	ulong64 res = 1;
	base %= mod;
	
	while(exp)
	{
		if(exp & 1) 
			res = (res * base) % mod;
		
		base = (base * base) % mod;
		exp >>= 1; // exp = exp / 2;
	}
	return res;
}

class PRNG
{
public:
    virtual void setSeed(uint32 seed) = 0;
    virtual ulong64 nextInt() = 0;
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
    
    ulong64 nextInt()
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
    
    ulong64 getByte(ulong64 n)
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
    
    ulong64 nextInt()
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
    
    ulong64 getBits(ulong64 n)
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
    const ulong64 M;
    const ulong64 a, b;
    const uint32 nrOfBits;
    
    LCG(ulong64 M_, ulong64 a_, ulong64 b_, const uint32 nrOfBits_)
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
    
    ulong64 nextInt()
    {
        seed = (a * seed + b) % M;
        return seed & pow2m1[nrOfBits];
    }
    
    uint32 getNrOfBits()
    {
        return nrOfBits;
    }
    
private:
    ulong64 seed = 1;
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
    
    ulong64 nextInt()
    {
        long64 nextx = xa *  x[(n+1)%3]  -  xb * x[n];
        long64 nexty = ya *  y[(n+2)%3]  -  yb * y[n];
        nextx = mymod(nextx, xm);
        nexty = mymod(nexty, ym);
        x[n] = nextx;
        y[n] = nexty;
        n = (n + 1)%3;
        return static_cast<ulong64>( (zm + nextx - nexty) % zm );
    }
    
    uint32 getNrOfBits()
    {
        return 31;
    }
    
private:
    long64 x[3], y[3];
    const long64 xa = 63308;
    const long64 xb = 183326;
    const long64 xm = 2147483647LL;
    const long64 ya = 86098;
    const long64 yb = 539608;
    const long64 ym = 2145483479LL;
    const long64 zm = 2147483647LL;
    int n;
    
    void reset()
    {
        setSeed(1);
    }
    
    long64 mymod(long64 a, long64 m)
    {
        if (m == 0)
            return a;
        if (m < 0)
            return mymod(-a, -m);
        long64 res = a % m;
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
    
    ulong64 nextInt()
    {
        return static_cast<ulong64>(rand());
    }
    
    uint32 getNrOfBits()
    {
        return 31;
    }
};

class BorlandPRNG : public PRNG
{
    ulong64 nextInt()
    {
        myseed = myseed * 0x015A4E35 + 1;
        return static_cast<ulong64>( (myseed >> 16) & 0x7FFF );
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
    ulong64 nextInt()
    {
        myseed = myseed * 0x343FDu + 0x269EC3u;
        return static_cast<ulong64>( (myseed >> 16) & 0x7FFF ); // 0x7FFF = 2^15 - 1 = 32 767 - 15 MSBs
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
    
    ulong64 nextInt()
    {
//         //ulong64 r = static_cast<ulong64>(eng());
        //fprintf(stderr, "%llu\n", r & pow2m1[63]);
        //return r & pow2m1[63];
        return static_cast<ulong64>(eng());
    }
    
    uint32 getNrOfBits()
    {
        return 64;
    }
    
    mt19937_64 eng;
};

/**********************************************************************************
 *                                                                                *
 *  BBS -  custom implementation of the Blum-Blum-Shub PRNG. This is a quadratic  *
 *         congruential generator. Blum-Blum-Shub is an example of                *
 *         a cryptographically secure PRNG.                                       *
 *         This class extends the generic class PRNG for PRNGs. Each invokation   *
 *         of this ghenerator produces an integer (of type unsigned long,         *
 *         typically 64-bit) constructed from bits outputted in the consecutive   *
 *         rounds of BBS.                                                         *
 *                                                                                *
 *         The internal implementation of this PRNG uses classes and methods      *
 *         for computations with large numbers from the CLN library               *
 *                                                                                *
 **********************************************************************************/
class BBS64_PRNG : public PRNG
{
public:
	
	BBS64_PRNG(const unsigned long p_, const unsigned long q_) :
		p(p_),
		q(q_),
		n(p * q)
	{		
		check_values();
	}
	
	BBS64_PRNG(const char* p_, const char* q_) :
		p(p_),
		q(q_),
		n(p * q)
	{		
		check_values();
	}
	
	void setSeed(uint32 seed)
    {
		this->state = seed;	
		
		if(cln::mod(state, p) == 0 || cln::mod(state, q) == 0)
			throw std::invalid_argument("BBS64_PRNG :: The seed should be co-prime with n = p*q.");
    }
	
	void setSeed(const char* seed) 
	{
		this->state = seed;
		
		if(cln::mod(state, p) == 0 || cln::mod(state, q) == 0)
			throw std::invalid_argument("BBS64_PRNG :: The seed should be co-prime with n = p*q.");
    }
	 
    ulong64 nextInt()
    {
		cln::cl_I r = 0;
		for(int i = 0; i < bw; ++i)
		{
			state = cln::mod(state * state, n);
			r <<= 1;
			r += (state & 1);
		}
		return cln::cl_I_to_ulong(r);

    }
	
	// prints a binary string consisting of bw bits generated by the generator
	//   -> for testing and debugging purposes
	string next_rnd_binary()
	{
		string s;
		for(int i = 0; i < bw; ++i)
		{
			state = cln::mod(state * state, n);
			s.push_back('0' + cln::cl_I_to_ulong(state & 1));
		}
		return s;
	}
	
    uint32 getNrOfBits()
    {
        return bw;
    }
	
private:
	void check_values() 
	{
		if(!cln::isprobprime(p))
			throw std::invalid_argument("BBS64_PRNG :: The paramater p must be a prime number.");
		if((p & 3) != 3)
			throw std::invalid_argument("BBS64_PRNG :: The paramater p should be congruent to 3 (mod 4).");
		if(!cln::isprobprime(q))
			throw std::invalid_argument("BBS64_PRNG :: The paramater q must be a prime number.");
		if((q & 3) != 3)
			throw std::invalid_argument("BBS64_PRNG :: The paramater q should be congruent to 3 (mod 4).");
	}
	
	static const uint32 bw = 8*sizeof(unsigned long); // bit width
	
	const cln::cl_I p;
	const cln::cl_I q;
	const cln::cl_I n;
	cln::cl_I state = 2;
    
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
    
    ulong64 nextInt()
    {
        if (seedNr % 100 == 0)
            //return 0x5555555555555555LLu;
            return 0x9999999999999999LLu;
        else
            return static_cast<ulong64>(eng());
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
    
    ulong64 nextInt() {
        // generate new path if current path contains too few "fresh" bits
		if(distance(it_path, eng.itPathEnd()) < getNrOfBits()) {
			initializePath();
		}
		auto it = it_path;
		it_path += getNrOfBits();
		ulong64 r = 0;
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
	long64 path_n; // length of the Dyck Path is 4n
    
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
    
    ulong64 nextInt() {
        if (seedNr % step == 0) {
            return nextIntFromPath();
		}
        else {
            return static_cast<ulong64>(mt_eng());
		}
    }
    
    uint32 getNrOfBits() {
        return 64;
    }
   
private:
    int seedNr = 0;
	uint32 step; 
	long64 path_n; // length of the Dyck Path is 4n
    
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

	ulong64 nextIntFromPath() {
		// generate new path if current path contains too few "fresh" bits
		if(distance(it_path, path_eng.itPathEnd()) < getNrOfBits()) {
			initializePath();
		}
		auto it = it_path;
		it_path += getNrOfBits();
		ulong64 r = 0;
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
    
    ulong64 nextInt()
    {
        s = (65539llu * s) % pow2[31];
        return s;
    }
    
    uint32 getNrOfBits()
    {
        return 31;
    }
    
    ulong64 s;
};

class GeneratorInvoker
{
public:
    GeneratorInvoker() = default;
    
    GeneratorInvoker(shared_ptr<PRNG>& prng_)
        : prng(prng_)
    {
    }
    
    GeneratorInvoker(shared_ptr<PRNG>& prng_, long64 nrOfSeedsToSkip_)
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
    
    void run(long64 nrOfStrings, long64 length, bool write_data_len)
    {
		fprintf(stderr, "GeneratorInvoker::run(%lld, %lld)\n", nrOfStrings, length);
        freopen (NULL, "wb", stdout);
        
        nrOfStrings -= nrOfSeedsToSkip;
		if(write_data_len) {
			fwrite(&nrOfStrings, sizeof(long64), 1, stdout);
			fwrite(&length, sizeof(long64), 1, stdout);
		}
        
        skipSeeds();
        
        for (long64 i = 1; i <= nrOfStrings; ++i)
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
    
    void run(long64 length, bool write_data_len)
    {
        int nrOfStrings = getNextIntFromFile();
        run(nrOfStrings, length, write_data_len);
    }
    
private:
    shared_ptr<PRNG> prng;
    FILE* seeds = 0;
    long64 nrOfSeedsToSkip = 0;
    ulong64 curr;
    int filled;
    
    void generateString(ulong64 nrOfBits)
    {
        ulong64 nrOfChunks = nrOfBits / 64;
        curr = 0;
        filled = 0;
        for (ulong64 i = 0; i < nrOfChunks; ++i)
        {
            ulong64 chunk = nextChunk();
            //fprintf(stderr, "filled = %d\n", filled);
            fwrite(&chunk, sizeof(ulong64), 1, stdout);
        }
    }
    
    ulong64 nextChunk()
    {
        ulong64 r = 0;
        int nrOfBits = prng->getNrOfBits();
        while (filled < 64)
        {
            r = prng->nextInt();
            curr += (r << filled);
            filled += nrOfBits;
        }
        int used = nrOfBits + 64 - filled;
        ulong64 res = curr;
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
        printf("Usage: %s [prng name] [number of strings | path to seeds] [log2 of length >= 6] [nrOfSeeds to skip] [-nolen] [-f step size for FlawedDyckMT] \n", argv[0]);
        exit(1);
}

shared_ptr<PRNG> getPRNG(char* name, uint32 log_len = 26, long64 step_flawed = 100)
{	
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
		return shared_ptr<PRNG>(new FlawedDyckMT(log_len, step_flawed));
	}
	else if(strncmp(name, "BBS", 3) == 0) // name == BBS or name == BBS_p_q
	{
		char* p = NULL;
		char* q = NULL;
		if(strlen(name) == 3)
		{ // Default parameters - two 64-bit prime numbers
			p = (char*) "11234773052181932039";
			q = (char*) "15755662711309472467";
		}
		else
		{ 
			p = strtok(name + 3, "_");
			if(p != NULL)
			{
				q = strtok(NULL, "_");
			}
		}
		if(p != NULL && q != NULL)
		{
			return shared_ptr<PRNG>(new BBS64_PRNG(p, q));
		}
	}
    return shared_ptr<PRNG>();
}

int main(int argc, char** argv)
{
    initPow();
    //printPow();
    
    if (argc < 4)
        wrongArgs(argc, argv);
    
    uint32 logLength = atoi(argv[3]);
    long64 nrOfStrings = atoi(argv[2]);
    if (logLength < 6) {
        wrongArgs(argc, argv); // calls exit()
	}
    
	long64 length = myPow(2LL, logLength);
    long64 skip = 0;
	long64 step_flawed = 100;
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
		if(strcmp(argv[4], "-f") == 0) {
			step_flawed = atoi(argv[5]);
		}
		else {
			skip = atoi(argv[4]);
			if(strcmp(argv[5], "-nolen") == 0) {
				write_data_len = false;
			}
		}
		
	}
	else if (argc == 7) {
		if(strcmp(argv[4], "-nolen") == 0) {
			write_data_len = false;
		}
		else {
			skip = atoi(argv[4]);
		}
		if(strcmp(argv[5], "-f") == 0) {
			step_flawed = atoi(argv[6]);
		}
	}
	else if (argc > 7) {
		skip = atoi(argv[4]);
		if(strcmp(argv[5], "-nolen") == 0) {
			write_data_len = false;
		}
		if(strcmp(argv[6], "-f") == 0) {
				step_flawed = atoi(argv[7]);
		}
	}
	
	shared_ptr<PRNG> prng = getPRNG(argv[1], logLength, step_flawed);
    if (!prng)
    {
        printf("Unknown prng: %s\n", argv[1]);
        exit(1);
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
    return 0;
}

# PRNG_Arcsine_test
Empirical tests for PRNGs based on the arcsine law

Tools for empirical test for PRNGs based on the Arcsine Law, the method is described in a paper: Paweł Lorek, Grzegorz Łoś, Filip Zagórski and Karol Gotfryd "On testing pseudorandom generators via statistical tests based on the arcsine law".
(to do: put link here)

## Requirements
Tool was written and tested in `The Julia Language v0.6.2`.

## PRNGs
Our implementation of PRNGs are in folder `prngs`. Main file: `prngs\prng.cpp`. Compiling:
````
[user@machine PRNG_Arcsine_test/prngs]$ g++ -O2 -std=c++17 -o prng.o prng.cpp ./FlawedPath.cpp
````

```Usage: ./prng.o [prng name] [number of strings | path to seeds] [log2 of length >= 6] [nrOfSeeds to skip] [-nolen] ```
where
* `prng_name` is one of `Rand, Rand0, Rand1, Rand3, Minstd, Minstd0, Minstd1, NewMinstd, NewMinstd1, NewMinstd3, CMRG, CMRG0, CMRG1, SBorland, C_PRG, SVIS, Mersenne, RANDU, zepsuty, FlawedDyck, FlawedDyckMT`
In the article we used only `Rand` (BSD lib rand()), `SVIS` (Microsoft Visual C++ rand()), `C_PRG` (GLIBC stdlib rand()), `NewMinstd3` (Minstd with multiplier 48271), `Mersenne` (Mersenne Twister mt19937_64) `FlawedDyckMT` (every hundredth sequence is flawed (based on Dych Paths) so that it is exactly half of the time above x-axis, and half of the time above; otherwise it is an ouput of `Mersenne`)
* ` [number of strings | path to seeds]`:  `number of strings` number of sequences to produce; or `path to seed`, a path to file with seeds (it will produce as many sequences as seeds in this file)
* `[log2 of length >= 6]` log2 of the length of each sequence
* `[-nolen]` by default, fist bits of output is the length of each sequence (needed for implemenations of our Arcsine tester). Can be useful if we want to produce only bits for another tester, e.g., for TestU01 of NIST Test Sutie.


## Testing PRNGs
Program consists of several modules. From user's perspective, the starting point is `jl/Main.jl`. Program reads a bit stream from stdin. The results are written in stdout.  
Usage 
```Usage: julia Main.jl [lil|asin] [nrOfCheckPoints] [pathToFile] [writeMode]```
* `[lil|asin]` a test to apply, LIL (Law Iterated Logarithm) or ASIN (Arscine Law Test)
* `[nrOfCheckPoints]` calculate statistics not only for whole length, but also for intemediate points.

Assuming that we have compiled `prngs/prng.cpp` as `prngs/prng.o`, the typical usage is following:

````
[user@machine PRNG_Arcsine_test]$  prngs/prng.o SVIS seeds/setAll.txt 6 | julia jl/Main.jl asin 4 
````
I.e., we use Microsoft Visual C++ rand() as PRNG, `seeds/setAll.txt` as seeds (there are 10000 of them in this file); each sequence is of length 2^6; we perform ASIN test and want statistics to be also computed after 2^2, 2^3, 2^4 and 2^5 steps.
Result:
````
       ;   String["2^2", "2^3", "2^4", "2^5", "2^6"]
     tv;   String["0.8414", "0.8047", "0.7236", "0.5492", "0.177"]
   sep1;   String["1.0", "1.0", "1.0", "1.0", "1.0"]
   sep2;   String["1.0", "1.0", "1.0", "1.0", "1.0"]
  p-val;   String["0.0", "0.0", "0.0", "0.0", "0.0"]
````

 
## Prepared bash scripts
In folder `scripts` we placed bash script used to generate most tables in the article. E.g., file `scripts/go_Mersenne_asin` is following:

````
#!/bin/bash
(time prngs/prng.o Mersenne seeds/setAll.txt 34 | julia jl/Main.jl asin 8) > results/Mersenne_asin_len34_mid_points_1.txt 2>&1
````
It will use 10000 seeds from  `seeds/setAll.txt` to generate 10000 sequences of length 2^34 each from PRNG `Mersenne`. The output will be placed in file `results/Mersenne_asin_len34_mid_points_1.txt`. To run it:
```
[user@machine PRNG_Arcsine_test]$ ./scripts prngs/prng.o SVIS seeds/setAll.txt 6 | julia jl/Main.jl asin 4 
````


## Testing own PRNGs
The input stream must have the following format:
* First 8 bytes contains 64-bit integer `nrOfStrings` (number of sequences)
* Next 8 bytes contains 64-bit integer `length` (length of each sequence)
* Next, `nrOfStrings * length/8` bytes of data


## ASCII Path of PRNG `Flawed` 
In `/prngs` we placed sample file to display in ASCII a path of PRNG `Flawed`. File `prngs/testFlawedPath.cpp` generates path of length 2^6.
Compilation:
```
 g++ -O2 -std=c++17 -o testFlawedPath ./testFlawedPath.cpp ./FlawedPath.cpp 
```
Result:

```
[peyo@peyo prngs]$ ./testFlawedPath 
--------------------------------------------------------------------------------
Path of length 64
1,0,0,1,0,1,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,11,0,0,1,0,1,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,1,

----------------------------------------
1     |\
0     |/
0    /|
1    \|
0    /|
1    \|
1     |\
1     | \
1     |  \
0     |  /
1     |  \
0     |  /
0     | /
1     | \
1     |  \
0     |  /
0     | /
1     | \
0     | /
1     | \
1     |  \
0     |  /
0     | /
0     |/
0    /|
1    \|
0    /|
1    \|
0    /|
1    \|
1     |\
0     |/
0    /|
1    \|
1     |\
0     |/
1     |\
0     |/
0    /|
1    \|
0    /|
1    \|
0    /|
1    \|
0    /|
0   / |
0  /  |
1  \  |
0  /  |
0 /   |
1 \   |
0 /   |
1 \   |
1  \  |
1   \ |
1    \|
1     |\
1     | \
0     | /
1     | \
0     | /
0     |/
0    /|
1    \|
--------------------------------------------------------------------------------
````

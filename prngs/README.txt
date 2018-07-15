
# Compiling main prng.cpp 

__my_dir__/final> g++  -O2 -std=c++17 -o prng.o prng.cpp ./FlawedPath.cpp 



# Compiling testFlawedPath.cpp (output Dych path - based flawed path, also draws ASCII path)

__my_dir__/final> g++ -O2 -std=c++17 -o testFlawedPath ./testFlawedPath.cpp ./FlawedPath.cpp 
 

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!                                                                                                         !!!!!
!!!! COMPILE WITH FLAG  -O2 -> optimization -> generating path 4-5 times faster  !!                          !!!!!
!!!!                                                                                                         !!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

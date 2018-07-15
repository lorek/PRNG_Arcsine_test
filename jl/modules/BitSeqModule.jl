module BitSeqModule

export  BitSeq,
        getNrOfBits,
        getNrOfBytes,
        get,
        countOnes,
        countFracs,
        calcSlilVal,
        S_star,
        S_lil,
        stringToBitArray
     
# Literature:
# [1] Y. Wang, T. Nicol, On Statistical Based Testing of Pseudo Random
#     Sequences and Experiments wiht PHP and Debian OpenSSL, 2014
   
# Type for representing 0-1 sequences.
type BitSeq

    # data from which bits are extracted.
    data::Array{UInt32, 1}
    
    # physical length of the array with data
    dataL::Int64
    
    wordIndex::Int64
    
    bitIndex::Int64
    
    tempWord::UInt32
    
    function BitSeq(data::Array{UInt32, 1})
        this = new()
        this.dataL = length(data)
        this.data = data
        this.wordIndex = 1;
        this.bitIndex = 0;
        this.tempWord = data[1];
        return this
    end
end #type BitSeq

function reset(bits::BitSeq)
    bits.wordIndex = 1;
    bits.bitIndex = 0;
    bits.tempWord = bits.data[1];
end

function next(bits::BitSeq)
    res = bits.tempWord & 1
    if (bits.bitIndex < 31)
        bits.bitIndex = bits.bitIndex + 1
        bits.tempWord = bits.tempWord >> 1
    else
        bits.wordIndex = bits.wordIndex + 1
        if (bits.wordIndex > bits.dataL)
            reset(bits)
        else
            bits.bitIndex = 0;
            bits.tempWord = bits.data[bits.wordIndex];
        end
    end
    res
end

function getNrOfBytes(bits::BitSeq)
    bits.dataL * 4
end

function getNrOfBits(bits::BitSeq)
    bits.dataL * 32
end

# Get ith bit in given bit sequence. Bits are indexed, as in Julia arrays, from 1.
function get(bits::BitSeq, i::Int64)
    if (i < 1 || i > getNrOfBits(bits))
        error("Invalid bit index: $i")
    end
    i = i-1
    nr, b = divrem(i, 32)
    a = bits.data[nr+1]
    return (a & (1 << b)) == 0 ? 0 : 1
end

# Counts number of ones in a bitstring.
# @param bits sequence (array) of bits
# @return number of ones in that sequence
function countOnes(bits::BitSeq)
    n = getNrOfBits(bits)
    s = 0
    reset(bits)
    for i in 1:n
        if get(bits, i) == 1
            s = s + 1
        end
    end
    s
end

# Counts number of ones in a bitstring in given checkpoints.
# Checkpoints are lengths of prefixes in which ones should
# be counted.
# @param bits sequence (array) of bits
# @param checkPoints check points as an array of ascending integers
# @return number of of ones in each check points, as an array of
#         of integers of the same length as checkpoints
function countOnes(bits::BitSeq, checkPoints::Array{Int64, 1})
    #println("countOnes $(getNrOfBits(bits))")
    n = getNrOfBits(bits) :: Int64
    nrOfCheckPoints = length(checkPoints) :: Int64
    if (n < checkPoints[nrOfCheckPoints])
        error("countOnes: given bit sequence is to short.\nLast " *
              "checkpoint equals $(checkPoints[nrOfCheckPoints]) while sequence is of length $n")
    end
    ones = Array{Int64}(nrOfCheckPoints)
    nrOfOnes = 0 :: Int64
    prev_cp = 0
    reset(bits)
    for cp_ind in 1:nrOfCheckPoints
        cp = checkPoints[cp_ind]
        for i in (prev_cp+1):cp
            nrOfOnes = nrOfOnes + next(bits) 
        end
        ones[cp_ind] = nrOfOnes
        prev_cp = cp
    end
    ones
end 

function calcSlilVal(bits::BitSeq, checkPoints::Array{Int64, 1})
    nrOfCheckPoints = length(checkPoints) :: Int64
    res = Array{Float64}(nrOfCheckPoints)
    ones = countOnes(bits, checkPoints)
    for i in 1:nrOfCheckPoints
        res[i] = S_lil(checkPoints[i], ones[i])
    end
    return res
end

# Counts fraction of time "above the line" for each checkpoint.
# function countFracs(bits::BitSeq, checkPoints::Array{Int64, 1})
    # n = getNrOfBits(bits)
    # nrOfCheckPoints = length(checkPoints)
    # if (n < checkPoints[nrOfCheckPoints])
        # error("countFracs: given bit sequence is to short.\nLast " *
              # "checkpoint equals $(checkPoints[nrOfCheckPoints]) while sequence is of length $n")
    # end
    # fracs = Array{Float64}(nrOfCheckPoints)
    # prevBalance = 0
    # balance = 0
    # aboveTheLine = 0
    # cp = checkPoints[1]
    # cp_ind = 1
    # for i in 1:n
        # prevBalance = balance
        # balance = balance + (get(bits, i) == 1 ? 1 : -1)
        # if (prevBalance > 0 || balance > 0)
            # aboveTheLine = aboveTheLine + 1
        # end
        # if (i >= cp)
            # fracs[cp_ind] = aboveTheLine / cp
            # cp_ind = cp_ind + 1
            
            # if (cp_ind <= nrOfCheckPoints)
                # cp = checkPoints[cp_ind]
            # else
                # break
            # end
        # end
    # end
    # fracs
# end 

function countFracs(bits::BitSeq, checkPoints::Array{Int64, 1})
    #println("countOnes $(getNrOfBits(bits))")
    n = getNrOfBits(bits) :: Int64
    nrOfCheckPoints = length(checkPoints) :: Int64
    if (n < checkPoints[nrOfCheckPoints])
        error("countOnes: given bit sequence is to short.\nLast " *
              "checkpoint equals $(checkPoints[nrOfCheckPoints]) while sequence is of length $n")
    end
    fracs = Array{Float64}(nrOfCheckPoints)
    prevBalance = 0 :: Int64
    balance = 0 :: Int64
    aboveTheLine = 0 :: Int64
    prev_cp = 0 :: Int64
    reset(bits)
    for cp_ind in 1:nrOfCheckPoints
        cp = checkPoints[cp_ind]
        for i in (prev_cp+1):cp
            prevBalance = balance
            balance = balance - 1 + 2 * next(bits)
            aboveTheLine = aboveTheLine + (balance > 0 || prevBalance > 0 ? 1 : 0);
        end
        fracs[cp_ind] = aboveTheLine / cp
        prev_cp = cp
    end
    fracs
end 

# Calculates values S* as defined in [1].
# @param n length of a bitstring
# @param ones number of ones in a bitstring
# @return value S*
function S_star(n::Int64, ones::Int64)
    (2*ones - n) / sqrt(n)
end

# Calculates values S_lil as defined in [1].
# @param n length of a bitstring
# @param ones number of ones in a bitstring
# @return value S_lil
function S_lil(n::Int64, ones::Int64)
    S_star(n, ones) / sqrt(2 * log(log(n)))
end

# Checks whether given character is a white space.
# @param c character to check
# @return true if and only if given character is a white space.
function isWhite(c::Char)
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
end

function boolVal(c::Char)
    c != '0'
end

# Extracts 32-bit unsigned integer from a string of zeros and ones,
# starting from index beg.
# @param str string in which number is written.
# @param beg index of the first character of a substring which codes a number.
# @return number read.
function readUInt32(str::String, beg::Int64)
    #println("readUInt32 $beg")
    v::UInt32 = 0
    for i in 0:31
        #print(str[beg+i])
        if (boolVal(str[beg+i]))
            v = v + (1 << i)
        end
    end
    v
end

# Converts a string into 0-1 sequence of length.
# Trailing (and ONLY trailing) whitespaces are omitted.
# Every '0' is converted to 0, and every other character is converted to 1.
# @param str string to be converted
# @return bit sequence as a Bool Array
function stringToBitArray(str::String)
    l = length(str)
    while (l > 0 && isWhite(str[l]))
        l = l-1
    end
    if (mod(l, 32) != 0)
        error("length of the string must be multiplicity of 32")
    end
    n = div(l, 32)
    data = Array{UInt32}(n)
    for i in 0:(n-1)
        data[i+1] = readUInt32(str, i*32 + 1)
        #println("stringToBitArray $(data[i+1])")
    end
    BitSeq(data)
end
   
end # module

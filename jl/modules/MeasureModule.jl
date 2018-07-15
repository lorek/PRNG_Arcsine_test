module MeasureModule

export  Partition,
        Measure,
        printMeasure,
        makeHistogram,
        makePartition,
        makePartitionForLil,
        makePartitionForAsin,
        makeIdealLilMeasure,
        makeIdealAsinMeasure,
        makeIdealLilMeasures,
        makeIdealAsinMeasures,
        distTV,
        distSep,
        distHell,
        distRMS,
        chisqTest

using Distributions

# A partition is represented as an array of float pairs.
# It must have the following form:
# (-inf, a_1), [a_1, a_2), ..., [a_{n-1}, a_n), [a_n, inf)
# It's programmers responsibility to double check that
# an array of float pairs is really a correct partition.
const Partition = Array{Tuple{Float64, Float64}, 1}
                            
# Measure is a class which represents... well, measures.
# Note, however, that this class is limited only to
# measuring intervals from the partition given in a
# constructor.
type Measure
    # partition specifies the intervals whose measure is known
    part::Array{Tuple{Float64, Float64}, 1}
    # vals give measures of the above intervals
    vals::Array{Float64, 1}

end # type Measure

function myRound(x, n)
    round(x * 10^n) / 10^n
end

function printMeasure(m::Measure)
    n = length(m.part)
    for i in 1:n
        p = m.part[i]
        a = myRound(p[1], 3)
        b = myRound(p[2], 3)
        s = sign(m.vals[i])
        v = myRound(abs(m.vals[i]), 5)
        print("[$a, $b) -> $v\n")
    end
end

function makeHistogram(m::Measure)
    PyPlot.plt.hist(m.vals)
end


function makePartition(nrOfParts, start, finish)
    part = Array{Tuple{Float64, Float64}}(nrOfParts)
    breaks = collect(Iterators.flatten([-Inf, linspace(start, finish, nrOfParts-1), Inf]))
	for i in 1:nrOfParts
        part[i] = (breaks[i], breaks[i+1])
    end
    return part
end

function makePartitionForAsin(nrOfParts::Int)
    n = nrOfParts - 2
    step = 1.0 / n
    start = -step/2
    finish = 1 - step/2
    makePartition(nrOfParts, start, finish)
end


function makePartitionForLil(nrOfParts::Int)
    makePartition(nrOfParts, -1, 1)
end


# Function gives distribution of fraction of the time spend "above the line"
# for a truly random bit sequence.
# @param n length of a bit sequence, for now it is ignored and assumed that
#          the distribution is well approximated by arcsine distribution.
# @param a begining of the interval whose measure is calculated
# @param b end of the interval whose measure is calculated
# @return probability that fraction of the time "above the line" is between a and b.
 function arcSineMeasureIdeal(n::Int64, a::Float64, b::Float64)
    if a > b
        return 0.0
    end
    if (a < 0)
        l = 0.0
    elseif (a > 1)
        return 1.0
    else
        l = 2.0 / pi * asin(sqrt(a))
    end
    
    if (b > 1)
        r = 1.0
    elseif (b < 0)
        return 0.0
    else
        r = 2.0 / pi * asin(sqrt(b))
    end
    r - l
 end
 
# Returns a distribution implied by function arcSineMeasureU in a form
# of an object of type Measure.    
# @param n length of bit sequence
# @param part partition on which Measure object is defined
# @return Measure giving distribution of the fraction of the time "above the line".
function makeIdealAsinMeasure(n::Int64, part::Partition)
    vals = zeros(Float64, length(part))
    for i in 1:length(part)
        vals[i] = arcSineMeasureIdeal(n, part[i][1], part[i][2])
    end
    Measure(part, vals)
end

function makeIdealAsinMeasures(lengths::Array{Int64, 1}, part::Partition)
    n = length(lengths)
    ms = Array{Measure}(n)
    for i in 1:n
        ms[i] = makeIdealAsinMeasure(lengths[i], part)
    end
    ms
end


# Function corresponding to \mu^U_n from [1].
# @param n length of a bit sequence
# @param a begining of the interval whose measure is calculated
# @param b end of the interval whose measure is calculated
# @return Value \mu^U_n(a, b)
function lilMeasureIdeal(n::Int64, a::Float64, b::Float64)
    N = Normal()
    s = sqrt(2*log(log(n)))
    cdf(N, b*s) - cdf(N, a*s)
end

# Returns \mu^U_n from [1] in a form of an object of type Measure.    
# @param n length of bit sequence
# @param part partition on which Measure object is defined
# @return Measure object corresponding to \mu^U_n
function makeIdealLilMeasure(n::Int64, part::Partition)
    vals = zeros(Float64, length(part))
    for i in 1:length(part)
        vals[i] = lilMeasureIdeal(n, part[i][1], part[i][2])
    end
    Measure(part, vals)
end

function makeIdealLilMeasures(lengths::Array{Int64, 1}, part::Partition)
    n = length(lengths)
    ms = Array{Measure}(n)
    for i in 1:n
        ms[i] = makeIdealLilMeasure(lengths[i], part)
    end
    ms
end





# Calculates total variation distance between to probability measures.
# @param u the first probability measure
# @param v the other probability measure
# @return total variation distance between given measures
function distTV(u::Measure, v::Measure)
    if u.part != v.part
        throw("Measures operate on different partitions")
    end
    n = length(u.part)
    d = 0.0
    for i in 1:n
        x = u.vals[i] - v.vals[i]
        if x > 0
            d = d + x
        end
    end
    return d
end

function distHell(u::Measure, v::Measure)
    if u.part != v.part
        throw("Measures operate on different partitions")
    end
    n = length(u.part)
    s = 0.0
    for i in 1:n
        x = u.vals[i]
        y = v.vals[i]
        s = s + (sqrt(x) - sqrt(y))^2
    end
    return sqrt(s/2)
end


function distRMS(u::Measure, v::Measure)
    if u.part != v.part
        throw("Measures operate on different partitions")
    end
    n = length(u.part)
    s = 0.0
    for i in 1:n
        x = u.vals[i]
        y = v.vals[i]
        s = s + (x - y)^2
    end
    return sqrt(s/n)
end

function maximum(arr::Array{Float64, 1})
    m = -Inf
    n = length(arr)
    for i in 1:n
        m = arr[i] > m ? arr[i] : m
    end
    return m
end

function distSep(u::Measure, v::Measure)
    if u.part != v.part
        throw("Measures operate on different partitions")
    end
    isZero = function(x)
        return abs(x) < 0.0000001
    end
    fun = function(x, y)
        if (isZero(y))
            if (isZero(x)) return 0.0
            else return 1.0
            end
        else
            return 1.0 - x / y
        end
    end
    n = length(u.part)
    arr = Array{Float64}(n)
    for i in 1:n
        arr[i] = fun(u.vals[i], v.vals[i])
    end
    return maximum(arr)
end

function chisqTest(m::Int64, obs::Measure, exp::Measure)
    if obs.part != exp.part
        throw("Measures operate on different partitions")
    end
    t = 0
    l = length(obs.vals)
    df = l-1
    for i in 1:l
        if (exp.vals[i] < 0.0000001)
            df = df - 1
            continue
        end
        O = m * obs.vals[i]
        E = m * exp.vals[i]
        t = t + (O - E)^2 / E
    end
    println("chisqTest $t")
    chi = Chisq(df)
    return ccdf(chi, t)
end

end # module

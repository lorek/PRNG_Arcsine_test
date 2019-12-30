push!(LOAD_PATH, joinpath(dirname(Base.source_path()), "modules"))
# 
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/MeasureCreatorModule.jl")
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/MeasureModule.jl")
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/ResultPresenterModule.jl")
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/ResultReader.jl")
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/ResultSetModule.jl")
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/TestInvokerModule.jl")
# include("/home/peyo/repos/PRNG_Arcsine_test/jl/modules/BitSeqModule.jl")


using BitSeqModule
using MeasureModule
using TestInvokerModule
using ResultSetModule
using ResultPresenterModule


# sample usage 
# [PRNG_Arcsine_test]$ cat tmp_nolen.tmp | /home/peyo/progs/julia/bin/julia jl/Main.jl asin 4 tmp.txt w 1000 6


# cat aes_tmp1000_len26.txt | /home/peyo/progs/julia/bin/julia jl/Main.jl asin 4 1000 26 

function main()
    println("Entering main()")
    
    testType, nrOfCheckPoints, nrOfStrings, len, pathToFile, writeMode    = getCommandLineArgs()
    
    println("nrOfStrings")
    println(nrOfStrings)
    println("len")
    println(len)
    
    println("pathToFile")
    println(pathToFile)
    println("done")
    
    #nrOfStrings, length = getDataSize()
    nrOfStrings =  parse(Int64, nrOfStrings)
    len2 = parse(Int64, len)
    length=2^len2
    
    if(nrOfStrings==0)
        nrOfStrings, length = getDataSize()
    end
    
    
    println("Julia: nrOfStrings = $nrOfStrings\nJulia: length = $length");
    
    
    loglen = convert(Int64, floor(log2(length)))
    println("Julia: typeof(loglen) = $(typeof(loglen))");
    
    checkPoints, checkPointsLabels = makeCheckPoints(nrOfCheckPoints, loglen)
    println("Julia: checkPoints = $checkPoints\nlabels = $checkPointsLabels");
    
 
    
    invoker = TestInvoker(getTestFunction(testType), checkPoints, checkPointsLabels)
    file = open(pathToFile, writeMode)
    setFileHandle(invoker, file, writeMode == "w")
    readAllBits(invoker, nrOfStrings, length)    
    close(file)
    
    part = makePartition(testType, 42)
    println("All bits read...")
    idealMeasures = getIdealMeasures(testType, part, checkPoints)
    pres = ResultPresenter(invoker.results, idealMeasures)
    init(pres, part)
    present(pres)
end

function getCommandLineArgs()
  println("length(ARGS)  =")
    println(length(ARGS))
    
    if (length(ARGS) < 1 || length(ARGS) > 6)
        error("Usage: julia Main.jl [lil|asin] [nrOfCheckPoints] [nrOfStrings] [length] [pathToFile] [writeMode] ")
    end
    
    if (length(ARGS) == 1)
        return ARGS[1], 0, "0", "0", "tmp.txt", "w"
    end
    
    nrOfCheckPoints = parse(ARGS[2])
    if (typeof(nrOfCheckPoints) != Int || nrOfCheckPoints < 0)
        error("Usage: julia Main.jl [lil|asin] {nrOfCheckPoints}")
    end
    
    if (length(ARGS) == 2)
        println("Asdfsadf")
        return ARGS[1], nrOfCheckPoints,     "0", "0", "tmp.txt", "w"
    end
    
    if (length(ARGS) == 4)
        return ARGS[1], nrOfCheckPoints, ARGS[3],ARGS[4],    "tmp.txt", "w"
    end
    
  
 
    return ARGS[1], nrOfCheckPoints, ARGS[3], ARGS[4], ARGS[5], ARGS[6]
end


function getDataSize()
    nrOfStrings = read(STDIN, Int64)
    length = read(STDIN, Int64)
    return nrOfStrings, length
end

function makeCheckPoints(nrOfCheckPoints, loglen)
    checkPoints = zeros(Int64, nrOfCheckPoints+1)
    checkPointsLabels = Array{String}(nrOfCheckPoints+1)
    for i in 0:nrOfCheckPoints
        checkPoints[i+1] = 2 ^ (loglen - nrOfCheckPoints + i)
        checkPointsLabels[i+1] = "2^$(loglen - nrOfCheckPoints + i)"
    end
    return checkPoints, checkPointsLabels
end

function getTestFunction(testType)
    if (testType == "lil")
        return calcSlilVal
    elseif (testType == "asin")
        return countFracs
    else
        error("Unknown test type: $testType")
    end
end

function readAllBits(invoker, nrOfStrings, length)
    println("Entering readAllBits()")
    
    counter = 0
    for i in 1:nrOfStrings
        data = read(STDIN, UInt32, div(length,32))
        #println("Julia: readAllBits")
        #for j in 1:div(length,32)
        #    @printf "%X" data[j]
        #end
        #println()
        bits = BitSeq(data)
        #println("readAllBits $i")
        addSeq(invoker, bits)
        counter = counter + 1
        if (counter % 10 == 0)
            println("Julia: read $counter")
        end
    end    
end

function makePartition(testType, nrOfParts)
    if (testType == "lil")
        return makePartitionForLil(nrOfParts)
    elseif (testType == "asin")
        return makePartitionForAsin(nrOfParts)
    else
        error("Unknown test type: $testType")
    end
end
    
function getIdealMeasures(testType::String, part::Partition, lengths::Array{Int64, 1})
    if (testType == "lil")
        return makeIdealLilMeasures(lengths, part)
    elseif (testType == "asin")
        return makeIdealAsinMeasures(lengths, part)
    else
        error("Unknown test type: $testType")
    end
end

function printSummary(measure, ideal)   
    println("Ideal:")
    printMeasure(ideal)
    println("\n\n\Empirical:")
    printMeasure(measure)
    
    tv = distTV(ideal, measure)
    hell = distHell(ideal, measure)
    rms = distRMS(ideal, measure)
    println("\n\ntotal variation = $tv")
    println("hellinger = $hell")
    println("root mean square = $rms")
end


#Profile.init()
#@profile begin
main()
#end
#Profile.print()

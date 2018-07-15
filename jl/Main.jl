push!(LOAD_PATH, joinpath(dirname(Base.source_path()), "modules"))

using BitSeqModule
using MeasureModule
using TestInvokerModule
using ResultSetModule
using ResultPresenterModule

function main()
    println("Entering main()")
    
    testType, nrOfCheckPoints, pathToFile, writeMode = getCommandLineArgs()
    
    nrOfStrings, length = getDataSize()
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
    if (length(ARGS) < 1 || length(ARGS) > 4)
        error("Usage: julia Main.jl [lil|asin] [nrOfCheckPoints] [pathToFile] [writeMode]")
    end
    
    if (length(ARGS) == 1)
        return ARGS[1], 0, "tmp.txt", "w"
    end
    
    nrOfCheckPoints = parse(ARGS[2])
    if (typeof(nrOfCheckPoints) != Int || nrOfCheckPoints < 0)
        error("Usage: julia Main.jl [lil|asin] {nrOfCheckPoints}")
    end
    
    if (length(ARGS) == 2)
        return ARGS[1], nrOfCheckPoints, "tmp.txt", "w"
    end
    
    if (length(ARGS) == 3)
        return ARGS[1], nrOfCheckPoints, ARGS[3], "w"
    end
    
    return ARGS[1], nrOfCheckPoints, ARGS[3], ARGS[4]
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

using ResultSetModule
using ResultPresenterModule
using MeasureModule

function main()
    println("Entering main()")
    
    testType, loglen, pathToFile = getCommandLineArgs()
    
    pair = readdlm(pathToFile, ';'; header=true)
    rset = ResultSet(vec(pair[2]))
    fillResultSet(rset, pair[1])
    
    part = makePartition(testType, 42)
    ideal = getIdealMeasures(testType, part, 2 .^ [(loglen-getNrOfColumns(rset)+1):loglen])
    pres = ResultPresenter(rset, ideal)
    setDisplay(pres, " & ", "\\\\ \\hline\n", 4, 2)
    init(pres, part)
    present(pres)
end

function getCommandLineArgs()
    if (length(ARGS) < 2 || length(ARGS) > 3)
        error("Usage: julia ResultReader.jl [lil|asin] [log2 of length] [pathToFile]")
    end
    
    loglen = parse(ARGS[2])
    if (typeof(loglen) != Int || loglen < 0)
        error("Usage: julia Main.jl [lil|asin] [log2 of length] [pathToFile]")
    end
    
    if (length(ARGS) == 2)
        return ARGS[1], loglen, "tmp.txt"
    end
    
    return ARGS[1], loglen, ARGS[3]
end

function fillResultSet(rset :: ResultSet, table :: Array{Float64, 2})
    rows, cols = size(table)
    for r in 1:rows
        addResult(rset, vec(table[r,:]))
    end
    return
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

main()

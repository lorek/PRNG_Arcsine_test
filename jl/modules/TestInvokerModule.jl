module TestInvokerModule

export TestInvoker,
       addSeq,
       setFileHandle,
       resetFileHandle

using ResultSetModule
using BitSeqModule

type TestInvoker
    results :: ResultSet
    
    testingFunction :: Function
    
    checkPoints::Array{Int64, 1}
    
    checkPointsLabels::Array{String, 1}
    
    writeToFile::Bool
    
    fileHandle::IOStream
    
    function TestInvoker(testingFunction::Function, checkPoints::Array{Int64, 1}, checkPointsLabels::Array{String, 1})
        this = new()
        this.testingFunction = testingFunction
        this.checkPoints = checkPoints
        this.checkPointsLabels = checkPointsLabels
        this.results = ResultSet(checkPointsLabels)
        this.writeToFile = false
        return this
    end
end #type TestInvoker

function setFileHandle(ti::TestInvoker, fileHandle::IOStream, makeHeader::Bool)
    ti.fileHandle = fileHandle
    ti.writeToFile = true
    if makeHeader
        write(fileHandle, join(ti.checkPointsLabels, "; "))
        write(fileHandle, "\n")
        flush(fileHandle)
    end
end

function resetFileHandle(ti::TestInvoker)
    ti.writeToFile = false
end

function addSeq(ti::TestInvoker, bits::BitSeq)
    res = ti.testingFunction(bits, ti.checkPoints)
    addResult(ti.results, res)
    if (ti.writeToFile)
        write(ti.fileHandle, join(res, "; "))
        write(ti.fileHandle, "\n")
        flush(ti.fileHandle)
    end
end

end #module


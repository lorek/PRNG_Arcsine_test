module ResultSetModule

export ResultSet,
       addResult,
       getNrOfRows,
       getNrOfColumns,
       getHeader,
       getColumn,
       getRowHeader
       
type ResultSet
    header::Vector{String}
    
    nrOfColumns::Int64
    
    nrOfRows::Int64
    
    results::Vector{Vector{Float64}}
    
    rowHeaders::Vector{String}

    function ResultSet(header)
        this = new()
        this.header = header
        this.results = []
        this.rowHeaders = []
        this.nrOfColumns = length(header)
        this.nrOfRows = 0
        return this
    end
end #type ResultSet

function addResult(rset::ResultSet, row::Vector{Float64}, rowHeader :: String)
    if (length(row) != rset.nrOfColumns)
        error("addResult :: Number of check points in ResultSet: $(rset.nrOfColumns), length of result vector: $(length(row))")
    end
    push!(rset.results, row)
    push!(rset.rowHeaders, rowHeader)
    rset.nrOfRows = rset.nrOfRows + 1
end

function addResult(rset::ResultSet, row::Vector{Float64})
    addResult(rset, row, "")
end

function getNrOfRows(resset::ResultSet)
    return resset.nrOfRows
end

function getNrOfColumns(resset::ResultSet)
    return resset.nrOfColumns
end


function getHeader(resset::ResultSet, nr::Int64)
    return resset.header[nr]
end


function getColumn(resset::ResultSet, column::Int64)
    #println("getColumn")
    n = resset.nrOfRows
    res = Array{Float64}(n)
    for i in 1:n
        res[i] = resset.results[i][column]
    end
    return res
end

function getRowHeader(resset::ResultSet, row::Int64)
    return resset.rowHeaders[row]
end

end #module


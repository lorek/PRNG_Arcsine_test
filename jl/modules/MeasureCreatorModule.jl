module MeasureCreatorModule

export MeasureCreator,
       makeMeasure
        
using MeasureModule
using ResultSetModule

# MeasureCreator serves to instantiate a probability measure implied
# by bit sequences obtained from a PRG. For each bit sequence the value
# of some statistic is calculated, and stored in object of type ResultSet.
# This gives an empirical distribution of the data (i.e. measure on the real line)
type MeasureCreator
    # partition on which created measure is defined
    part::Partition
    
    # number of intervals in a partition (i.e. length of part)
    nrOfParts::Int64
    
    # Object for storing values of the statistic
    results::ResultSet
    
    # Represents empirical function of type
    # (check point index, interval index) -> number of corresponding S_frac values falling to specified interval
    # This array is updated as new sequences appear. It is later used to create a measure.
    buckets::Array{Int64, 2}
    
    
    function MeasureCreator(part_::Partition, results::ResultSet)
        this = new()
        this.part = copy(part_)
        this.nrOfParts = length(this.part)
        this.results = results
        this.buckets = Array{Int64}(0, 0)
        return this
    end
end

function initBuckets(mc::MeasureCreator)
    #println("initBuckets")
    cols = getNrOfColumns(mc.results)
    mc.buckets = Array{Int64}(cols, mc.nrOfParts)
    for cp in 1:cols
        for p in 1:mc.nrOfParts
            mc.buckets[cp, p] = 0
        end
    end            
    return
end
    
function addToBucket(mc::MeasureCreator, cp_ind::Int64, val::Float64)
    for i in 1:mc.nrOfParts
        p = mc.part[i]
        if (p[1] <= val < p[2])
            mc.buckets[cp_ind, i] = mc.buckets[cp_ind, i] + 1
            return
        end
    end
    error("Appriopriate interval not found!!")
end

function fillBuckets(mc::MeasureCreator)
    #println("fillBuckets")
    n = getNrOfRows(mc.results)
    for cp in 1:getNrOfColumns(mc.results)
        vals = getColumn(mc.results, cp)
        for i in 1:n
            #addToBucket(mc, cp, vals[i])
            addToBucket(mc, cp, round(vals[i], 4))
        end
    end      
end

function makeMeasure(mc::MeasureCreator, cp_ind::Int64)
    println("Making measure $cp_ind...")
    if (length(mc.buckets) == 0)
        initBuckets(mc)
        fillBuckets(mc)
    end
    n = mc.nrOfParts
    nrOfSeqs = getNrOfRows(mc.results)
    vals = zeros(Float64, n)
    for i in 1:n
        vals[i] = mc.buckets[cp_ind, i] / nrOfSeqs
    end
    Measure(mc.part, vals)
end
   


 
 end #module
 
module GridFlow

class ::Object; def FloatOrSymbol(x) Float(x) rescue x.intern end end
end # module GridFlow

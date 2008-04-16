module GridFlow
class << self; attr_accessor :data_path end
@data_path=[GridFlow::DIR+"/images"]
def GridFlow.find_file s
	s=s.to_s
	if s==File.basename(s) then
		dir = GridFlow.data_path.find {|x| File.exist? "#{x}/#{s}" }
		if dir then "#{dir}/#{s}" else s end
	else s end
end

class ::Object; def FloatOrSymbol(x) Float(x) rescue x.intern end end
end # module GridFlow

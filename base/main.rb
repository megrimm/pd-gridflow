module GridFlow
class << self
	attr_accessor :data_path
end
@data_path=[GridFlow::DIR+"/images"]
class ::Object; def FloatOrSymbol(x) Float(x) rescue x.intern end end
class FObject
	def initialize2; end
	def initialize(*) end
end
def GridFlow.find_file s
	s=s.to_s
	if s==File.basename(s) then
		dir = GridFlow.data_path.find {|x| File.exist? "#{x}/#{s}" }
		if dir then "#{dir}/#{s}" else s end
	else s end
end
end # module GridFlow

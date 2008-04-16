module GridFlow
def self.post(s,*a) post_string(sprintf(s,*a)) end
class << self
	attr_accessor :data_path
	attr_reader :fobjects
end
@data_path=[GridFlow::DIR+"/images"]
class ::Object; def FloatOrSymbol(x) Float(x) rescue x.intern end end
class FObject
	def self.inspect; @pdname or super; end
	def self.gfattrs; @gfattrs={} if not defined? @gfattrs; @gfattrs end
	def self.gfattr(s,*v)
		s=s.intern if String===s
		gfattrs[s]=v
		attr_accessor s
		module_eval "def _0_#{s}(o) self.#{s}=o end"
	end
	def initialize2; end
	def initialize(*) end
	def _0_get(s=nil)
		s=s.intern if String===s
		if s then
			if respond_to? s then send_out noutlets-1,s,__send__(s) else ___get s end
		else
			self.class.gfattrs.each_key{|k| _0_get k }
		end
	end
end
def GridFlow.find_file s
	s=s.to_s
	if s==File.basename(s) then
		dir = GridFlow.data_path.find {|x| File.exist? "#{x}/#{s}" }
		if dir then "#{dir}/#{s}" else s end
	else s end
end
end # module GridFlow
def GridFlow.load_user_config
	user_config_file = ENV["HOME"] + "/.gridflow_startup"
	begin
		load user_config_file if File.exist? user_config_file
	rescue Exception => e
		GridFlow.post "#{e.class}: #{e}:\n" + e.backtrace.join("\n")
		GridFlow.post "while loading ~/.gridflow_startup"
	end
end

class JMaxFileHandler
	Size = [4,1,2,4]
	Packer = ["N","c","n","N"]
	OpTable = [
		[0, :return],
		[1, :push, :int],
		[2, :push, :float],
		[3, :push, :symbol],
		[5, :set, :int],
		[6, :set, :float],
		[7, :set, :symbol],
		[9, :pop_args, :int],
		[10, :push_obj, :int],
		[11, :mv_obj, :int],
		[12, :pop_objs, :int],
		[13, :make_obj, :int],
		[14, :put_prop, :symbol],
		[16, :obj_mess, :int, :int, :int],
		[18, :push_obj_table, :int],
		[19, :pop_obj_table],
		[20, :connect],
		[21, :make_top_obj, :int],
	]
	OpTableById = {}
	OpTableByName = {}
	OpTable.each {|entry|
		id,name,arg = entry
		OpTableById[id] = entry
		OpTableByName[name] = entry
	}
end

class JObject
	attr_reader :properties
	attr_accessor :parent_patcher
	attr_reader :init_messages
	attr_reader :connections
	def initialize(*args)
		@args = args
		@properties = {}
		@init_messages = []
	end
	def send_in(inlet,*args)
		@init_messages << [inlet,*args]
	end
	def connect(inlet,target,outlet)
	end
	def subobjects
		@subobjects={} if not defined? @subobjects
		@subobjects
	end
end

class JMaxFileReader < JMaxFileHandler
	def initialize(f,factory=JObject)
		@f = f
		@symbols = []
		@estack = []
		@ostack = []
		@tstack = []
		@factory = factory
	end
	def parse
		magic, code_size, n_symbols = @f.read(12).unpack("a4NN")
		case magic
		when "bMax"; #ok
		when "bMa2"; raise "bMa2 format (jMax 4) is not supported yet"
		else raise "not a jMax file"
		end
		@code = @f.read code_size
		@symbols = @f.read.split(/\0/).map {|x| x.intern }
		@index = 0
		while @index < @code.size
			read_opcode
		end
		raise "@estack.size!=0" if @estack.size!=0
		raise "@ostack.size!=1" if @ostack.size!=1
		raise "@tstack.size!=0" if @tstack.size!=0
		@ostack
	end
	def read_opcode
		#puts "#{@index} of #{@code.size}"
		op = @code[@index]; @index+=1
		op1,op2 = op&0x3f,op>>6
		entry = OpTableById[op1]
		if not entry
			puts "skipping unknown opcode #{op1},#{op2}"
			return
		end
		args = []
		(entry.length-2).times {|i|
			args << (case entry[2+i]
			when :int
				n=Size[op2]; v=@code[@index,n].unpack(Packer[op2])[0]
				if v[8*n-1]!=0 then ~(~v&0x7fffffff) else v end
			when :float
				n=4; @code[@index,4].unpack("f")[0]
			when :symbol
				n=Size[op2]; @symbols[@code[@index,n].unpack(Packer[op2])[0]]
			when nil
			end)
			@index+=n
		}
		text = sprintf "%05d: %2d,%1d: %s", @index, op1, op2, entry[1]
		text << "(" << args.map{|x|x.inspect}.join(",") << ")"
		#puts text
		send entry[1], *args
	end
	def push x; @estack << x end
	def set x
		if @estack.size>0 then @estack[-1]=x else @estack << x end
	end
	def put_prop x; @ostack[-1].properties[x] = @estack[-1] end
	def make_obj x
		patcher = @ostack[-1] if @ostack.size>0
		baby = @factory.new(*(@estack[-x,x]))
		@ostack << baby
		@ostack[-1].parent_patcher = patcher
		patcher.subobjects[baby]=true if patcher
	end
	alias :make_top_obj :make_obj
	def pop_args x; @estack[-x,x]=[] end
	def push_obj_table x; @tstack<<[] end
	def mv_obj x; @tstack[-1][x]=@ostack[-1] end
	def pop_objs x; @ostack[-x,x]=[] end
	def obj_mess i,s,n; @ostack[-1].send_in i,s,*(@estack[-n,n]) end
	def push_obj x; @ostack<<@tstack[-1][x] end
	def connect; @ostack[-1].connect @estack[-1],@ostack[-2],@estack[-2] end
	def pop_obj_table; @tstack.pop end
	def return; end
end

if $0 == __FILE__
	jff = JMaxFileReader.new File.open("samples/fire.jmax")
	jff.parse
end

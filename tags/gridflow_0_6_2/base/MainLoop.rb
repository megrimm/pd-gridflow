# $Id$
=begin

        MetaRuby
        file: Main Loop

        Copyright (c) 2001,2002 by Mathieu Bouchard
        Licensed under the same license as Ruby.

=end

class MainLoop

=begin
a Message is something that can be called (activated).
it includes receiver, selector, and parameters (and block).

# the former name "Action" is now reserved for something else.
=end
class Message
	attr_accessor :receiver,:selector,:params,:block
	def initialize(receiver, selector,*params,&block)
		self.receiver = receiver
		self.selector = selector
		self.params   = params
		self.block    = block
	end
	def call
		receiver.send(selector,*params,&block)
	end
end

=begin
a MessageQueue is an array of Messages that have to be called in the order
they are received. you can #add a Message at the end of the queue, or you
can #consume the oldest Message of the Queue.
=end

class MessageQueue
	def initialize
		@queue = []
	end
	def add(x=nil)
		x ||= Message.new(Proc.new,:call)
		@queue << x
	end
	def consume
		@queue.shift.call
	end
	def empty?; @queue.empty?; end
	def length; @queue.length; end
end

=begin
a TimerQueue is a timeline of Messages that must be triggered only
after a certain point in time. note: Those Messages are now
transferred to the end of the main message queue, to allow timers
to reschedule themselves without hogging resources.
=end

class TimerQueue
	TimerEntry = Struct.new(:time,:message)
	def initialize(message_queue)
		@queue = []
		@message_queue = message_queue
	end

	# schedule a message or a block for execution at a specified time
	# time is a Time object.
	# delay is a Numeric object. in seconds.
	# message is a Message object.
	# block is an Object that responds to #call

	def at_time_call(a_time, a_message, &a_block)
		a_message ||= Message.new(a_block,:call)

		entry = TimerEntry.new(a_time, a_message)
		i = 0
		@queue.each {|e| e.time < entry.time or break; i+=1 }
		@queue[i,0] = [entry]
	end

	# schedule a message or a block for execution at a certain delay
	# after the current time

	def after(a_delay,a_message=nil,&block)
		at_time_call(Time.now+a_delay,a_message,&block)
	end

	def delay_no_next; 1.0; end

	def delay_until_next
		return delay_no_next if @queue.length == 0
		delay = @queue[0].time - Time.new
		delay = 0 if delay < 0
		delay
	end

	def consume
		# @queue.shift.message.call if delay_until_next == 0
		@message_queue.add @queue.shift.message if delay_until_next == 0
	end
end

=begin
a Unix Stream is what is commonly known as a file handle even though it
may be referring to a non-file like a pipe or socket. 

a Stream Observer is an object that is to be notified of certain events of
a file handle, usually incoming data.

a StreamMap is an object that keeps track of the state of various Unix
Streams and what are their associated observers.

add_stream(stream,observer,type_mask)
remove_stream(stream)

adding an already added stream automatically removes the previous registration.

stream is an IO object.

type_mask is a set of the following flags:
	StreamMap::READ
	StreamMap::WRITE
	StreamMap::EXCEPT

observer is an Object that responds to some of the following (depending on the
event-type mask):
	#ready_to_read(stream)
	#ready_to_write(stream)
	#stream_exception(stream)

=end

class StreamMap
	StreamEntry = Struct.new(:stream,:stream_observer,:type_mask)
	READ = 4
	WRITE = 2
	EXCEPT = 1

	def initialize
		@streams = {}
	end

	# why do i use #to_s again?
	def add_stream(stream,observer,type_mask)
		@streams[stream.to_s] =
			StreamEntry.new(stream,observer,type_mask)
	end

	def remove_stream(stream)
		@streams.remove(stream.to_s)
	end

	def streams_by_mask(mask)
		@streams.values.
			find_all {|a| a.type_mask & mask }.
			map      {|a| a.stream }
	end

	# like IO.select, but using this object's lists, and
	# returning always a three element array.
	def select(time=nil)
		IO.select(
			streams_by_mask(READ),
			streams_by_mask(WRITE),
			streams_by_mask(EXCEPT),
			time) || [[]]*3
	end

	def make_messages(time=nil)
		selectors = [:ready_to_read,:ready_to_write,:stream_exception]
		flags =     [          READ,          WRITE,        EXCEPT   ]

		lists = select(time)
		#p lists
		messages = []
		(0..2).each {|i|
			lists[i].each{|s|
				se = @streams[s.to_s]
				observer = se.stream_observer
				messages << Message.new(observer,selectors[i],s) \
					if se.type_mask & flags[i] > 0
			}
		}
		p messages if messages.length>0
		messages
	end
end

attr_reader :messages
attr_reader :timers
attr_reader :streams

def initialize
	@messages = MessageQueue.new
	@timers  = TimerQueue.new(@messages)
	@streams = StreamMap.new
end

def one(delay=nil)
	while @timers.delay_until_next == 0
		@timers.consume
	end
	@messages.length.times { @messages.consume }
	@streams.make_messages(delay || @timers.delay_until_next).each {|m|
		m.call
		# @messages.add m
	}
	# while not @messages.empty?; @messages.consume; end
	@messages.length.times { @messages.consume }
end

def loop
	while true; one; end
end

#--------------------------------#
end # of class MainLoop

#todo:
# algorithms for message priorities:
#  * in order (current)
#  * fully random
#  * with explicit priority level
#  * priority based on percentage of time use, frequency of use, etc.
#      (trying to be fair to different program activities)
# put examples of use that shows off all features at once
#    (tests/MainLoop.rb or samples/something...)
#  * add semi-threads (or whatever i may call them)
#  * add support for ruby-async signals.
#  * add mailboxes and such

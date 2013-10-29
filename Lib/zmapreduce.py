"""libmapreduce wrapper for ZeroVM

ZMapReduce module is used for running mapreduce jobs using zerovm-zwift infrastructure.
All you need is to define key functions like map, reduce etc, and create map/reduce
node instance initialized with given functions.

Functions description

map function takes 4 positional arguments: data, size, last_chunk, buffer.
'data' is input raw data. It's presented as memoryview python object to
avoid copying overhead. libmapreduce reads input files, splits them and
passes data read to map function.
'size' is input data size, in bytes.
'last_chunk' is integer flag indicating end of file. If it is set,
then it's the last map function call.
'buffer' is output key-value pairs buffer. Any parsed data should be placed here.
map function should return parsed bytes count.
libmapreduce expects map function to parse input raw data somehow and place
appropriate key-value records in output buffer. Function is required.

reduce function takes 1 positional argument: buffer.
'buffer' is input key-value buffer that should be 'reduced'
You could iterate over key-value pairs and process them. Function is required.

combine function takes 2 positonal arguments: input, output.
Both arguments are key-value buffers, input buffer is filled with sorted data,
output buffer is empty.
libmapreduce uses this function to 'reduce data in-place'. You could
think of it as built-in reduce node in a map node. After map function parsed data,
libmapreduce tries to combine it in order to 'compress' data before sending it to
reduce nodes. Function is optional.

comparator function is used to compare hashes of keys in key-value pairs.
It takes two positional arguments: h1, h2. Arguments are binary strings.
It should return -1, 0, 1.
Builtin implementation already performs memcmp of hashes. So, it is not
necessary to provide user-defined function in most cases. Function is optional.

Attributes description

Node ID. Integer. Represents node in libmapreduce infrastructure. Should be unique
inside mapreduce job. Required.

Hash size. Integer. Determines size in bytes of a hash field in key-value pairs.
Required.

MRItem size. Integer, Determines size of record (key-value pair) size in a buffer.
This attribute depends on hash size, and possibly could be not public. Typically,
it is 18 bytes (size of internal structure) plus hash size. Required.

Data flow

First of all, each map node in libmapreduce framework reads input file, chunk by chunk.
Chunk size is set in ENVIRONMENT. Typically it equals to 100MB. Each chunk is passed
to map function which fills output buffer.

Next step is sorting output buffer by hash with qsort (happens inside libmapreduce)
and passing sorted output buffer to combine function if present. Combine function
'reduces' output buffer, decreasing data count.

Last step is transferring data over network to reduce nodes. Reducers perform further
data reduce. Reduce nodes combine data on every reception.

_zmapreduce module types

'Record'. Represents key-value pair. Essentially, it's some part of underlying
buffer. Has 3 attributes:
'key'. Key. Could be 'str' or memoryview object. Arbitrary size.
'value'. Value. Could be 'str' or memoryview object. Arbitrary size.
'hash'. Hash of a key. Should be 'str'. Size equals Node.hash_size.
Used in sorting.

'Buffer'. List of a records. Iterable (for record in buffer).
Has __getitem__ method defined (buffer[N]). Has two appending methods:
'append()'. No arguments. Returns newly created record which is used
to populate data.
'append(key, value. hash)'. 3 positional arguments (they are same as record
attributes). Created for performance reasons.

Usage example:

def reduce(buffer):
    # iterating over buffer
	for record in buffer:
	    # no actual reduce here, just output
		print "[%s] %s = %s" % (
			record.hash, record.key, record.value)
	return 0

def map(data, size, last_chunk, buffer):
    # convert raw bytes to string to operate on
	string = data.tobytes()
	# for every word in line
	for word in string.split():
	    # creating new record = key-value pair
		r = buffer.append()
		# set record.key as word
		r.key = word
		# set record value as 1
		r.value = "1"
        # calculate hash somehow
		r.hash = hash_function(word)
	# return bytes count processed
	return len(string)

r = Reducer(1)
r.map_fn = map
r.reduce_fn = reduce
r.hash_size = 10
r.mritem_size = 28

r.start()


"""

import _zmapreduce

class Node():
    """Base class for node types. Represents a node.

    Usage example:
    node_id = 1
    n = Node(node_id)

    n.map_fn = map
    n.reduce_fn = reduce
    n.combine_fn = combine
    n.comparator_fn = compare
    n.hash_size = 8
    n.mritem_size = 28

    n.start()
    """
    def __init__(self, node_id):
        """Node ctor.

        node_id sets current node ID used in libmapreduce infrastructure."""

        self.node_id = node_id
        self.map_fn = None
        self.reduce_fn = None
        self.combine_fn = None
        self.comparator_fn = None
        self.hash_size = 0
        self.mritem_size = 0

    def start(self):
        """Initialize node and run it.

        First, basic init procedure is performed. Define _do_init() in subclasses
        to perform various init actions.
        Next, C callbacks is set to appropriate python functions.
        At last, node is run. Function returns after node has finished its work.
        """
        self._do_init()
        _zmapreduce._module_set_callbacks(self.map_fn,
                                          self.reduce_fn,
                                          self.combine_fn,
                                          self.comparator_fn,
                                          self.mritem_size,
                                          self.hash_size)
        return self._do_run()

    def _do_init(self):
        """Initialize current node."""
        raise NotImplementedError

    def _do_run(self):
        """Run current node."""
        raise NotImplementedError

class Mapper(Node):
    """Mapper node.

    It initialize read-write channels with others mapper nodes, write channels with
    reduce nodes in _do_init().
    """
    def _do_init(self):
        """Initialize map node.

        It calls C function from libmapreduce to setup rw map channels and w reduce channels."""
        _zmapreduce._module_setup_map_channels(self.node_id)
    def _do_run(self):
        """Run map node.

        Calls appropriate C function from libmapreduce.
        """
        return _zmapreduce._module_run_map_main()

class Reducer(Node):
    """Reduce node.

    It initialize read channels with mapper nodes in _do_init().
    """
    def _do_init(self):
        """Initialize reduce node.

        It calls C function from libmapreduce to setup r map channels.
        """
        _zmapreduce._module_setup_reduce_channels(self.node_id)
    def _do_run(self):
        """Run reduce node.

        Calls appropriate C function from libmapreduce.
        """
        return _zmapreduce._module_run_reduce_main()
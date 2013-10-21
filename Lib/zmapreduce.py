"""
ZMapReduce module. libmapreduce wrapper for ZeroVM.
"""

import _zmapreduce

class Node():
    """
    Base class for node types. Represent a node.
    """
    def __init__(self, node_id):
        self.node_id = node_id
        self.map_fn = None
        self.reduce_fn = None
        self.combine_fn = None
        self.comparator_fn = None
        self.hash_size = 0
        self.mritem_size = 0

    def start(self):
        self._do_init()
        _zmapreduce._module_set_callbacks(self.map_fn,
                                          self.reduce_fn,
                                          self.combine_fn,
                                          self.comparator_fn,
                                          self.mritem_size,
                                          self.hash_size)
        return self._do_run()

    def _do_init(self):
        raise NotImplementedError

    def _do_run(self):
        raise NotImplementedError

class Mapper(Node):
    def _do_init(self):
        _zmapreduce._module_setup_map_channels(self.node_id)
    def _do_run(self):
        return _zmapreduce._module_run_map_main()

class Reducer(Node):
    def _do_init(self):
        _zmapreduce._module_setup_reduce_channels(self.node_id)
    def _do_run(self):
        return _zmapreduce._module_run_reduce_main()
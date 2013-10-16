#include "zmapreducemodule.h"

#include "mapreduce/elastic_mr_item.h"
#include "mapreduce/map_reduce_lib.h"
#include "networking/eachtoother_comm.h"
#include "networking/channels_conf.h"
#include "networking/channels_conf_reader.h"

static struct ChannelsConfigInterface chan_if;
static struct MapReduceUserIf mr_if;
static char map_node_type_text[] = "map";
static char red_node_type_text[] = "red";

static PyObject* PyComparatorHashFunc = 0;
static PyObject* PyReduceFunc = 0;
static PyObject* PyMapFunc = 0;
static PyObject* PyCombineFunc = 0;

static int
ComparatorHash(const void *h1, const void *h2){
  PyObject* args = Py_BuildValue("(s#, s#)", h1, mr_if.data.hash_size,
                                 h2, mr_if.data.hash_size);
  PyObject* val = PyObject_CallObject(PyComparatorHashFunc, args);
  Py_DECREF(args);

  int ret = PyInt_AsLong(val);
  Py_DECREF(val);
  return ret;
}
static int FallBackComparatorHash(const void *h1, const void *h2)
{
  return memcmp(h1,h2, mr_if.data.hash_size);
}

static int
ComparatorElasticBufItemByHashQSort(const void *p1, const void *p2){
  return ComparatorHash( &((ElasticBufItemData*)p1)->key_hash,
                         &((ElasticBufItemData*)p2)->key_hash );
}
static int
FallBackComparatorElasticBufItemByHashQSort(const void *p1, const void *p2){
  return FallBackComparatorHash( &((ElasticBufItemData*)p1)->key_hash,
                         &((ElasticBufItemData*)p2)->key_hash );
}


static char*
PrintableHash( char* str, const uint8_t* hash, int size){
    memcpy(str, hash, mr_if.data.hash_size);
    return str;
}


/*******************************************************************************
 * User map for MAP REDUCE
 * Readed data will organized as MrItem with a
 * 10bytes key, 90bytes data and 10bytes hash exactly equal to the key
 * @param size size of data must be multiple on 100,
 * set MAP_CHUNK_SIZE env variable properly*/
static int Map(const char *data,
               size_t size,
               int last_chunk,
               Buffer *map_buffer ){

  assert(PyMapFunc);

  // create arguments for Reduce function call
  // MapReduceBuffer to fill
  PyObject* MapReduceBuffer = MapReduceBuffer_FromBuffer(map_buffer);
  // buffer and memoryview to scan for raw data
  Py_buffer* buffer = (Py_buffer*) malloc(sizeof(Py_buffer));
  PyBuffer_FillInfo(buffer, NULL, (void*)data, size, 0, PyBUF_CONTIG);
  PyObject* mv = PyMemoryView_FromBuffer(buffer);
  // size and last_chunk vars
  PyObject* args = Py_BuildValue("(OniO)",
                                 mv,
                                 size,
                                 last_chunk,
                                 MapReduceBuffer);

  fprintf(stderr, "before  addr=%p buffer count = %d, size = %d data=%p pybuffer=%p\n",
          map_buffer,
          map_buffer->header.count,
          map_buffer->header.buf_size,
          data,
          buffer->buf);

  // call python reduce routine
  PyObject* val = PyObject_CallObject(PyMapFunc, args);
  Py_DECREF(args);
  Py_DECREF(MapReduceBuffer);
  free(buffer);
  Py_DECREF(mv);

  if (!val) {
    // error happened
    return -1;
  }

  fprintf(stderr, "after buffer count = %d, size = %d \n", map_buffer->header.count,
          map_buffer->header.buf_size);

  int ret = PyInt_AsLong(val);
  Py_DECREF(val);
  return ret;

}

static int Reduce( const Buffer *reduced_buffer ){
  assert(PyReduceFunc);

  // create arguments for Reduce function call
  PyObject* MapReduceBuffer = MapReduceBuffer_FromBuffer((Buffer*)reduced_buffer);
  PyObject* args = Py_BuildValue("(O)", MapReduceBuffer);
  Py_DECREF(MapReduceBuffer);
  // call python reduce routine
  PyObject* val = PyObject_CallObject(PyReduceFunc, args);
  Py_DECREF(args);

  if (!val) {
    // error happened
    return -1;
  }

  int ret = PyInt_AsLong(val);
  Py_DECREF(val);

  // do cleanup after pyreduce
  int i;
  for ( i=0; i < reduced_buffer->header.count; i++ ){
    ElasticBufItemData* elasticdata = (ElasticBufItemData*)BufferItemPointer( reduced_buffer, i );
    TRY_FREE_MRITEM_DATA(elasticdata);
  }
  return ret;

}



static int Combine( const Buffer *map_buffer,
                    Buffer *reduce_buffer ){

  assert (PyCombineFunc);
  PyObject* MapReduceBufferOutput = MapReduceBuffer_FromBuffer((Buffer*)reduce_buffer);
  PyObject* MapReduceBufferInput = MapReduceBuffer_FromBuffer((Buffer*)map_buffer);
  PyObject* args = Py_BuildValue("(OO)",
                                 MapReduceBufferInput,
                                 MapReduceBufferOutput);
  // call python reduce routine
  PyObject* val = PyObject_CallObject(PyCombineFunc, args);
  Py_DECREF(MapReduceBufferInput);
  Py_DECREF(MapReduceBufferOutput);
  Py_DECREF(args);

  if (!val)
  {
    PyErr_Print();
    exit(-1);
  }

  Py_DECREF(val);
  return 0;
}

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  ElasticBufItemData* data;
} MapReduceRecord;

static PyObject* record_get_key(MapReduceRecord* self, void* closure)
{
  PyObject* key = PyString_FromStringAndSize((const char*)self->data->key_data.addr,
                                             self->data->key_data.size);
  return key;
}
static int record_set_key(MapReduceRecord* self, PyObject* val, void* closure)
{
  if (PyMemoryView_Check(val))
  {
    Py_buffer* buffer = PyMemoryView_GET_BUFFER(val);

    const char* c = (char*)(buffer->buf);
    /*set key data: pointer + key data length. using pointer to existing data*/
    self->data->key_data.addr = (uintptr_t)c;
    self->data->key_data.size = buffer->len;
    self->data->own_key = EDataNotOwned;
  }
  else if (PyString_Check(val))
  {
    PyStringObject* string = (PyStringObject*)val;

    const char* c = (char*)(string->ob_sval);
    char* tmp = malloc(PyString_Size((PyObject*)string));
    memcpy(tmp, c, PyString_Size((PyObject*)string));
    /*set key data: pointer + key data length. using pointer to existing data*/
    self->data->key_data.addr = (intptr_t)tmp;
    self->data->key_data.size = PyString_Size((PyObject*)string);
    self->data->own_key = EDataOwned;
    // TODO: do not know waht to do with this
    //    memcpy(&self->data->key_hash, c, PyString_Size(string));
  }
  else
    return -1;

  // decref val?
  return 0;
}
static PyObject* record_get_value(MapReduceRecord* self, void* closure)
{
  PyObject* value = PyString_FromStringAndSize((const char*)self->data->value.addr,
                                               self->data->value.size);
  return value;
}
static int record_set_value(MapReduceRecord* self, PyObject* val, void* closure)
{
  if (PyMemoryView_Check(val))
  {
    Py_buffer* buffer = PyMemoryView_GET_BUFFER(val);

    const char* c = (char*)(buffer->buf);
    /*save as hash record key without changes to variable length struct member*/
    /*interpret elasticdata->value.addr as 4bytes pointer and save data as binary*/
    self->data->value.size = buffer->len;
    self->data->value.addr = (uintptr_t)c;
    self->data->own_value = EDataNotOwned;
  }
  else if (PyString_Check(val))
  {
    PyStringObject* string = (PyStringObject*)val;

    const char* c = (char*)(string->ob_sval);
    char* tmp = malloc(PyString_Size((PyObject*)string));
    memcpy(tmp, c, PyString_Size((PyObject*)string));
    /*set key data: pointer + key data length. using pointer to existing data*/
    self->data->value.addr = (intptr_t)tmp;
    self->data->value.size = PyString_Size((PyObject*)string);
    self->data->own_key = EDataOwned;
  }
  else
    return -1;

  // decref val?
  return 0;
}

static PyObject* record_get_hash(MapReduceRecord* self, void* closure)
{
  // TODO: hash size! where to get, where to store
  PyObject* key = PyString_FromStringAndSize((char*)&self->data->key_hash,
                                             mr_if.data.hash_size);
  return key;
}
static int record_set_hash(MapReduceRecord* self, PyObject* val, void* closure)
{
  if (PyMemoryView_Check(val))
  {
    Py_buffer* buffer = PyMemoryView_GET_BUFFER(val);

    const char* c = (char*)(buffer->buf);
    /*set key data: pointer + key data length. using pointer to existing data*/
    memcpy( &self->data->key_hash, c, mr_if.data.hash_size );
  }
  else if (PyString_Check(val))
  {
    const char* c = PyString_AsString(val);
    memcpy(&self->data->key_hash, c, mr_if.data.hash_size);
  }
  else
    return -1;

  // decref val?
  return 0;
}

static int
record_init(MapReduceRecord *self, PyObject *args, PyObject *kwds)
{
  self->data = 0;
  return 0;
}
static PyGetSetDef record_getseters[] = {
  {"key",
   (getter)record_get_key, (setter)record_set_key,
   "record key",
   NULL},
  {"value",
   (getter)record_get_value, (setter)record_set_value,
   "record value",
   NULL},
  {"hash",
   (getter)record_get_hash, (setter)record_set_hash,
   "hash value",
   NULL},
  {NULL}  /* Sentinel */
};
static PyTypeObject MapReduceRecordType = {
  PyObject_HEAD_INIT(NULL)
  0,				/* ob_size        */
  "_zmapreduce.Record",		/* tp_name        */
  sizeof(MapReduceRecord),		/* tp_basicsize   */
  0,				/* tp_itemsize    */
  0,				/* tp_dealloc     */
  0,				/* tp_print       */
  0,				/* tp_getattr     */
  0,				/* tp_setattr     */
  0,				/* tp_compare     */
  0,				/* tp_repr        */
  0,				/* tp_as_number   */
  0,				/* tp_as_sequence */
  0,				/* tp_as_mapping  */
  0,				/* tp_hash        */
  0,				/* tp_call        */
  0,				/* tp_str         */
  0,				/* tp_getattro    */
  0,				/* tp_setattro    */
  0,				/* tp_as_buffer   */
  Py_TPFLAGS_DEFAULT,		/* tp_flags       */
  "Simple objects are simple.",	/* tp_doc         */
  0,		               /* tp_traverse */
  0,		               /* tp_clear */
  0,		               /* tp_richcompare */
  0,		               /* tp_weaklistoffset */
  0,		               /* tp_iter */
  0,		               /* tp_iternext */
  0,             /* tp_methods */
  0,             /* tp_members */
  record_getseters,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)record_init,      /* tp_init */
  0,                         /* tp_alloc */
  0,                 /* tp_new */
};

PyObject* MapReduceRecord_FromElasticBufItemData(ElasticBufItemData* item)
{
  MapReduceRecord* record = PyObject_NEW(MapReduceRecord, &MapReduceRecordType);
  record->data = item;

  return (PyObject*)record;
}

// ----------------------------------------------------------------------------

static PyObject* buffer_append(MapReduceBuffer* self)
{
  ElasticBufItemData* elasticdata =
      (ElasticBufItemData*)BufferItemPointer(self->data,
                                             AddBufferItemVirtually(self->data));

  PyObject* record = MapReduceRecord_FromElasticBufItemData(elasticdata);
  return record;
}

static PyObject* buffer_append_record(MapReduceBuffer* self, PyObject* args)
{
  PyObject* key = 0;
  PyObject* value = 0;
  PyObject* hash = 0;

  if (!PyArg_ParseTuple(args, "OOO", &key, &value, &hash))
    return NULL;

  ElasticBufItemData* elasticdata =
      (ElasticBufItemData*)BufferItemPointer(self->data,
                                             AddBufferItemVirtually(self->data));

  MapReduceRecord r;
  r.data = elasticdata;
  record_set_hash(&r, hash, NULL);
  record_set_key(&r, key, NULL);
  record_set_value(&r, value, NULL);

  Py_RETURN_NONE;
}

static PyObject* buffer_clear(MapReduceBuffer* self)
{
  self->data->header.count = 0;

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* buffer_GetItem(MapReduceBuffer *self, Py_ssize_t i)
{
  if (self->data->header.count <= i)
  {
    PyErr_SetString(PyExc_IndexError, "get_item: index is out of buffer bounds!");
    return NULL;
  }

  ElasticBufItemData* elasticdata =
      (ElasticBufItemData*)BufferItemPointer(self->data,
                                             i);
  PyObject* record = MapReduceRecord_FromElasticBufItemData(elasticdata);
  return record;
}

Py_ssize_t buffer_Size(MapReduceBuffer* self)
{
  return self->data->header.count;
}

static PyMethodDef MapReduceBufferMethods[] = {
  {"append", (PyCFunction)buffer_append, METH_NOARGS,
   "append record"
  },
  {"append_record", (PyCFunction)buffer_append_record, METH_VARARGS,
   "append record"
  },
  {"clear", (PyCFunction)buffer_clear, METH_NOARGS,
   "clear buffer, resetting it header count to 0"
  },
  {NULL}  /* Sentinel */
};

static PySequenceMethods MapReduceSequenceMethods = {
  (lenfunc)buffer_Size,
  0,
  0,
  (ssizeargfunc)buffer_GetItem,
  0,
  0,
  0,
  0,
  /* Added in release 2.0 */
  0,
  0,
};

PyObject* buffer_iter(PyObject *self)
{
  MapReduceBuffer *buffer = (MapReduceBuffer *)self;
  buffer->index = 0;
  Py_INCREF(self);
  return self;
}
PyObject* buffer_iternext(PyObject *self)
{
  MapReduceBuffer *buffer = (MapReduceBuffer *)self;
  if (buffer->index < buffer->data->header.count)
  {
    PyObject* record = MapReduceRecord_FromElasticBufItemData(
                         (ElasticBufItemData*)BufferItemPointer(buffer->data, buffer->index));
    // Py_DECREF(record); ?
    PyObject *tmp = Py_BuildValue("O", record);
    buffer->index++;
    Py_DECREF(record);
    return tmp;
  } else {
    /* Raising of standard StopIteration exception with empty value. */
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
}
PyObject* buffer_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
  PyErr_SetString(PyExc_Exception, "__new__: Creating buffer objects is not allowed!");
  return NULL;
}

PyObject* buffer_alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  PyErr_SetString(PyExc_Exception, "__alloc__: Creating buffer objects is not allowed!");
  return NULL;
}
int buffer_init(PyObject *self, PyObject *args, PyObject *kwds)
{
  PyErr_SetString(PyExc_Exception, "__init__: Creating buffer objects is not allowed!");
  return -1;
}


static PyTypeObject MapReduceBufferType = {
  PyObject_HEAD_INIT(NULL)
  0,				/* ob_size        */
  "_zmapreduce.Buffer",		/* tp_name        */
  sizeof(MapReduceBuffer),		/* tp_basicsize   */
  0,				/* tp_itemsize    */
  0,				/* tp_dealloc     */
  0,				/* tp_print       */
  0,				/* tp_getattr     */
  0,				/* tp_setattr     */
  0,				/* tp_compare     */
  0,				/* tp_repr        */
  0,				/* tp_as_number   */
  &MapReduceSequenceMethods,				/* tp_as_sequence */
  0,				/* tp_as_mapping  */
  0,				/* tp_hash        */
  0,				/* tp_call        */
  0,				/* tp_str         */
  0,				/* tp_getattro    */
  0,				/* tp_setattro    */
  0,				/* tp_as_buffer   */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,		/* tp_flags       */
  "Buffer object",	/* tp_doc         */
  0,		               /* tp_traverse */
  0,		               /* tp_clear */
  0,		               /* tp_richcompare */
  0,		               /* tp_weaklistoffset */
  buffer_iter,		               /* tp_iter */
  buffer_iternext,		               /* tp_iternext */
  MapReduceBufferMethods,             /* tp_methods */
  0,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  buffer_init,      /* tp_init */
  buffer_alloc,                         /* tp_alloc */
  buffer_new,                 /* tp_new */
};

static PyObject* module_setup_map_channels(PyObject* self, PyObject* args)
{
  int ownnodeid = -1;
  if (!PyArg_ParseTuple(args, "i", &ownnodeid))
    return NULL;

  SetupChannelsConfigInterface( &chan_if, ownnodeid, EMapNode );

  /***********************************************************************
   * setup network configuration of cluster: */
  int res = -1;
  /* add manifest channels to read from another map nodes */
  res = AddAllChannelsRelatedToNodeTypeFromDir( &chan_if,
                                                IN_DIR,
                                                EChannelModeRead,
                                                EMapNode,
                                                map_node_type_text );
  assert( res == 0 );
  /* add manifest channels to read from another reduce nodes */
  res = AddAllChannelsRelatedToNodeTypeFromDir( &chan_if,
                                                IN_DIR,
                                                EChannelModeRead,
                                                EReduceNode,
                                                red_node_type_text );
  assert( res == 0 );
  /* add manifest channels to write into another map nodes */
  res = AddAllChannelsRelatedToNodeTypeFromDir( &chan_if,
                                                OUT_DIR,
                                                EChannelModeWrite,
                                                EMapNode,
                                                map_node_type_text );
  assert( res == 0 );
  /* add manifest channels to write into another reduce nodes */
  res = AddAllChannelsRelatedToNodeTypeFromDir( &chan_if,
                                                OUT_DIR,
                                                EChannelModeWrite,
                                                EReduceNode,
                                                red_node_type_text );
  assert( res == 0 );
  /*add input channel into config*/
  res = chan_if.AddChannel( &chan_if,
                            EInputOutputNode,
                            1, /*nodeid*/
                            0, /* STDIN associated input mode*/
                            EChannelModeRead ) != NULL? 0: -1;

  /*add fake channel into config, to access map nodes count later via
    channel interface at runtime; if we are not add this then mapreduce
    will fail because map nodes count will unavailable*/
  res = chan_if.AddChannel( &chan_if,
                            EMapNode,
                            ownnodeid,
                            -1,
                            EChannelModeRead ) != NULL? 0: -1;
  assert( res == 0 );

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject* module_setup_reduce_channels(PyObject* self, PyObject* args)
{
  int ownnodeid = -1;
  if (!PyArg_ParseTuple(args, "i", &ownnodeid))
    return NULL;
  SetupChannelsConfigInterface( &chan_if, ownnodeid, EReduceNode );

  /***********************************************************************
   * setup network configuration of cluster: */
  int res = -1;
  /* add manifest channels to read from map nodes */
  res = AddAllChannelsRelatedToNodeTypeFromDir( &chan_if,
                                                IN_DIR,
                                                EChannelModeRead,
                                                EMapNode,
                                                map_node_type_text );
  assert( res == 0 );
  /*associate stdout with results output of reduce node*/
  res = chan_if.AddChannel( &chan_if,
                            EInputOutputNode,
                            EReduceNode,
                            1, // STDOUT
                            EChannelModeWrite ) != NULL? 0: -1;

  assert( res == 0 );

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject* module_setup_callbacks(PyObject* self, PyObject* args)
{
  memset(&mr_if, '\0', sizeof(struct MapReduceUserIf));

  if (!PyArg_ParseTuple(args, "OOOOii",
                        &PyMapFunc,
                        &PyReduceFunc,
                        &PyCombineFunc,
                        &PyComparatorHashFunc,
                        &mr_if.data.mr_item_size,
                        &mr_if.data.hash_size))
    return NULL;

  if (PyComparatorHashFunc && PyCallable_Check(PyComparatorHashFunc))
  {
    mr_if.ComparatorHash = ComparatorHash;
    mr_if.ComparatorMrItem = ComparatorElasticBufItemByHashQSort;
  }
  else
  {
    mr_if.ComparatorHash = FallBackComparatorHash;
    mr_if.ComparatorMrItem = FallBackComparatorElasticBufItemByHashQSort;
  }

  if (PyReduceFunc && PyCallable_Check(PyReduceFunc))
    mr_if.Reduce = Reduce;

  if (PyMapFunc && PyCallable_Check(PyMapFunc))
    mr_if.Map = Map;

  if (PyCombineFunc && PyCallable_Check(PyCombineFunc))
    mr_if.Combine = Combine;

  // TODO: value addr is data
  mr_if.data.value_addr_is_data = 0;
  mr_if.DebugHashAsString = PrintableHash;


  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject* module_run_map_main(PyObject* self)
{
  PyObject* ret = PyLong_FromLong(MapNodeMain(&mr_if, &chan_if));

  CloseChannels(&chan_if);
  return ret;
}
static PyObject* module_run_reduce_main(PyObject* self)
{
  PyObject* ret = PyLong_FromLong(ReduceNodeMain(&mr_if, &chan_if));

  CloseChannels(&chan_if);
  return ret;
}


static PyMethodDef MapReduceModuleMethods[] = {
  {"_module_set_callbacks", (PyCFunction)module_setup_callbacks, METH_VARARGS, ""},
  {"_module_setup_map_channels", (PyCFunction)module_setup_map_channels, METH_VARARGS, ""},
  {"_module_setup_reduce_channels", (PyCFunction)module_setup_reduce_channels, METH_VARARGS, ""},
  {"_module_run_map_main", (PyCFunction)module_run_map_main, METH_NOARGS, ""},
  {"_module_run_reduce_main", (PyCFunction)module_run_reduce_main, METH_NOARGS, ""},
  {NULL}  /* Sentinel */
};


PyMODINIT_FUNC
init_zmapreduce(void)
{
  PyObject* m;

  MapReduceRecordType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&MapReduceRecordType) < 0)
    return;

  MapReduceBufferType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&MapReduceBufferType) < 0)
    return;

  m = Py_InitModule3("_zmapreduce", MapReduceModuleMethods,
                     "Example module that creates an extension type.");
  if (m == NULL)
    return;

  Py_INCREF(&MapReduceRecordType);
  PyModule_AddObject(m, "Record", (PyObject *)&MapReduceRecordType);
  Py_INCREF(&MapReduceBufferType);
  PyModule_AddObject(m, "Buffer", (PyObject *)&MapReduceBufferType);
}

PyObject* MapReduceBuffer_FromBuffer(Buffer* buffer)
{
  MapReduceBuffer* buf = PyObject_NEW(MapReduceBuffer, &MapReduceBufferType);
  buf->data = buffer;
  buf->index = 0;

  return (PyObject*)buf;
}

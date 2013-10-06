#include "ztestmodule.h"

static PyObject* record_get_key(MapReduceRecord* self, void* closure)
{
  PyObject* key = PyString_FromStringAndSize((const char*)self->data->key_data.addr,
                                             self->data->key_data.size);
  Py_IncRef(key);
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
    memcpy( &self->data->key_hash, c, buffer->len );
  }
  else if (PyString_Check(val))
  {
    PyStringObject* string = (PyStringObject*)val;

    const char* c = (char*)(string->ob_sval);
    char* tmp = malloc(PyString_Size(string));
    memcpy(tmp, c, (PyString_Size(string));
    /*set key data: pointer + key data length. using pointer to existing data*/
    self->data->key_data.addr = tmp;
    self->data->key_data.size = PyString_Size(string);
    self->data->own_key = EDataOwned;
    // TODO: do not know waht to do with this
//    memcpy(&self->data->key_hash, c, PyString_Size(string));
  }
  else
    return -1;

  return 0;
}
static PyObject* record_get_value(MapReduceRecord* self, void* closure)
{
  PyObject* value = PyString_FromStringAndSize((const char*)self->data->value.addr,
                                               self->data->value.size);
  Py_IncRef(value);
  return value;
}
static int record_set_value(MapReduceRecord* self, PyObject* val, void* closure)
{
  if (!PyMemoryView_Check(val))
    return -1;

  Py_buffer* buffer = PyMemoryView_GET_BUFFER(val);

  const char* c = (char*)(buffer->buf);
  /*save as hash record key without changes to variable length struct member*/
  /*interpret elasticdata->value.addr as 4bytes pointer and save data as binary*/
  self->data->value.size = buffer->len;
  self->data->value.addr = (uintptr_t)c;
  self->data->own_value = EDataNotOwned;

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
  {NULL}  /* Sentinel */
};
static PyTypeObject MapReduceRecordType = {
  PyObject_HEAD_INIT(NULL)
  0,				/* ob_size        */
  "ztest.Record",		/* tp_name        */
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
// ----------------------------------------------------------------------------
static PyObject* buffer_get_value(MapReduceBuffer* self, PyObject* args)
{
  int index = 0;
  if (!PyArg_ParseTuple(args, "i", &index))
    return NULL;

  ElasticBufItemData* elasticdata = (ElasticBufItemData*)BufferItemPointer( self->data, index );
  PyObject* value = PyString_FromStringAndSize((const char*)elasticdata->value.addr,
                                               elasticdata->value.size);

  Py_INCREF(value);
  return value;
}
static PyObject* buffer_get_key(MapReduceBuffer* self, PyObject* args)
{
  int index = 0;
  if (!PyArg_ParseTuple(args, "i", &index))
    return NULL;

  ElasticBufItemData* elasticdata = (ElasticBufItemData*)BufferItemPointer( self->data, index );
  PyObject* value = PyString_FromStringAndSize((const char*)elasticdata->key_data.addr,
                                               elasticdata->key_data.size);

  Py_INCREF(value);
  return value;
}
static PyObject* buffer_get_count(MapReduceBuffer* self)
{
  PyObject* value = PyInt_FromLong(self->data->header.count);
  Py_INCREF(value);
  return value;
}
static PyObject* buffer_get_item_size(MapReduceBuffer* self)
{
  PyObject* value = PyInt_FromLong(self->data->header.item_size);
  Py_INCREF(value);
  return value;
}
static PyObject* buffer_append(MapReduceBuffer* self)
{
//  Py_buffer* buffer = (Py_buffer*) malloc(sizeof(Py_buffer));
//  int record_size = 0;
//  int position = 0;
//  if (!PyArg_ParseTuple(args, "w*ii", buffer, &position, &record_size)) {
//    return NULL;
//  }

  ElasticBufItemData* elasticdata =
      (ElasticBufItemData*)BufferItemPointer(self->data,
                                             AddBufferItemVirtually(self->data));
//  const int HASH_SIZE = 10;
//  const int TERRASORT_RECORD_SIZE = record_size;

//  const char* c = &((char*)(buffer->buf))[position];
//  /*set key data: pointer + key data length. using pointer to existing data*/
//  elasticdata->key_data.addr = (uintptr_t)c;
//  elasticdata->key_data.size = HASH_SIZE;
//  elasticdata->own_key = EDataNotOwned;
//  /*save as hash record key without changes to variable length struct member*/
//  memcpy( &elasticdata->key_hash, c, HASH_SIZE );
//  /*interpret elasticdata->value.addr as 4bytes pointer and save data as binary*/
//  elasticdata->value.size = TERRASORT_RECORD_SIZE - HASH_SIZE;
//  elasticdata->value.addr = (uintptr_t)c + HASH_SIZE;
//  elasticdata->own_value = EDataNotOwned;

  PyObject* record = MapReduceRecord_FromElasticBufItemData(elasticdata);
  fprintf(stderr, "buffer %p out addr=%p addr=%p\n",
          self->data,
          (void*)elasticdata->key_data.addr,
          (void*)elasticdata->value.addr);
  return record;
}

static PyMethodDef MapReduceBufferMethods[] = {
  {"value", (PyCFunction)buffer_get_value, METH_VARARGS,
   "Return the value"
  },
  {"key", (PyCFunction)buffer_get_key, METH_VARARGS,
   "Return the key"
  },
  {"count", (PyCFunction)buffer_get_count, METH_NOARGS,
   "Return the count"
  },
  {"item_size", (PyCFunction)buffer_get_item_size, METH_NOARGS,
   "Return the item size"
  },
  {"append", (PyCFunction)buffer_append, METH_NOARGS,
   "append record"
  },
  {NULL}  /* Sentinel */
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
    PyObject *tmp = Py_BuildValue("O", record);
    buffer->index++;
    return tmp;
  } else {
    /* Raising of standard StopIteration exception with empty value. */
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
}


static PyTypeObject MapReduceBufferType = {
  PyObject_HEAD_INIT(NULL)
  0,				/* ob_size        */
  "ztest.Buffer",		/* tp_name        */
  sizeof(MapReduceBuffer),		/* tp_basicsize   */
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
  0,      /* tp_init */
  0,                         /* tp_alloc */
  0,                 /* tp_new */
};


PyMODINIT_FUNC
initztest(void)
{
  PyObject* m;

  MapReduceRecordType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&MapReduceRecordType) < 0)
    return;

  MapReduceBufferType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&MapReduceBufferType) < 0)
    return;

  m = Py_InitModule3("ztest", NULL,
                     "Example module that creates an extension type.");
  if (m == NULL)
    return;

  Py_INCREF(&MapReduceRecordType);
  PyModule_AddObject(m, "Record", (PyObject *)&MapReduceRecordType);
  Py_INCREF(&MapReduceBufferType);
  PyModule_AddObject(m, "Buffer", (PyObject *)&MapReduceBufferType);
}


PyObject* MapReduceRecord_FromElasticBufItemData(ElasticBufItemData* item)
{
  MapReduceRecord* record = PyObject_NEW(MapReduceRecord, &MapReduceRecordType);
  record->data = item;
  Py_INCREF(record);

  return (PyObject*)record;
}


PyObject* MapReduceBuffer_FromBuffer(Buffer* buffer)
{
  MapReduceBuffer* buf = PyObject_NEW(MapReduceBuffer, &MapReduceBufferType);
  buf->data = buffer;
  buf->index = 0;
  Py_INCREF(buf);

  return (PyObject*)buf;
}

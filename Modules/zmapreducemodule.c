#include "zmapreducemodule.h"

#include "mapreduce/elastic_mr_item.h"

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
    memcpy( &self->data->key_hash, c, buffer->len );
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
  "zmapreduce.Record",		/* tp_name        */
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
//    Py_DECREF(record);
    return tmp;
  } else {
    Py_DECREF(self);
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
  "zmapreduce.Buffer",		/* tp_name        */
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


PyMODINIT_FUNC
initzmapreduce(void)
{
  PyObject* m;

  MapReduceRecordType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&MapReduceRecordType) < 0)
    return;

  MapReduceBufferType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&MapReduceBufferType) < 0)
    return;

  m = Py_InitModule3("zmapreduce", NULL,
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

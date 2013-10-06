#ifndef ZTESTMODULE_H
#define ZTESTMODULE_H

#include <Python.h>

#include "mapreduce/elastic_mr_item.h"
#include "mapreduce/buffer.h"

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  ElasticBufItemData* data;
} MapReduceRecord;

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  Buffer* data;
} MapReduceBuffer;

PyObject* MapReduceRecord_FromElasticBufItemData(ElasticBufItemData* item);
PyObject* MapReduceBuffer_FromBuffer(Buffer* buffer);

#endif // ZTESTMODULE_H

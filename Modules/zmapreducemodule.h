#ifndef ZMAPREDUCEMODULE_H
#define ZMAPREDUCEMODULE_H

#include <Python.h>

#include "mapreduce/buffer.h"

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  Buffer* data;
  int index;
} MapReduceBuffer;

PyObject* MapReduceBuffer_FromBuffer(Buffer* buffer);

#endif // ZMAPREDUCEMODULE_H

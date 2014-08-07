#ifndef COREMAP_H
#define COREMAP_H

#include "thread.h"

class Thread;

class CoreMapEntry {
 public:
  int vpn;
  bool use;
  bool dirty;
  Thread *thread;
};
#endif

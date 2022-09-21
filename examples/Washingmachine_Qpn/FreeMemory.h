// Copied from:
// https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory

#ifndef __FREEMEMORY_H
#define __FREEMEMORY_H

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return 1 + (&top - __brkval);
#else  // __arm__
  return 1 + (__brkval ? &top - __brkval : &top - __malloc_heap_start);
#endif  // __arm__
}

#endif

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#if defined(__GNUG__)
#define _msize  malloc_usable_size
#endif

#include <execinfo.h>
#include <iostream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <boost/thread/recursive_mutex.hpp>
#include <fstream>
#include <map>
#include <vector>

#include "leak_detector.h"
#include <signal.h>


void LeakDumpHandle(int sig) {
    std::cout << "Leak dumping with signal(" << sig << ")" << std::endl;
    leakd::MemoryLeakDetector::inst().Dump();
}

void InitLeakSignal() {
    leakd::MemoryLeakDetector::inst().Dump();
    int leak_detect_sig = 16;
    if (SIG_ERR == signal(leak_detect_sig, LeakDumpHandle)) {
        std::cerr << "Cannot catch signal:" << leak_detect_sig << std::endl;
    } else {
        std::cout << "Init leak dump signal:" << leak_detect_sig << std::endl;
    }
}

extern "C" {
void *__real_malloc(size_t sz);

void *__wrap_malloc(size_t sz) {
    void *p = __real_malloc(sz);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, sz, cs);
    return p;
}

void __real_free(void *p);

void __wrap_free(void *p) {
    leakd::MemoryLeakDetector::inst().Erase(p);
    __real_free(p);
    return;
}

char *__real_strdup(char* s);

char *__wrap_strdup(char* s) {
    char *p = __real_strdup(s);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, malloc_usable_size(p), cs);
    return p;
}

void *__real_realloc(void *p, unsigned int newsize);

void *__wrap_realloc(void *p, unsigned int newsize) {
    leakd::MemoryLeakDetector::inst().Erase(p);

    void *q = __real_realloc(p, newsize);

    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(q, newsize, cs);

    return q;
}

void *__real_calloc(size_t n, size_t size);

void *__wrap_calloc(size_t n, size_t size) {
    void *p = __real_calloc(n, size);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, n*size, cs);
    return p;
}

void __real_cfree(void* p);

void __wrap_cfree(void* p) {
    leakd::MemoryLeakDetector::inst().Erase(p);
    __real_cfree(p);
    return;
}

void* __real_memalign(size_t align, size_t s);

void* __wrap_memalign(size_t align, size_t s) {
    void* p = __real_memalign(align, s);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, s, cs);
    return p;
}

void* __real_valloc(size_t size);

void* __wrap_valloc(size_t size) {
    void* p = __real_valloc(size);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, size, cs);
    return p;
}


void* __real_pvalloc(size_t size);

void* __wrap_pvalloc(size_t size) {
    void *p = __real_pvalloc(size);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, size, cs);
    return p;
}
}

void* operator new(size_t sz) {
    void *p = __real_malloc(sz);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, sz, cs);
    return p;
}

void* operator new[] (size_t sz) {
    void *p = __real_malloc(sz);
    leakd::CallStack* cs = leakd::CallStackLookupTable::inst().Lookup();
    leakd::MemoryLeakDetector::inst().Insert(p, sz, cs);
    return p;
}


void operator delete(void* p) {
    leakd::MemoryLeakDetector::inst().Erase(p);
    __real_free(p);
    return;
}

void operator delete[](void* p) {
    leakd::MemoryLeakDetector::inst().Erase(p);
    __real_free(p);
    return;
}


#include <stdio.h>
#include <stdlib.h>
#include <tr1/functional>
#include <iostream>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <set>
#include <map>
#include <stdint.h>
#include <pthread.h>
#include <execinfo.h>
#include <sstream>
#include <string>


extern "C" {
    void *__real_malloc(size_t s);
    void __real_free(void* p);
}

static size_t s_leak_detector_size = 0;

template <class T>
class real_allocator;

template<>
class real_allocator<void> {
public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
    typedef void        value_type;

    template <class U> struct rebind {
        typedef real_allocator<U> other;
    };
};

template <class _T1, class _T2>
inline bool operator==(const real_allocator<_T1>&, const real_allocator<_T2>&) {
    return true;
}

template <class _T1, class _T2>
inline bool operator!=(const real_allocator<_T1>&, const real_allocator<_T2>&) {
    return false;
}

template <class T>
class real_allocator {
public:
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef T*         pointer;
    typedef const T*   const_pointer;
    typedef T&         reference;
    typedef const T&   const_reference;
    typedef T          value_type;

    template <class U> struct rebind {
        typedef real_allocator<U> other;
    };


    real_allocator() {}

    real_allocator(const real_allocator& a) {}

    template <class U> real_allocator(const real_allocator<U>& a) {}

    ~real_allocator() {}

    inline pointer address(reference x) const __attribute__((always_inline)) {
        return &x;
    }

    inline const_pointer address(const_reference x) const __attribute__((always_inline)) {
        return &x;
    }


    inline T* allocate(size_type n, const void* = 0) __attribute__((always_inline)) {
        __sync_add_and_fetch(&s_leak_detector_size, n*sizeof(T));
        return static_cast<T*>(__real_malloc(n*sizeof(T)));
    }

    // p is not permitted to be a null pointer.
    inline void deallocate(pointer p, size_type n = 0) __attribute__((always_inline)) {
        n = (0 == n) ? 1 : n;
        __sync_sub_and_fetch(&s_leak_detector_size, n*sizeof(T));
        __real_free(p);
    }

    inline size_type max_size() const __attribute__((always_inline)) {
        return size_type(-1) / sizeof(T);
    }

    inline void construct(pointer p, const T& val) __attribute__((always_inline)) {
        new(p) T(val);
    }

    inline void destroy(pointer p) __attribute__((always_inline)) {
        p->~T();
    }
};

template<typename OutStream>
void PrintHumanReadableBytes(OutStream& os, size_t n) {
    double bytes = static_cast<double>(n);
    const char *u = " kMG";
    while ((bytes /= 1000) >= 1) {
        ++u;
    }
    os << bytes*1000 << *u << "B (" << n << ")";
}


struct CallStack {
    enum {
        MAX_STACK_SIZE = 12
    };

    void* _stack[MAX_STACK_SIZE];
    size_t _size;

    CallStack() : _size(0) {
        _stack[0] = 0;
    }

    template<typename OutStream>
    void Output(OutStream& os) const {
        os << "<callstack>" << std::endl;
        for (size_t i = 0; i < _size; ++i) {
            const char *symbol = "(unknown)";
            char symbolized[1024];
            if (google::Symbolize(reinterpret_cast<char *> (_stack[i]) - 1, symbolized, sizeof(symbolized))) {
                symbol = symbolized;
                os << "@" << _stack[i] << "    " << symbol << std::endl;
            }
        }
    }
};

struct CallStackHasher {
    size_t operator() (const CallStack& cs) const {
        size_t result = 0;
        std::tr1::hash<void*> hash_oper;
        for (size_t i = 0; i < cs._size; ++i) {
            result ^= hash_oper(cs._stack[i]);
        }
        return result;
    }
};

class CallStackEqualFn {
public:
    bool operator() (const CallStack& cs1, const CallStack& cs2) const {
        if (cs1._size != cs2._size) {
            return false;
        }

        for (size_t i = 0; i < cs1._size; ++i) {
            if (cs1._stack[i] != cs2._stack[i]) {
                return false;
            }
        }
        return true;
    }
};


class CallStackLookupTable {
public:
    typedef std::tr1::unordered_set<CallStack,
        CallStackHasher,
        CallStackEqualFn,
        real_allocator<CallStack> > CallStackHashSet;
    typedef CallStackHashSet::iterator HashSetIterator;
    ~CallStackLookupTable() {}
    static CallStackLookupTable& /*__attribute__((constructor))*/ inst() {
        static CallStackLookupTable o;
        return o;
    }

    CallStack* Lookup() {
        CallStack* call_stack = NULL;
        pthread_mutex_lock(&m_mutex);
        ++m_recursive_counter;
        if (1 == m_recursive_counter) {
            CallStack cs;
            cs._size = backtrace(cs._stack, CallStack::MAX_STACK_SIZE);
            std::pair<HashSetIterator, bool> result = m_call_stack_hash_set.insert(cs);
            call_stack = const_cast<CallStack*>(&(*result.first));
        }
        --m_recursive_counter;
        pthread_mutex_unlock(&m_mutex);
        return call_stack;
    }

private:
    CallStackLookupTable() {
        std::cout << "CallStackLookupTable initialized!" << std::endl;
        pthread_mutexattr_init(&m_attr);
        pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &m_attr);
        m_recursive_counter = 0;
    }

    pthread_mutexattr_t m_attr;
    pthread_mutex_t     m_mutex;
    int                 m_recursive_counter;
    CallStackHashSet    m_call_stack_hash_set;
};


struct LeakMemInfo {
    void*      _mem;
    size_t     _size;
    CallStack* _cs;
    size_t     _sn;
    LeakMemInfo() : _mem(0), _size(0), _cs(0), _sn(0) {}
    ~LeakMemInfo() {}

    template<typename OutStream>
    void Output(OutStream& os) const {
        os << "SN." << _sn << " addr=" << _mem << " size=";
        PrintHumanReadableBytes(os, _size);
        os << std::endl;
        _cs->Output(os);
    }
};

struct LeakMemInfoHasher {
    size_t operator() (const LeakMemInfo& lmi) const {
        std::tr1::hash<void*> hash_oper;
        return hash_oper(lmi._mem);
    }
};

struct LeakMemInfoEqual {
    bool operator() (const LeakMemInfo& lmi1, const LeakMemInfo& lmi2) const {
        return lmi1._mem == lmi2._mem;
    }
};

struct LeakMemInfoSnCompare {
    bool operator() (const LeakMemInfo& a, const LeakMemInfo& b) const {
        return std::less<size_t>()(a._sn, b._sn);
    }
};

struct LeakMemInfoSizeCompare {
    bool operator() (const LeakMemInfo& a, const LeakMemInfo& b) const {
        return std::less<size_t>()(b._size, a._size);
    }
};

template<typename T>
struct more {
    bool operator() (const T& a, const T& b) const {
        return std::less<T>()(b, a);
    }
};

class MemoryLeakDetector {
public:
    typedef std::tr1::unordered_set<LeakMemInfo,
        LeakMemInfoHasher,
        LeakMemInfoEqual,
        real_allocator<LeakMemInfo> > LeakMemoryHashSet;
    typedef LeakMemoryHashSet::iterator HashSetIterator;

    typedef std::set<LeakMemInfo,
        LeakMemInfoSnCompare,
        real_allocator<LeakMemInfo> > LeakTimeOrderedSet;

    typedef std::multiset<LeakMemInfo,
        LeakMemInfoSizeCompare,
        real_allocator<LeakMemInfo> > LeakSizeOrderedSet;

    typedef std::tr1::unordered_map<CallStack*,
        size_t,
        std::tr1::hash<CallStack*>,
        std::equal_to<CallStack*>,
        real_allocator<std::pair<const CallStack*, size_t> > > LeakCallStackHashMap;

    typedef std::multimap<size_t,
        CallStack*,
        more<size_t>,
        real_allocator<std::pair<const size_t, CallStack*> > > LeakCallStackOrderedMap;

    ~MemoryLeakDetector() {
        Dump();
    }

    static MemoryLeakDetector& /*__attribute__((constructor))*/ inst() {
        static MemoryLeakDetector o;
        return o;
    }

    void Insert(void* p, size_t n, CallStack* cs) {
        if (p == NULL) { return; }
        bool need_dump = false;
        pthread_mutex_lock(&m_mutex);
        ++m_recursive_counter;
        if (1 == m_recursive_counter) {
            if (m_report_start_sn > 0) {
                if (m_total_alloc_size > m_phy_mem_use_limit) {
                    need_dump = true;
                }
            }
            ++m_alloc_serial_no;
            __sync_add_and_fetch(&m_total_alloc_size, n);
            LeakMemInfo lmi;
            lmi._mem = p;
            lmi._size = n;
            lmi._cs = cs;
            lmi._sn = m_alloc_serial_no;
            std::pair<HashSetIterator, bool> res = m_leak_mem_hash_set.insert(lmi);
            if (!res.second) {
                ++m_double_alloc_counter;
            }
        }
        --m_recursive_counter;
        pthread_mutex_unlock(&m_mutex);
        if (need_dump) {
            std::cout << "memory use too much, report no." << m_report_no << std::endl;
            Dump(1024);
        }
    }

    void Erase(void* p) {
        if (p == NULL) { return; }
        pthread_mutex_lock(&m_mutex);
        ++m_recursive_counter;
        if (1 == m_recursive_counter) {
            LeakMemInfo lmi;
            lmi._mem = p;
            HashSetIterator iter = m_leak_mem_hash_set.find(lmi);
            if (iter != m_leak_mem_hash_set.end()) {
                __sync_sub_and_fetch(&m_total_alloc_size, iter->_size);
                m_leak_mem_hash_set.erase(iter);
            } else {
                ++m_double_free_counter;
            }
        }
        --m_recursive_counter;
        pthread_mutex_unlock(&m_mutex);
    }

    size_t GetMallocSize() {
        return __sync_add_and_fetch(&m_total_alloc_size, 0);
    }

    size_t GetDetectorSize() {
        return __sync_add_and_fetch(&s_leak_detector_size, 0);
    }

    void Dump(size_t topn = 100) {
        pthread_mutex_lock(&m_mutex);
        ++m_recursive_counter;
        if (1 == m_recursive_counter) {
            std::ostringstream oss;
            oss << "leak-detector-report-" << m_report_no++ << "." << getpid();
            std::ofstream fs(oss.str().c_str(), std::ios::app);
            fs << "<<xcloud leak detector report>>" << std::endl;
            fs << "phy memory size : ";
            PrintHumanReadableBytes(fs, resource::MemInfo::instance()->physical_mem());
            fs << std::endl;
            fs << "phy used size : ";
            PrintHumanReadableBytes(fs, resource::MemInfo::instance()->mem_used());
            fs << std::endl;
            fs << "phy use limit : ";
            PrintHumanReadableBytes(fs, m_phy_mem_use_limit);
            fs << std::endl;
            fs << "current malloc size : ";
            PrintHumanReadableBytes(fs, GetMallocSize());
            fs << std::endl;
            fs << "current detector size : ";
            PrintHumanReadableBytes(fs, GetDetectorSize());
            fs << std::endl;
            fs << "double free counter : " << m_double_free_counter << std::endl;
            fs << "double alloc counter : " << m_double_alloc_counter << std::endl;

            CollectLeakTimeOrderedSet(fs, topn);
            CollectLeakSizeOrderedSet(fs, topn);
            CollectLeakCallStackOrderedMap(fs, topn);
            if (0 == m_report_start_sn) {
                m_report_start_sn = m_alloc_serial_no;
                m_phy_mem_use_limit = resource::MemInfo::instance()->physical_mem() * 7 / 10;
            }
        }
        --m_recursive_counter;
        pthread_mutex_unlock(&m_mutex);
    }

private:
    void CollectLeakTimeOrderedSet(std::ofstream& ofs, size_t topn) {
        LeakTimeOrderedSet leak_time_ordered_set;
        size_t count = 0;
        for (HashSetIterator iter = m_leak_mem_hash_set.begin();
            iter != m_leak_mem_hash_set.end(); ++iter) {
            if (iter->_sn < m_report_start_sn) { continue; }
            if (count < topn) {
                leak_time_ordered_set.insert(*iter);
                ++count;
            } else {
                const LeakMemInfo& cur_lmi = *iter;
                const LeakMemInfo& last_lmi = *(leak_time_ordered_set.rbegin());
                if (LeakMemInfoSnCompare()(cur_lmi, last_lmi)) {
                    leak_time_ordered_set.insert(*iter);
                    LeakTimeOrderedSet::iterator i = leak_time_ordered_set.end();
                    --i;
                    leak_time_ordered_set.erase(i);
                }
            }
        }

        ofs << std::endl << "---Leak memory ordered by time serial no---" << std::endl;
        count = 0;
        for (LeakTimeOrderedSet::iterator i = leak_time_ordered_set.begin();
            i != leak_time_ordered_set.end(); ++i) {
            ofs << std::endl << "TOP." << count++ << std::endl;
            i->Output(ofs);
        }
    }

    void CollectLeakSizeOrderedSet(std::ofstream& ofs, size_t topn) {
        LeakSizeOrderedSet leak_size_ordered_set;
        size_t count = 0;
        for (HashSetIterator iter = m_leak_mem_hash_set.begin();
            iter != m_leak_mem_hash_set.end(); ++iter) {
            if (iter->_sn < m_report_start_sn) { continue; }
            if (count < topn) {
                leak_size_ordered_set.insert(*iter);
                ++count;
            } else {
                const LeakMemInfo& cur_lmi = *iter;
                const LeakMemInfo& last_lmi = *(leak_size_ordered_set.rbegin());
                if (LeakMemInfoSizeCompare()(cur_lmi, last_lmi)) {
                    leak_size_ordered_set.insert(*iter);
                    LeakTimeOrderedSet::iterator i = leak_size_ordered_set.end();
                    --i;
                    leak_size_ordered_set.erase(i);
                }
            }
        }
        ofs << std::endl << "---Leak memory ordered by alloc size---" << std::endl;
        count = 0;
        for (LeakTimeOrderedSet::iterator i = leak_size_ordered_set.begin();
            i != leak_size_ordered_set.end(); ++i) {
            ofs << std::endl << "TOP." << count++ << std::endl;
            i->Output(ofs);
        }
    }

    void CollectLeakCallStackOrderedMap(std::ofstream& ofs, size_t topn) {
        LeakCallStackHashMap leak_call_stack_hash_map;
        for (HashSetIterator iter = m_leak_mem_hash_set.begin();
            iter != m_leak_mem_hash_set.end(); ++iter) {
            if (iter->_sn < m_report_start_sn) { continue; }
            LeakMemInfo& cur_lmi = const_cast<LeakMemInfo&>(*iter);
            LeakCallStackHashMap::iterator i = leak_call_stack_hash_map.find(cur_lmi._cs);
            if (i == leak_call_stack_hash_map.end()) {
                leak_call_stack_hash_map.insert(
                    std::pair<CallStack*, size_t>(
                        cur_lmi._cs, cur_lmi._size));
            } else {
                i->second += cur_lmi._size;
            }
        }

        size_t count = 0;
        LeakCallStackOrderedMap leak_call_stack_ordered_map;
        for (LeakCallStackHashMap::iterator i = leak_call_stack_hash_map.begin();
            i != leak_call_stack_hash_map.end(); ++i) {
            if (count < topn) {
                ++count;
                leak_call_stack_ordered_map.insert(std::pair<const size_t, CallStack*>(i->second, i->first));
            } else {
                if (leak_call_stack_ordered_map.rbegin()->first < i->second) {
                    leak_call_stack_ordered_map.insert(std::pair<const size_t, CallStack*>(i->second, i->first));
                    LeakCallStackOrderedMap::iterator last_iter = leak_call_stack_ordered_map.end();
                    --last_iter;
                    leak_call_stack_ordered_map.erase(last_iter);
                }
            }
        }
        ofs << std::endl
            << "---Leak call stack ordered by alloc size---" << std::endl;
        count = 0;
        for (LeakCallStackOrderedMap::iterator i = leak_call_stack_ordered_map.begin();
            i != leak_call_stack_ordered_map.end(); ++i) {
            ofs << std::endl << "TOP." << count++ << std::endl;
            ofs << "alloc_size=";
            PrintHumanReadableBytes(ofs, i->first);
            ofs << std::endl;
            i->second->Output(ofs);
        }
    }



private:
    MemoryLeakDetector() {
        std::cout << "MemoryLeakDetector initialized!" << std::endl;
        pthread_mutexattr_init(&m_attr);
        pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &m_attr);
        m_recursive_counter = 0;

        m_total_alloc_size = 0;
        m_double_alloc_counter = 0;
        m_double_free_counter = 0;
        m_alloc_serial_no = 0;

        m_report_no = 0;
        m_report_start_sn = 0;

        m_phy_mem_use_limit = 0;
    }

    pthread_mutexattr_t m_attr;
    pthread_mutex_t     m_mutex;
    int                 m_recursive_counter;

    LeakMemoryHashSet   m_leak_mem_hash_set;
    size_t              m_total_alloc_size;
    size_t              m_double_alloc_counter;
    size_t              m_double_free_counter;
    size_t              m_alloc_serial_no;

    size_t              m_report_no;
    size_t              m_report_start_sn;

    size_t              m_phy_mem_use_limit;
};

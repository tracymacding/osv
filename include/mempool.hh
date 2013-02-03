#ifndef MEMPOOL_HH
#define MEMPOOL_HH

#include <cstdint>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/list.hpp>

namespace memory {

using std::size_t;

const size_t page_size = 4096;

extern size_t phys_mem_size;

void* alloc_page();
void free_page(void* page);
void* alloc_page_range(size_t bytes);
void free_page_range(void* start, size_t bytes);
void setup_free_memory(void* start, size_t bytes);

namespace bi = boost::intrusive;

class pool {
public:
    explicit pool(unsigned size);
    ~pool();
    void* alloc();
    void free(void* object);
    unsigned get_size();
    static pool* from_object(void* object);
private:
    struct page_header;
    struct free_object;
private:
    void add_page();
    static page_header* to_header(free_object* object);
private:
    unsigned _size;

    struct page_header {
        pool* owner;
        unsigned nalloc;
        bi::list_member_hook<> free_link;
        free_object* local_free;  // free objects in this page
    };

    typedef bi::list<page_header,
                     bi::member_hook<page_header,
                                     bi::list_member_hook<>,
                                     &page_header::free_link>,
                     bi::constant_time_size<false>
                    > free_list_type;
    free_list_type _free;
public:
    static const size_t max_object_size;
    static const size_t min_object_size;
};

struct pool::free_object {
    free_object* next;
    page_header* to_page_header();
};

class malloc_pool : public pool {
public:
    malloc_pool();
private:
    static size_t compute_object_size(unsigned pos);
};

struct page_range {
    explicit page_range(size_t size);
    size_t size;
    boost::intrusive::set_member_hook<> member_hook;
};

void free_initial_memory_range(void* addr, size_t size);

}

#endif
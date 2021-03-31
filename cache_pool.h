#include <mutex>
#include <string.h>

typedef struct cache_s cache_t;
struct cache_s {
    void *data_;
    cache_s *next;
};

typedef void(*init_cache_handler)(cache_t *cache, void *private_data);
typedef void(*del_cache_handler)(cache_t *cache, void *private_data);

class cache_pool {
private:
    unsigned char *array_;
    cache_t *free_;
    std::mutex mut_;
    del_cache_handler del_cache_handler_;
    unsigned int block_count_;
    unsigned int item_size_;
    void *private_data_;
public:
    cache_pool();
    ~cache_pool();
    bool create_cache_pool(void *private_data, init_cache_handler init_handler, del_cache_handler del_handler, unsigned int block_count, unsigned int block_size);
    void free_item(cache_t *cache);
    cache_t *get_item();
};
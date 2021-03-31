#include "cache_pool.h"

cache_pool::cache_pool() : array_(nullptr), free_(nullptr), block_count_(0) {}

cache_pool::~cache_pool() {
    if (array_) {
        unsigned char *array_item = array_;
        cache_t *cache_item = nullptr;
        std::unique_lock<std::mutex> lk { mut_ };
        for (int i=0; i<block_count_; i++) {
            cache_item = (cache_t*)array_item;
            if (del_cache_handler_) {
                del_cache_handler_(cache_item, private_data_);
            }
            array_item = array_item + item_size_;
        }
        free(array_);
        array_ = nullptr;
        free_ = nullptr;
    }
}

bool cache_pool::create_cache_pool(void *private_data, init_cache_handler init_handler, del_cache_handler del_handler, unsigned int block_count, unsigned int block_size) {
    del_cache_handler_ = del_handler;
    block_count_ = block_count;
    private_data_ = private_data;
    item_size_ = sizeof(cache_t) + block_size;
    try {
        unsigned char* array_item = nullptr;
        array_ = (unsigned char*)malloc(block_count * item_size_);
        if (array_) {
            memset(array_, 0, block_count * item_size_);
            array_item = array_;
        } else {
            return false;
        }
        cache_t *tmp_cache = nullptr;
        cache_t *pre_cache = (cache_t *)array_item;
        pre_cache->data_ = (void*)(pre_cache + 1);
        pre_cache->next = nullptr;

        if (init_handler) {
            init_handler(pre_cache, private_data);
        }

        free_ = pre_cache;

        for(int i=1; i<block_count; i++) {
            array_item = array_item + item_size_;
            tmp_cache = (cache_t *)array_item;
            tmp_cache->data_ = (void*)(tmp_cache + 1);
            tmp_cache->next = nullptr;
            if (init_handler) {
                init_handler(tmp_cache, private_data);
            }
            pre_cache->next = tmp_cache;
            pre_cache = tmp_cache;
        }
    } catch(std::bad_alloc &e) {
        return false;
    }
    return true;
}

void cache_pool::free_item(cache_t *cache) {
    std::unique_lock<std::mutex> lk{ mut_ };
    if (cache) {
        cache->next = free_;
        free_ = cache;
    }
}

cache_t *cache_pool::get_item() {
    cache_t *res = nullptr;
    std::unique_lock<std::mutex> lk { mut_ };
    if (free_) {
        res = free_;
        free_ = free_->next;
        res->next = nullptr;
    }
    return res;
}
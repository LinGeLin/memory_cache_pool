#include <iostream>
#include <thread>
#include "cache_pool.h"

typedef struct ts {
    int x;
    int y;
    unsigned char* name;
};

int count = 0;
void init_handler(cache_t *cache_item, void *private_data) {
    ts* ttt = (ts*)cache_item->data_;
    ttt->name = (unsigned char*)malloc(32);
    strcpy((char*)ttt->name, "hello world");
    ttt->x = count ++;
    ttt->y = 20;
    std::cout << "init count" << ttt->x << std::endl;
}

void del_handler(cache_t *cache_item, void *private_data) {
    ts *ttt = (ts*)cache_item->data_;
    if (ttt->name) {
        free(ttt->name);
        std::cout << "free" << std::endl;
        std::cout << "count :" << ttt->x << std::endl;
    }
}

void thread1(cache_pool *cp) {
    cache_t *tt = NULL;
    for(int i=0; i<5; i++) {
        tt = cp->get_item();
        std::cout << "thread1:" << ((ts*)(tt->data_))->x << std::endl;
        cp->free_item(tt);
    }
}

void thread2(cache_pool *cp) {
    cache_t *tt = NULL;
    for(int i=0; i<5; i++) {
        tt = cp->get_item();
        std::cout << "thread2:" << ((ts*)(tt->data_))->x << std::endl;
        cp->free_item(tt);
    }
}

int main() {
    cache_pool *cp = new cache_pool();
    cp->create_cache_pool(nullptr, init_handler, del_handler, 10, sizeof(ts));
    std::thread t1(thread1, cp);
    std::thread t2(thread2, cp);
    t1.join();
    t2.join();
    delete cp;
    return 0;
}
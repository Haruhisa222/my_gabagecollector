
/*
mallocしたメモリアドレスを順に記録、最後にまとめてfreeするプログラム

1.mallocするたび、mallocしたアドレスを指す構造体を作る
2.その構造体達を連結リストにしておく
3.連結リストを順に辿ってmallocしたアドレスをfreeしていく
*/

#include <stdio.h>
#include <stdlib.h>

//mallocしたアドレスを指す構造体。nextを持たせて連結リストにする
typedef struct Block {
    void* ptr;
    struct Block* next;
} Block;

//先頭の構造体を保持(一番最近作った構造体を追う)
Block* head = NULL;

//size分mallocし、それを追う構造体を作り、nextを更新
void* gc_malloc(size_t size) {
    void* p = malloc(size);

    Block* b = malloc(sizeof(Block));
    b->ptr = p;
    b->next = head;
    head = b;

    return p;
}

//連結リストをheadから順に見ていき、mallocしたものをfreeする
void gc_free_all() {
    Block* cur = head;

    while (cur) {
        free(cur->ptr);

        Block* tmp = cur;
        cur = cur->next;
        free(tmp);
    }

    head = NULL;
}

int main() {
    int* a = gc_malloc(sizeof(int));
    int* b = gc_malloc(sizeof(int));

    *a = 10;
    *b = 20;

    printf("ポインタaの指す値:%d ポインタbの指す値:%d\n", *a, *b);
    printf("ポインタaの指すアドレス:%p ポインタbの指すアドレス:%p\n", a, b);

    gc_free_all();
    printf("---free---\n");
    
    printf("ポインタaの指すアドレス:%p(free済) ポインタbの指すアドレス:%p(free済)\n", a, b);
    printf("free は指定アドレスをnullにするのではなく、使えないようにするだけ\n");
}
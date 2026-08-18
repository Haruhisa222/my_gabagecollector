
/*
mallocしたメモリアドレスを順に記録、最後にまとめてfreeするプログラム
に、手動で「使用中」の印を付ける（マーク）

1.mallocするたび、mallocしたアドレスを指す構造体を作る
2.その構造体達を連結リストにしておく
3.連結リストを順に辿ってmallocしたアドレスをfreeしていく
*/

#include <stdio.h>
#include <stdlib.h>

//mallocしたアドレスを指す構造体。nextを持たせて連結リストにする marked追加
// Blockの構造
// ┌─────────────┐
// │ ptr    ────────→ mallocされた領域
// │ marked = 0  │
// │ next   ────────→ 次のBlock
// └─────────────┘
typedef struct Block {
    void* ptr;
    int marked;
    struct Block* next;
} Block;

//先頭の構造体を保持(一番最近作った構造体を追う)
Block* head = NULL;

//size分mallocし、それを追う構造体を作り、nextを更新
void* gc_malloc(size_t size) {
    void* p = malloc(size);

    Block* b = malloc(sizeof(Block));
    b->ptr = p;
    b->marked = 0;
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

//指定したBlockのmarkedを1にする
void gc_mark(void* ptr){
    Block* cur = head;

    while(cur){
        if(cur->ptr == ptr){//今見てるcurと、この関数の引数のptrが同じならmarkd
            cur->marked = 1;
            return;
        }

        cur = cur->next;//先頭(head)から順にBlockを見ていく
    }
}

//各Blockのアドレス・管理対象のアドレス・markedの値を表示
void gc_print_blocks() {
    Block* cur = head;

    while (cur) {
        printf("ptr: %p, marked: %d\n", cur->ptr, cur->marked);
        cur = cur->next;
    }
}

int main() {
    int* a = gc_malloc(sizeof(int));
    int* b = gc_malloc(sizeof(int));
    int* c = gc_malloc(sizeof(int));

    *a = 10;
    *b = 20;
    *c = 30;

    gc_mark(a);
    gc_mark(c);

    gc_print_blocks();
}
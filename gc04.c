
/*
マークアンドスイープを自動化する

Blockリスト(malloc領域自体を追跡) の他に、
Rootリスト(mallocしたポインタごと追跡)　を追加

【Blockリスト】
「mallocされた領域自体をメモ」
Block ─────→ Block
 ptr          ptr
  ↓            ↓
領域A          領域B
  ↑            ↑
  a            b

【Rootリスト】
「プログラム上のポインタ変数ごと管理」
Root ──────→ Root
 ptr           ptr
  ↓             ↓
 &a             &b
 ↓              ↓
 a              b
 ↓              ↓
領域A          領域B

これにより、プログラム上のポインタがmallocした領域を指さなくなった場合、
その領域を自動でfreesできるようになる

ex) malloc領域Cを指すポインタcがある場合
1. c = NULL;//ポインタcは何も指さなくなり、領域Cはメモリリーク
2. Rootリストを辿った結果、cが何も指してないことが判明
3. Blockリストにメモってある領域Cをfree

*/







#include <stdio.h>
#include <stdlib.h>

//mallocされた領域自体をメモ
typedef struct Block {
    void* ptr; //mallocされた領域を追うポインタ
    int marked;
    struct Block* next;
} Block;

//先頭の構造体を保持(headは一番最近作った構造体を追う)
Block* head = NULL;

//プログラム上のポインタ変数ごと管理
typedef struct Root{
    void** ptr;//プログラム上のポインタを追うポインタ
    struct Root* next;
} Root;

//先頭の構造体を保持(headは一番最近作った構造体を追う)
Root* root_head = NULL;

//size分mallocし、それを追う構造体を作り、nextを更新
void* gc_malloc(size_t size) {
    void* p = malloc(size);

    Block* b = malloc(sizeof(Block));
    b->ptr = p;
    b->marked = 0;
    b->next = head;
    head = b;//headにこの構造体を追わせる

    return p;
}

//プログラム上のポインタ変数をRootに登録
void gc_add_root(void** ptr){//ptrは、引数である「プログラム中のポインタ変数自体のアドレス」を追う
    Root* r = malloc(sizeof(Root));

    r->ptr = ptr;
    r->next = root_head;
    root_head = r;
}

//「BlockリストとRootリスト」をheadから順に見ていき、mallocしたもの全てをfreeする(未使用)
void gc_free_all() {
    Block* cur = head;
    while (cur) {
        free(cur->ptr);
        Block* tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    head = NULL;

    Root* rcur = root_head;
    while (rcur) {
        Root* tmp = rcur;
        rcur = rcur->next;
        free(tmp);
    }
    head = NULL;
}

//指定したBlockのmarkedを1にする
void gc_mark(void* ptr){
    Block* cur = head;//Blockを追跡するポインタ

    while(cur){
        if(cur->ptr == ptr){//今見てるcurと、この関数の引数のptrが同じならmarkd
            cur->marked = 1;
            return;
        }

        cur = cur->next;//先頭(head)から順にBlockを見ていく
    }
}

//全Blockを見てmarkedされていないBlockをfree
void gc_sweep(){
    Block** cur = &head;//headや.next を指す二重ポインタ
    while (*cur)
    {
        Block* b = *cur;//blockを見るポインタ

        if(!b->marked){
            *cur = b->next;//「今見てるblockを指しているnext」に「今見てるblockのnext」を代入
            free(b->ptr);
            free(b);
        }else{
            b->marked = 0;
            cur = &b->next;//二重ポインタが「今見てるblockのnext自体」を指す
        }
    }

}

//Rootを辿って、参照先のあるポインタを自動mark
void gc_mark_roots(){
    Root* cur = root_head;

    while(cur){
        void* p = *(cur->ptr);//ex) cur->ptr:&aなら、*(cur->ptr):*(&a):a、つまりvoid*p=a;

        if(p != NULL){
            gc_mark(p);
        }

        cur=cur->next;
        
    }
}

//自動GCで呼ぶのをまとめる
void gc_collect(){
    gc_mark_roots();
    gc_sweep();
}

//管理対象のアドレスを表示
void gc_print_blocks() {
    Block* cur = head;
    while (cur) {
        printf("ptr: %p\n", cur->ptr);
        cur = cur->next;
    }
}

int main() {
    int* a = gc_malloc(sizeof(int));
    int* b = gc_malloc(sizeof(int));
    int* c = gc_malloc(sizeof(int));

    *a = 10;
    gc_add_root((void**)&a);

    *b = 20;
    gc_add_root((void**)&b);

    *c = 30;
    gc_add_root((void**)&c);

    gc_print_blocks();

    b = NULL;

    printf("------- b = NULL; gc_collect; ----------\n");

    gc_collect();

    gc_print_blocks();

}
# My Garbage Collector in C

C言語でGarbage Collection（GC）の仕組みを理解するために、ゼロから実装している学習用プロジェクトです。

## 概要

C言語では、`malloc()`で確保したメモリはプログラマ自身が`free()`する必要があります。

```c
int* p = malloc(sizeof(int));

*p = 100;

/* 不要になった場合 */
free(p);
```

しかし、プログラムが複雑になると、

- `free()`の書き忘れによるメモリリーク
- どのタイミングで`free()`すればよいか分からなくなる
- 複雑なポインタ関係の管理が難しくなる

といった問題が発生します。

そこでこのプロジェクトでは、C言語で簡単なGarbage Collectorを自作しながら、

- `malloc` / `free`
- ポインタ・二重ポインタ
- 連結リスト
- メモリ管理
- Mark & Sweep
- Root Set
- オブジェクト間参照
- Conservative GC
- 世代別GC / Copying GC

などの仕組みを学習します。

## 最終目標

最終的には、通常の

```c
int* p = malloc(sizeof(int));
```

を

```c
int* p = my_malloc(sizeof(int));
```

に置き換える程度で、自動的に不要なメモリを検出・解放できるGCを目指します。

理想的には、

```c
void func(void) {
    int* p = my_malloc(sizeof(int));

    *p = 100;

    printf("%d\n", *p);

    // free(p)を書かなくても、
    // 到達不能になったメモリをGCが回収する
}
```

のように使用できることを目標としています。

## このプロジェクトの目的

主目的はGarbage CollectionとC言語のメモリ管理について学習することです。

また、C言語でメモリを直接扱いたい一方で、`free()`の管理やメモリリークをできるだけ減らしたい場合に使える、小さなGCライブラリに発展させることも目標としています。

> [!WARNING]
> 現在は学習目的の実装です。
> 実用的なGCライブラリとしての安全性・性能・移植性は保証していません。

## 実装ロードマップ

### gc01.c - mallocした領域を記録

`malloc()`で確保した領域を連結リストで管理します。

```text
head
 ↓
Block → Block → Block
 ↓       ↓       ↓
ptr     ptr      ptr
 ↓       ↓       ↓
領域A    領域B    領域C
```

GCが「どのメモリを確保したのか」を把握するための基本部分です。

---

### gc02.c - Mark

各Blockに`marked`フラグを追加します。

```c
typedef struct Block {
    void* ptr;
    int marked;
    struct Block* next;
} Block;
```

使用中のメモリに

```text
marked = 1
```

という印を付けられるようにします。

---

### gc03.c - Sweep

Blockリストを走査し、

```text
marked = 1 → 使用中なので残す
marked = 0 → 不要なのでfree
```

というMark & Sweepの基本処理を実装します。

---

### gc04.c - Root Set

Blockリストに加えてRootリストを導入します。

```text
Blockリスト
「確保したメモリの一覧」

Block → Block → Block
 ↓       ↓       ↓
 A       B       C


Rootリスト
「プログラムからメモリへ到達するための入口」

Root → Root
 ↓       ↓
&a      &b
 ↓       ↓
 a       b
 ↓       ↓
 A       B
```

例えば、

```c
b = NULL;
```

になると、

```text
Root
 ↓
&b
 ↓
 b → NULL

       B ← Blockリストには残っている
```

となります。

RootからBへ到達できないためBはMarkされず、Sweep時にBlockリストからBを発見して解放します。

これによって、人間が直接`gc_mark()`するのではなく、GC自身がRoot SetからMarkできるようになります。

---

### gc05.c - オブジェクト間参照の追跡

今後実装予定。

```text
Root
 ↓
 A → B → C
```

のような場合に、Rootから直接参照されているAだけでなく、AからB、BからCへと参照を再帰的に辿り、到達可能なオブジェクトをすべてMarkできるようにします。

---

### gc06.c - Conservative GC

今後実装予定。

スタック領域を走査し、GCが管理しているメモリを指している可能性のある値を探します。

これによって、

```c
gc_add_root((void**)&p);
```

のようなRootの手動登録を不要にし、より自動的なGCを目指します。

---

### gc07以降 - GCの発展

基本的なMark & Sweep完成後、

- Generational GC（世代別GC）
- Copying GC
- GCの性能改善

など、より発展的なGCについて実装・学習する予定です。

## GCの基本的な考え方

このGCでは、

> 「今後使われるかどうか」

を予測するのではなく、

> 「現在、Rootからそのメモリへ到達できるか」

によって生存判定を行います。

```text
確保済みメモリ

A   B   C   D
↑       ↑
Rootから到達可能

        ↓

A, C → Mark

        ↓

B, D → Markされない

        ↓

Sweepでfree
```

この

```text
Mark → Sweep
```

が本プロジェクトの基本となるGCアルゴリズムです。

## 現在の進捗

- [x] malloc領域の追跡
- [x] Mark
- [x] Sweep
- [x] Root Setの導入
- [ ] オブジェクト間参照の追跡
- [ ] スタック走査によるConservative GC
- [ ] `malloc` → `my_malloc` の置き換えだけで利用できるAPI化
- [ ] Generational GC
- [ ] Copying GC

## 注意

このプロジェクトはGC・ポインタ・メモリ管理の学習を目的として、自作しているものです。

現時点では、実際のアプリケーションや重要なデータを扱うプログラムでの使用は想定していません。
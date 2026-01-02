#ifndef BASE_LIST_H
#define BASE_LIST_H

#include "base_core.h"
#include "base_arena.h"

#define Generate_List(T, Name)                                                    \
                                                                                  \
typedef struct Name Name;                                                         \
struct Name {                                                                     \
    T     *array;                                                                 \
    usize  len;                                                                   \
    usize  cap;                                                                   \
    Arena *arena;                                                                 \
};                                                                                \
                                                                                  \
internal Name                                                                     \
Name##_make(Arena *arena, usize initial_cap) {                                    \
    Name list = {0};                                                              \
    list.arena = arena;                                                           \
    list.cap   = initial_cap;                                                     \
    if (initial_cap) {                                                            \
        list.array = (T *)arena_alloc(                                            \
            arena,                                                                \
            sizeof(T) * initial_cap,                                              \
            AlignOf(T),                                                           \
            Code_Location);                                                       \
    }                                                                             \
    return list;                                                                  \
}                                                                                 \
                                                                                  \
internal void                                                                     \
Name##_reserve(Name *list, usize new_cap) {                                       \
    if (new_cap <= list->cap) return;                                             \
                                                                                  \
    usize old_size = sizeof(T) * list->cap;                                       \
    usize new_size = sizeof(T) * new_cap;                                         \
                                                                                  \
    list->array = (T *)arena_realloc(                                             \
        list->arena,                                                              \
        list->array,                                                              \
        new_size,                                                                 \
        old_size);                                                                \
                                                                                  \
    list->cap = new_cap;                                                          \
}                                                                                 \
                                                                                  \
internal void                                                                     \
Name##_push(Name *list, T value) {                                                \
    if (list->len + 1 > list->cap) {                                              \
        usize new_cap = list->cap ? list->cap * 2 : 8;                            \
        Name##_reserve(list, new_cap);                                            \
    }                                                                             \
    list->array[list->len++] = value;                                             \
}                                                                                 \
                                                                                  \
force_inline T *                                                                  \
Name##_at(Name *list, usize index) {                                              \
    Assert(index < list->len);                                                    \
    return &list->array[index];                                                   \
}                                                                                 \
                                                                                  \
force_inline T *                                                                  \
Name##_last(Name *list) {                                                         \
    Assert(list->len > 0);                                                        \
    return &list->array[list->len - 1];                                           \
}                                                                                 \
                                                                                  \
force_inline void                                                                 \
Name##_clear(Name *list) {                                                        \
    list->len = 0;                                                                \
}

#endif

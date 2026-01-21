#ifndef ARENA_H
#define ARENA_H

#define ARENA_HEADER_SIZE 128

typedef struct Arena Arena;
struct Arena {
  Arena *prev;
  Arena *cur;
  b32 cannot_chain;
  b32 has_backing_buffer;
  // TODO virtual memory
  //u64 reserve_size; // virtual memory reserved
  u64 size;  // actual memory committed
  u64 base_pos;
  u64 pos;
  u64 free_size;
  Arena *free_last;
};

STATIC_ASSERT(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

typedef struct ArenaScope ArenaScope;
struct ArenaScope {
  Arena *arena;
  u64 pos;
};


global read_only u64 ARENA_DEFAULT_SIZE = KB(64);

#define arena_create(size) arena_create_ex(((u64)(size)), 0, (void*)0)
Arena* arena_create_ex(u64 size, b32 cannot_chain, void *backing_buffer);

void arena_destroy(Arena *arena);

void *arena_push(Arena *arena, u64 size, u64 align);
u64   arena_pos(Arena *arena);
void  arena_pop_to(Arena *arena, u64 pos);

void arena_clear(Arena *arena);
void arena_pop(Arena *arena, u64 amount);

ArenaScope arena_scope_begin(Arena *arena);
void arena_scope_end(ArenaScope scope);

#define arena_scope(a) for(ArenaScope __scope__##__LINE__ = arena_scope_begin((a)); __scope__##__LINE__.arena != (Arena*)0; arena_scope_end(__scope__##__LINE__), __scope__##__LINE__.arena = (Arena*)0)

#define push_array_no_zero_aligned(a, T, n, align) (T*)arena_push((a), sizeof(T)*(n), (align))
#define push_array_aligned(a, T, n, align) (T*)memory_zero(push_array_no_zero_aligned(a, T, n, align), sizeof(T)*(n))
#define push_array_no_zero(a, T, n) push_array_no_zero_aligned(a, T, n, MAX(8, alignof(T)))
#define push_array(a, T, n) push_array_aligned(a, T, n, MAX(8, alignof(T)))
#define push_struct(a, T) push_array(a, T, 1)
#define push_struct_no_zero(a, T) push_array_no_zero(a, T, 1)


#endif

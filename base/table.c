#ifndef ARRAY_C
#define ARRAY_C

void arr_init_(__Arr_header *arr, Arena *arena, s64 stride, s64 cap) {
  arr->count = 0;
  arr->cap = cap;
  arr->arena = arena;
  arr->d = arena_push(arena, cap * stride, 1);
}

void* arr_push_no_zero_(__Arr_header *arr, s64 stride, s64 push_count) {
  ASSERT(arr->d && arr->cap && arr->arena);

  if(arr->count + push_count >= arr->cap) {
    s64 new_cap = arr->cap << 1;

    while(new_cap < arr->count + push_count) {
      new_cap <<= 1;
    }

    void *new_d = arena_push(arr->arena, new_cap * stride, 1);
    memory_copy(new_d, arr->d, stride * (arr->count + push_count));
    arr->d = new_d;
    arr->cap = new_cap;
  }

  void *result = (u8*)(arr->d) + arr->count;
  arr->count += push_count;

  return result;
}

#endif

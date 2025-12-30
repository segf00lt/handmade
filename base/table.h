#ifndef ARRAY_H
#define ARRAY_H


typedef struct __Map_header __Arr_header;
struct __Arr_header {
  void *d;
  s64 count;
  s64 cap;

  Arena *arena;
};

typedef struct __Slice_header __Slice_header;
struct __Slice_header {
  void *d;
  s64 count;
};

#define DECL_ARR_TYPE(T) DECL_ARR_TYPE_NAME(T, Arr_##T)
#define DECL_SLICE_TYPE(T) DECL_SLICE_TYPE_NAME(T, Slice_##T)

#define DECL_ARR_TYPE_NAME(T, name) \
  typedef struct name name; \
  struct name {                 \
    T *d;                           \
    s64 count;                      \
    s64 cap;                        \
    Arena *arena;                   \
  };                                \

#define DECL_SLICE_TYPE_NAME(T, name) \
  typedef struct name name; \
  struct name {                   \
    T *d;                               \
    s64 count;                          \
  };                                    \


#define ARRAY_DEFAULT_CAP 64


#define Arr(T)   struct { T *d; s64 count; s64 cap; Arena *arena; }
#define Slice(T) struct { T *d; s64 count; }

#define header_from_arr(arr)         (*(__Arr_header*)(void*)(&(arr)))
#define header_from_arr_ptr(arr)     (*(__Arr_header*)(void*)(arr))
#define header_ptr_from_arr(arr)     ((__Arr_header*)(void*)(&(arr)))
#define header_ptr_from_arr_ptr(arr) ((__Arr_header*)(void*)(arr))

#define header_from_slice(slice)         (*(__Slice_header*)(void*)(&(slice)))
#define header_from_slice_ptr(slice)     (*(__Slice_header*)(void*)(slice))
#define header_ptr_from_slice(slice)     ((__Slice_header*)(void*)(&(slice)))
#define header_ptr_from_slice_ptr(slice) ((__Slice_header*)(void*)(slice))

#define arr_stride(array) ((s64)sizeof(*((array).d)))
#define arr_ptr_stride(array) ((s64)sizeof(*((array)->d)))

#define arr_init(array, arena) arr_init_(header_ptr_from_arr((array)), arena, arr_stride(array), ARRAY_DEFAULT_CAP)
#define arr_init_ex(array, arena, cap) arr_init_(header_ptr_from_arr((array)), arena, arr_stride(array), cap)

#define arr_push(array, elem) ((arr_push_no_zero_(header_ptr_from_arr((array)), arr_stride(array), 1)), (array).d[(array).count-1] = (elem))
#define arr_push_n_ptr(array, n) ((arr_push_no_zero_(header_ptr_from_arr((array)), arr_stride(array), (n))), &((array).d[(array).count - (n)]))
#define arr_push_n_index(array, n) ((arr_push_no_zero_(header_ptr_from_arr((array)), arr_stride(array), (n))), (s64)((array).count - (n)))

#define arr_clear(array) ((array).count = 0, memory_zero((array).d, arr_stride((array)) * (array).cap))

#define arr_pop(array)        ( ( ((array).count > 0) ? ((array).count--) : (0) ), ( ((array).count <= 0) ? (arr_clear((array))) : (0) ), (array).d[(array).count] )

#define arr_first(array) ((array).d[0])

#define arr_last(array) ( (array).d[ ( ((array).count > 0) ? ((array).count - 1) : (0) ) ] )

#define arr_to_slice(T, array) (*(Slice(T)*)(&(array)))
#define carray_to_slice(T, carray) ((Slice(T)){ .d = (T*)carray, .count = (sizeof(carray)/sizeof(T)) })

#define slice_last arr_last
#define slice_stride arr_stride

void  arr_init_(__Arr_header *arr, Arena *arena, s64 stride, s64 cap);
void* arr_push_no_zero_(__Arr_header *arr, s64 stride, s64 push_count);

#endif

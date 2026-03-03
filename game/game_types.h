#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#define MAX_ENTITIES 128

#define ENTITY_KINDS \
X(PLAYER_1) \
X(PLAYER_2) \

#define ENTITY_FLAGS \

typedef enum Entity_kind {
  ENTITY_KIND_NONE = 0,
  #define X(x) ENTITY_KIND_##x,
  ENTITY_KINDS
  #undef X
  ENTITY_KIND_MAX,
} Entity_kind;

typedef enum Entity_flags_index {
  ENTITY_FLAG_INDEX_NONE = -1,
  #define X(x) ENTITY_FLAG_INDEX_##x,
  ENTITY_FLAGS
  #undef X
  ENTITY_FLAG_INDEX_MAX,
} Entity_flags_index;

typedef u64 Entity_flags;
#define X(x) const Entity_flags ENTITY_FLAG_##x = ((u64)1 << ENTITY_FLAG_INDEX_##x);
ENTITY_FLAGS
#undef X


typedef struct Color Color;
struct Color {
  f32 r;
  f32 g;
  f32 b;
  f32 a;
};

typedef struct Game_sound_buffer Game_sound_buffer;
struct Game_sound_buffer {
  int samples_per_second;
  int sample_count;
  s16 *samples;
};

typedef struct Game_render_buffer Game_render_buffer;
struct Game_render_buffer {
  u8 *pixels;
  s32 width;
  s32 height;
  u32 stride; // NOTE jfd: This comes from the backbuffer thing
};

typedef struct Game_input Game_input;
struct Game_input {
  Keyboard_modifier modifier_mask;
  u32 key_pressed[KBD_KEY_MAX];
  b32 key_released[KBD_KEY_MAX];
  v2 mouse_pos;
  v2 mouse_delta;
  v2 scroll_delta;
};

TYPEDEF_SLICE(Game_input);

typedef struct Chunk Chunk;
struct Chunk {
  u8 *tiles;
};

typedef struct Chunk_pos Chunk_pos;
struct Chunk_pos {
  s32 chunk_x;
  s32 chunk_y;
  s32 chunk_z;
  v2_s32 tile;
};

typedef struct Tile_map_pos Tile_map_pos;
struct Tile_map_pos {
  u32 tile_x;
  u32 tile_y;
  u32 tile_z;
  v2 tile_offset;
};

typedef struct Bitmap Bitmap;
struct Bitmap {
  u32 *pixels;
  s32 width;
  s32 height;
};

#pragma pack(push, 1)
typedef struct Bitmap_header Bitmap_header;
struct Bitmap_header {
  u16 file_type;     /* File type, always 4D42h ("BM") */
  u32 file_size;     /* Size of the file in bytes */
  u16 reserved_1;    /* Always 0 */
  u16 reserved_2;    /* Always 0 */
  u32 bitmap_offset; /* Starting position of image data in bytes */
  u32 size;            /* Size of this header in bytes */
  s32 width;           /* Image width in pixels */
  s32 height;          /* Image height in pixels */
  u16 planes;          /* Number of color planes */
  u16 bits_per_pixel;    /* Number of bits per pixel */
  u32 compression;
  u32 size_of_bitmap;
  s32 horizontal_resolution;
  s32 vertical_resolution;
  u32 colors_user;
  u32 colors_important;
};
#pragma pack(pop)

typedef struct Entity Entity;
struct Entity {
  Entity_kind kind;
  Entity_flags flags;

  Tile_map_pos pos;
  f32       width;
  f32       height;
  f32       mass;
  v2        vel;
  v2        accel;

  b32 changed_z;

  Bitmap bitmap;

};

typedef struct Game Game;
struct Game {
  f32 t;
  b32 did_reload;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *temp_arena;

  Game_render_buffer render;
  Game_sound_buffer sound;
  Game_input input;

  s32 test_sound_pitch;

  Tile_map_pos camera_pos;
  Tile_map_pos camera_offset;

  b8 once;
  b8 should_init_player;

  u32 world_chunks_x_count;
  u32 world_chunks_y_count;
  u32 world_chunks_z_count;
  Chunk *world_chunks;
  Chunk chunk;

  s32 random_number_index;

  f32 pixels_per_meter;
  f32 meters_per_pixel;

  s32 tiles_per_room_width;
  s32 tiles_per_room_height;

  Entity entities[MAX_ENTITIES];

  Bitmap guy_bitmap;

  Tile_map_pos debug_new_player_pos;

};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))




#endif

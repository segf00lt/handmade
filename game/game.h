#ifndef GAME_H
#define GAME_H


#define KM(x) ((f32)(x) * 1.0e3f)
#define M(x)  ((f32)x)
#define DM(x) ((f32)(x) * 1.0e-1f)
#define CM(x) ((f32)(x) * 1.0e-2f)
#define MM(x) ((f32)(x) * 1.0e-3f)

#define CHUNK_SHIFT                   4
#define CHUNK_SIZE                    (1 << CHUNK_SHIFT)
#define CHUNK_MASK                    (CHUNK_SIZE - 1)
#define TILE_SIZE_METERS              M(2.0f)
#define CHUNK_HASH_TABLE_COUNT        (1 << 10)
#define CHUNK_SAFE_MARGIN             (MAX_S32/64)

// #define PLAYER_ACCEL   (M(230))
#define PLAYER_ACCEL   (M(500))
#define FROG_ACCEL     (M(40))
#define MONSTER_ACCEL  (M(180))

#define MONSTER_ATTACK_DAMAGE 1

#define PIXELS_PER_METER (13.0f)
#define MIN_PIXELS_PER_METER (4.0f)
#define MAX_PIXELS_PER_METER (PIXELS_PER_METER*2.0f)
#define METERS_PER_PIXEL (1.0f/PIXELS_PER_METER)
#define METERS_TO_PIXELS(x) ((f32)((f32)(x)*PIXELS_PER_METER))
#define PIXELS_TO_METERS(x) ((f32)((f32)(x)*METERS_PER_PIXEL))

#define MAX_ENTITIES_PER_CHUNK 1024
#define MAX_ENTITIES 4096
#define MAX_SIM_REGION_ENTITIES (8*MAX_ENTITIES_PER_CHUNK)
#define MAX_SIM_REGION_CHUNKS 1024

#define ENTITY_KINDS \
X(PLAYER_1) \
X(PLAYER_2) \
X(MONSTER) \
X(BULLET) \
X(FROG) \
X(WALL) \
X(DOOR_UP) \
X(DOOR_DOWN) \

#define ENTITY_FLAGS \
X(DIE_NOW) \
X(DRAW_BITMAP) \
X(APPLY_FRICTION) \
X(ACCEL_MOTION) \
X(SLOW) \
X(APPLY_DAMAGE) \
X(IGNORE_DAMAGE) \
X(APPLY_COLLISION) \
X(IGNORE_COLLISION) \
X(TILE_COLLISION) \
/* TODO jfd 12/03/26: remove these flags once we change to a better collision system */ \
X(TILE_LEFT_ENABLED) \
X(TILE_RIGHT_ENABLED) \
X(TILE_TOP_ENABLED) \
X(TILE_BOTTOM_ENABLED) \
X(TILE_BOTTOM_LEFT_CORNER_ENABLED) \
X(TILE_BOTTOM_RIGHT_CORNER_ENABLED) \
X(TILE_TOP_LEFT_CORNER_ENABLED) \
X(TILE_TOP_RIGHT_CORNER_ENABLED) \
X(BLINK_RED) \
X(DRAW_RED_TINT) \
X(DONT_UPDATE_THIS_FRAME) \

#define ENTITY_ORDERS   \
X(FIRST) \
X(SECOND) \
X(LAST) \

#define ENTITY_CONTROLS \
X(PLAYER_1) \
X(PLAYER_2) \
X(MONSTER_FOLLOW_AND_ATTACK) \
X(FROG_FOLLOW) \
X(BULLET) \

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

typedef enum Entity_order {
  ENTITY_ORDER_MIN = -1,
  #define X(x) ENTITY_ORDER_##x,
  ENTITY_ORDERS
  #undef X
  ENTITY_ORDER_MAX,
} Entity_order;

typedef enum Entity_control {
  ENTITY_CONTROL_NONE = 0,
  #define X(x) ENTITY_CONTROL_##x,
  ENTITY_CONTROLS
  #undef X
  ENTITY_CONTROL_MAX,
} Entity_control;

TYPEDEF_ARRAY_NAME(int, Int_array);

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

typedef struct Bitmap Bitmap;
struct Bitmap {
  u32 *pixels;
  s32 width;
  s32 height;
};

#pragma pack(push, 1)
typedef struct Bitmap_header Bitmap_header;
struct Bitmap_header {
  u16 file_type;         /* File type, always 4D42h ("BM") */
  u32 file_size;         /* Size of the file in bytes */
  u16 reserved_1;        /* Always 0 */
  u16 reserved_2;        /* Always 0 */
  u32 bitmap_offset;     /* Starting position of image data in bytes */
  u32 size;              /* Size of this header in bytes */
  s32 width;             /* Image width in pixels */
  s32 height;            /* Image height in pixels */
  u16 planes;            /* Number of color planes */
  u16 bits_per_pixel;    /* Number of bits per pixel */
  u32 compression;
  u32 size_of_bitmap;
  s32 horizontal_resolution;
  s32 vertical_resolution;
  u32 colors_user;
  u32 colors_important;
};
#pragma pack(pop)

typedef struct Chunk Chunk;
typedef struct Entity Entity;

typedef struct Chunk_pos Chunk_pos;
struct Chunk_pos {
  s32 chunk_x;
  s32 chunk_y;
  s32 chunk_z;
  // nocheckin
  // v2 chunk_offset;
  v3 chunk_offset;
};

struct Chunk {
  Chunk *hash_next;
  Entity *entities[MAX_ENTITIES_PER_CHUNK];
  s32 entities_count;
  s32 chunk_x;
  s32 chunk_y;
  s32 chunk_z;
};

TYPEDEF_ARRAY_NAME(Chunk*, Chunk_ptr_array);

struct Entity {
  Entity *free_list_next;

  Entity_kind    kind;
  Entity_order   order;
  Entity_control control;
  Entity_flags   flags;

  Chunk_pos chunk_pos;

  f32 width;
  f32 height;
  f32 mass;
  f32 friction;
  // nocheckin
  // v2  pos;
  // v2  vel;
  // v2  accel;

  v3  pos;
  v3  vel;
  v3  accel;

  s32 health;
  s32 damage_received;
  s32 damage_to_deal;
  b32 collision_received;

  b32 changed_z;
  f32 blink_red_time;
  f32 red_tint_time;

  f32 monster_attack_delay_time;

  Bitmap bitmap;

  // NOTE jfd: per frame data

};
STATIC_ASSERT(sizeof(Entity) <= 256, entity_is_less_than_256_bytes);
#define ENTITY_SIZE ((u64)256)

TYPEDEF_ARRAY_NAME(Entity*, Entity_ptr_array);

typedef struct Sim_region Sim_region;
struct Sim_region {
  Chunk_pos        origin_chunk_pos;
  v3               size_meters;
  s32              chunks_count;
  s32              entities_count;
  Chunk           *chunks[MAX_SIM_REGION_CHUNKS];
  Entity          *entities[MAX_SIM_REGION_ENTITIES];
};

typedef struct Game Game;
struct Game {
  u32 frame_counter;
  f32 t;
  b32 did_reload;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *temp_arena;

  Game_render_buffer render;
  Game_sound_buffer sound;
  Game_input input;

  s32 test_sound_pitch;

  Chunk_pos camera_chunk_pos;

  b8 once;
  b8 should_init_player;

  u32 world_chunks_x_count;
  u32 world_chunks_y_count;
  u32 world_chunks_z_count;
  Chunk *world_chunks[CHUNK_HASH_TABLE_COUNT];

  s32 random_number_index;

  f32 pixels_per_meter;
  f32 meters_per_pixel;

  s32 tiles_per_room_x;
  s32 tiles_per_room_y;
  s32 tiles_per_room_z;

  Entity entities[MAX_ENTITIES];
  Entity *free_entity;

  Bitmap guy_bitmap;
  Bitmap monster_bitmap;
  Bitmap frog_bitmap;

  b32 camera_jitter;
  f32 camera_jitter_time;

  v3 chunk_size;

};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))


#ifdef GAME_FUNCTION_HEADERS

internal Entity* entity_alloc(Game *gp, Entity_order order, Entity_kind kind, Entity_control control, Entity_flags flags);

internal void entity_free(Game *gp, Entity *ep);

internal Bitmap load_bitmap(Game *gp, Str8 path);

internal u32 get_random(Game *gp);

internal f32 get_random_f32(Game *gp, f32 begin, f32 end, s32 steps);

internal void debug_render_weird_gradient(Game *gp, int x_offset, int y_offset);

force_inline Color alpha_blend(Color bottom, Color top);

force_inline Color color_from_pixel(u32 pixel);

force_inline u32 pixel_from_color(Color color);

internal void draw_rect_min_max(Game *gp, Color color, f32 min_x, f32 min_y, f32 max_x, f32 max_y);

internal void draw_bitmap(Game *gp, Bitmap bitmap, f32 x, f32 y, Color tint);

internal void draw_rect_lines_min_max(Game *gp, Color color, f32 line_thickness, f32 min_x, f32 min_y, f32 max_x, f32 max_y);

internal void clear_screen(Game *gp);

internal void draw_rect(Game *gp, Color color, f32 x, f32 y, f32 width, f32 height);

internal void draw_rect_lines(Game *gp, Color color, f32 line_thickness, f32 x, f32 y, f32 width, f32 height);

internal b32 was_key_pressed_once(Game *gp, Keyboard_key key);

internal b32 is_key_pressed(Game *gp, Keyboard_key key);

internal b32 was_key_released(Game *gp, Keyboard_key key);

internal void debug_silence(Game *gp);

// nocheckin
// internal Chunk_pos chunk_pos_from_point(Game *gp, v2 pos, s32 z);
internal Chunk_pos chunk_pos_from_point(Game *gp, v3 pos);

// nocheckin
// internal v2 point_from_chunk_pos(Game *gp, Chunk_pos chunk_pos);
internal v3 point_from_chunk_pos(Game *gp, Chunk_pos chunk_pos);

force_inline Chunk* get_chunk(Game *gp, s32 chunk_x, s32 chunk_y, s32 chunk_z);

internal void init_player_1(Game *gp);

internal void init_player_2(Game *gp);

internal void init_monster(Game *gp);

internal void init_frog(Game *gp);

internal void init_camera(Game *gp);

internal void create_tile_entity(Game *gp, u32 abs_tile_x, u32 abs_tile_y, u32 abs_tile_z, Entity_kind tile_entity_kind, Entity_flags tile_entity_flags);

internal void init_tile_map(Game *gp);

internal v3_s32 tile_pos_from_sim_pos(Game *gp, v3 pos);

internal void add_entity_to_chunk(Game *gp, Entity *ep);

internal v3 sim_pos_from_chunk_pos(Game *gp, Chunk_pos camera_origin_chunk_pos, Chunk_pos chunk_pos);

internal Chunk_pos chunk_pos_from_sim_pos(Game *gp, Chunk_pos camera_origin_chunk_pos, v3 sim_pos);

internal Sim_region* begin_sim_region(Game *gp, Chunk_pos origin_chunk_pos, v3 size_meters, s32 apron);

internal void end_sim_region(Game *gp, Sim_region* sim_region);

shared_function void game_update_and_render(Game *gp);

shared_function void game_get_sound_samples(Game *gp);

shared_function Game* game_init(Platform *pp);


#endif

#endif

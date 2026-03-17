#ifndef GAME_C
#define GAME_C


/////////////////////////////////
// globals

#include "game_random.h"



/////////////////////////////////
// functions

internal Entity*
func entity_alloc(Game *gp, Entity_order order, Entity_kind kind, Entity_control control, Entity_flags flags) {
  Entity *ep = 0;

  if(gp->free_entity) {
    ep = gp->free_entity;
    sll_stack_pop_n(gp->free_entity, free_list_next);
  } else {
    ep = (Entity*)push_array_no_zero_aligned(gp->main_arena, u8, ENTITY_SIZE, alignof(Entity)); // TODO jfd 16/03/26: add an entity arena
  }

  ASSERT(ep);

  memory_zero(ep, ENTITY_SIZE);

  ep->order   = order;
  ep->kind    = kind;
  ep->control = control;
  ep->flags   = flags;

  return ep;
}

internal void
func entity_free(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_NONE;
}

internal Bitmap
func load_bitmap(Game *gp, Str8 path) {
  Bitmap result = {0};

  char *path_cstr = cstr_from_str8(gp->temp_arena, path);

  Str8 data = platform_read_entire_file(gp->temp_arena, path_cstr);
  Bitmap_header *header = (Bitmap_header*)data.s;
  u32 *pixels = (u32*)(data.s + header->bitmap_offset);
  u32 *result_pixels = push_array_no_zero(gp->main_arena, u32, header->width*header->height);

  for(int y = 0; y < header->height; y++) {
    u32 *write_row = result_pixels + y*header->width;
    u32 *read_row = pixels + (header->height - 1 - y)*header->width;
    memory_copy(write_row, read_row, sizeof(*write_row)*header->width);
  }

  arena_clear(gp->temp_arena);

  result.width = header->width;
  result.height = header->height;
  result.pixels = result_pixels;

  return result;
}

internal u32
func get_random(Game *gp) {
  if(gp->random_number_index >= ARRLEN(game_random_choices)) {
    gp->random_number_index = 0;
  }
  u32 result = game_random_choices[gp->random_number_index++];
  return result;
}

internal f32
func get_random_f32(Game *gp, f32 begin, f32 end, s32 steps) {
  u32 random = get_random(gp);
  random %= steps;
  f32 t = (f32)(steps - random) / steps;
  f32 result = lerp_f32(t, begin, end);
  return result;
}

internal void
func debug_render_weird_gradient(Game *gp, int x_offset, int y_offset) {
  u8 *row = gp->render.pixels;
  for(int y = 0; y < gp->render.height; y++) {
    u32 *pixel = (u32*)row;

    for(int x = 0; x < gp->render.width; x++) {
      //
      // pixel in memory
      //  0  1  2  3
      //  B  G  R  x
      // 00 00 00 00

      u8 r = (u8)(x + x_offset);
      u8 g = 0;
      u8 b = (u8)(y + y_offset);

      // *pixel++ = (r << 16) | (g << 8) | b;
      // *pixel++ = (r << 16) | (b << 8) | g;
      *pixel++ = (r << 8) | (g << 16) | b;
    }

    row += gp->render.stride;
  }

}

force_inline Color
func alpha_blend(Color bottom, Color top) {

  Color final;
  final.a = 1.0f / (top.a + bottom.a*(1.0f - top.a));
  final.r = (top.r*top.a + bottom.r*bottom.a*(1.0f - top.a)) * final.a;
  final.g = (top.g*top.a + bottom.g*bottom.a*(1.0f - top.a)) * final.a;
  final.b = (top.b*top.a + bottom.b*bottom.a*(1.0f - top.a)) * final.a;

  return final;
}

force_inline Color
func color_from_pixel(u32 pixel) {
  // pixel in memory
  //  0  1  2  3
  //  B  G  R  x
  // 00 00 00 00

  f32 color_scale = 1.0f/(f32)MAX_U8;

  Color result = {
    (f32)((pixel & 0xff0000) >> 16) * color_scale,
    (f32)((pixel & 0x00ff00) >> 8) * color_scale,
    (f32)((pixel & 0x0000ff)) * color_scale,
    (f32)((pixel & 0xff000000) >> 24) * color_scale,
  };

  return result;
}

force_inline u32
func pixel_from_color(Color color) {
  // pixel in memory
  //  0  1  2  3
  //  B  G  R  x
  // 00 00 00 00

  u8 a = (u8)(color.a * (f32)MAX_U8);
  u8 r = (u8)(color.r * (f32)MAX_U8);
  u8 g = (u8)(color.g * (f32)MAX_U8);
  u8 b = (u8)(color.b * (f32)MAX_U8);

  u32 result = (a << 24) | (r << 16) | (g << 8) | b;
  return result;
}

internal void
func draw_rect_min_max(Game *gp, Color color, f32 min_x, f32 min_y, f32 max_x, f32 max_y) {

  f32 render_width = (f32)gp->render.width;
  f32 render_height = (f32)gp->render.height;

  f32 x0 = fminf(min_x, max_x);
  f32 x1 = fmaxf(min_x, max_x);
  f32 y0 = fminf(min_y, max_y);
  f32 y1 = fmaxf(min_y, max_y);

  if(x0 < 0.0f) {
    x0 = 0.0f;
  }

  if(x1 >= render_width) {
    x1 = (f32)render_width;
  }

  if(y0 < 0.0f) {
    y0 = 0.0f;
  }

  if(y1 >= render_height) {
    y1 = (f32)render_height;
  }

  int begin_x = (int)floor_f32(fmaxf(0, x0));
  int begin_y = (int)floor_f32(fmaxf(0, y0));
  int end_x   = (int)floor_f32(fminf(x1, (f32)render_width));
  int end_y   = (int)floor_f32(fminf(y1, (f32)render_height));

  u8 *row = gp->render.pixels;

  for(int y = begin_y; y < end_y; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x; x++) {
      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }

}

internal void
func draw_bitmap(Game *gp, Bitmap bitmap, f32 x, f32 y, Color tint) {

  f32 x0 = x;
  f32 y0 = y;
  f32 x1 = x + bitmap.width;
  f32 y1 = y + bitmap.height;

  f32 render_width = (f32)gp->render.width;
  f32 render_height = (f32)gp->render.height;

  s32 source_offset_x = 0;
  if(x0 < 0.0f) {
    source_offset_x = (s32)-x0;
    x0 = 0.0f;
  }

  if(x1 >= render_width) {
    x1 = (f32)render_width;
  }

  s32 source_offset_y = 0;
  if(y0 < 0.0f) {
    source_offset_y = (s32)-y0;
    y0 = 0.0f;
  }

  if(y1 >= render_height) {
    y1 = (f32)render_height;
  }

  int begin_x = (int)floor_f32(fmaxf(0, x0));
  int begin_y = (int)floor_f32(fmaxf(0, y0));
  int end_x   = (int)floor_f32(fminf(x1, (f32)render_width));
  int end_y   = (int)floor_f32(fminf(y1, (f32)render_height));

  u8 *row = gp->render.pixels;
  u32 *bitmap_row = bitmap.pixels;
  bitmap_row += bitmap.width*source_offset_y + source_offset_x;

  for(int cur_y = begin_y, bitmap_y = 0; cur_y < end_y; cur_y++, bitmap_y++) {
    u32 *pixel_row = (u32*)(row + cur_y * gp->render.stride);
    u32 *bitmap_pixel_row = bitmap_row + bitmap_y * bitmap.width;

    for(int cur_x = begin_x, bitmap_x = 0; cur_x < end_x; cur_x++, bitmap_x++) {
      Color cur_color = color_from_pixel(pixel_row[cur_x]);

      Color bitmap_pixel_color = color_from_pixel(bitmap_pixel_row[bitmap_x]);

      Color tinted_color;
      if(cur_color.a > 0.0f && bitmap_pixel_color.a > 0.0f) {
        tinted_color = alpha_blend(bitmap_pixel_color, tint);
      } else {
        tinted_color = bitmap_pixel_color;
      }
      Color final_color = alpha_blend(cur_color, tinted_color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[cur_x] = final_pixel_color;
    }

  }

}

internal void
func draw_rect_lines_min_max(Game *gp, Color color, f32 line_thickness, f32 min_x, f32 min_y, f32 max_x, f32 max_y) {

  f32 render_width = (f32)gp->render.width;
  f32 render_height = (f32)gp->render.height;

  f32 x0 = fminf(min_x, max_x);
  f32 x1 = fmaxf(min_x, max_x);
  f32 y0 = fminf(min_y, max_y);
  f32 y1 = fmaxf(min_y, max_y);

  if(x0 < 0.0f) {
    x0 = 0.0f;
  }

  if(x1 >= render_width) {
    x1 = (f32)render_width;
  }

  if(y0 < 0.0f) {
    y0 = 0.0f;
  }

  if(y1 >= render_height) {
    y1 = (f32)render_height;
  }

  int begin_x = (int)floor_f32(fmaxf(0, x0));
  int begin_y = (int)floor_f32(fmaxf(0, y0));
  int end_x   = (int)floor_f32(fminf(x1, (f32)gp->render.width));
  int end_y   = (int)floor_f32(fminf(y1, (f32)gp->render.height));

  line_thickness = fminf(line_thickness, fminf(x1-x0, y1-y0));

  u8 *row = gp->render.pixels;

  for(int y = begin_y; y < begin_y + (int)floor_f32(line_thickness) && y < gp->render.height; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x && x < gp->render.width; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }


  for(int y = begin_y + (int)floor_f32(line_thickness); y < end_y - (int)floor_f32(line_thickness) && y < gp->render.height; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < begin_x + (int)floor_f32(line_thickness) && x < gp->render.width; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

    for(int x = end_x - (int)floor_f32(line_thickness); x < end_x && x < gp->render.width; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }

  for(int y = end_y - (int)floor_f32(line_thickness); y < end_y && y < gp->render.height; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }


}

internal void
func clear_screen(Game *gp) {
  u64 *pixels = (u64*)gp->render.pixels;
  s32 total_pixels = (gp->render.width * gp->render.height) >> 1;

  for(int i = 0; i < total_pixels; i++) {
    pixels[i] = ((u64)255 << (32 + 24)) | ((u64)255 << 24);
  }

}

internal void
func draw_rect(Game *gp, Color color, f32 x, f32 y, f32 width, f32 height) {

  f32 min_x = x;
  f32 min_y = y;

  f32 max_x = min_x + width;
  f32 max_y = min_y + height;

  draw_rect_min_max(gp, color, min_x, min_y, max_x, max_y);

}

internal void
func draw_rect_lines(Game *gp, Color color, f32 line_thickness, f32 x, f32 y, f32 width, f32 height) {

  f32 min_x = x;
  f32 min_y = y;

  f32 max_x = min_x + width;
  f32 max_y = min_y + height;

  draw_rect_lines_min_max(gp, color, line_thickness, min_x, min_y, max_x, max_y);

}

internal b32
func was_key_pressed_once(Game *gp, Keyboard_key key) {
  return (gp->input.key_pressed[key] == 1);
}

internal b32
func is_key_pressed(Game *gp, Keyboard_key key) {
  return (gp->input.key_pressed[key] > 0);
}

internal b32
func was_key_released(Game *gp, Keyboard_key key) {
  return (gp->input.key_released[key] == true);
}


internal void
func debug_silence(Game *gp) {
  memory_zero(gp->sound.samples, sizeof(gp->sound.samples[0]) * gp->sound.sample_count);
}

internal Chunk_pos
func chunk_pos_from_point(Game *gp, v2 pos, s32 z) {
  Chunk_pos result = {0};

  v2 chunk_pos_float = scale_v2(pos, 1.0f/CHUNK_SIZE_METERS);
  chunk_pos_float.x = round_f32(chunk_pos_float.x);
  chunk_pos_float.y = round_f32(chunk_pos_float.y);

  v2 chunk_offset = sub_v2(pos, scale_v2(chunk_pos_float, CHUNK_SIZE_METERS));

  v2_s32 chunk_pos = cast_v2_f32_to_s32(chunk_pos_float);

  result.chunk_x = chunk_pos.x;
  result.chunk_y = chunk_pos.y;
  result.chunk_z = z;
  result.chunk_offset = chunk_offset;

  return result;
}

internal v2
func point_from_chunk_pos(Game *gp, Chunk_pos chunk_pos) {
  v2 result = chunk_pos.chunk_offset;
  result.x += CHUNK_SIZE_METERS * (f32)chunk_pos.chunk_x;
  result.y += CHUNK_SIZE_METERS * (f32)chunk_pos.chunk_y;
  return result;
}

force_inline Chunk*
func get_chunk(Game *gp, s32 chunk_x, s32 chunk_y, s32 chunk_z) {
  ASSERT(chunk_x > -CHUNK_SAFE_MARGIN);
  ASSERT(chunk_y > -CHUNK_SAFE_MARGIN);
  ASSERT(chunk_z > -CHUNK_SAFE_MARGIN);
  ASSERT(chunk_x <  CHUNK_SAFE_MARGIN);
  ASSERT(chunk_y <  CHUNK_SAFE_MARGIN);
  ASSERT(chunk_z <  CHUNK_SAFE_MARGIN);

  Chunk *result = 0;

  // TODO jfd 10/03/26: replace with a better hash function (never gonna do it)
  u32 hash = 19*chunk_x + 7*chunk_y + 3*chunk_z;
  u32 slot = hash & (CHUNK_HASH_TABLE_COUNT - 1);
  ASSERT(slot < CHUNK_HASH_TABLE_COUNT);

  for(Chunk *chunk = gp->world_chunks[slot]; chunk; chunk = chunk->hash_next) {
    if(chunk->chunk_x == chunk_x && chunk->chunk_y == chunk_y && chunk->chunk_z == chunk_z) {
      result = chunk;
      break;
    }
  }

  if(!result) {
    Chunk *new_chunk = push_struct(gp->main_arena, Chunk);
    Chunk **table = gp->world_chunks;
    // NOTE jfd: this may produce a bug, careful
    sll_stack_push_n(table[slot], new_chunk, hash_next);
    new_chunk->chunk_x = chunk_x;
    new_chunk->chunk_y = chunk_y;
    new_chunk->chunk_z = chunk_z;
    result = new_chunk;
  }

  return result;
}

#if 0
// TODO jfd 06/03/26: line segment intersection
internal Line_segment_intersection
func get_line_segment_intersection(v2 p1, v2 p2, v2 p3, v2 p4) {
  f32 numerator_t = 0, numerator_u = 0, denominator = 0;

  f32 p12x = p1.x - p2.x;
  f32 p12y = p1.y - p2.y;
  f32 p34x = p3.x - p4.x;
  f32 p34y = p3.y - p4.y;
  f32 p13x = p1.x - p3.x;
  f32 p13y = p1.y - p3.y;

  denominator = p12x*p34y - p12y*p34x;
  denominator = 1.0f/denominator;

  numerator_t = p13x * p34y - p13y * p34x;
  numerator_u = p13x * p12y - p13y * p12x;

  f32 t = numerator_t * denominator;
  f32 u = numerator_u * denominator;

  Vector2 contact = (Vector2){ p1.x - t*p12x, p1.y - t*p12y };

  b32 collided = !!((0.0 <= t && t <= 1.0) && (0.0 <= u && u <= 1.0));

  Vector2 delta = { -p12x, -p12y };
  Vector2 dir = {0};
  Vector2 normal = {0};
  b32 segment_is_horizontal = 0;
  b32 segment_is_vertical = 0;

  return result;
}
#endif

internal void
func init_player_1(Game *gp) {

  Entity *ep = entity_alloc(gp,
    ENTITY_ORDER_FIRST,
    ENTITY_KIND_PLAYER_1,
    ENTITY_CONTROL_PLAYER_1,
    ENTITY_FLAG_ACCEL_MOTION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_TILE_COLLISION |
    ENTITY_FLAG_DRAW_BITMAP |
    0
  );

  ep->chunk_pos = chunk_pos_from_point(gp, (v2){ 6, 4 }, 0);
  ep->width = CM(200);
  ep->height = CM(400);
  ep->friction = 19.0f;
  ep->bitmap = gp->guy_bitmap;
  ep->health = 100;

  add_entity_to_chunk(gp, ep);

}

internal void
func init_player_2(Game *gp) {

  Entity *ep = entity_alloc(gp,
    ENTITY_ORDER_FIRST,
    ENTITY_KIND_PLAYER_2,
    ENTITY_CONTROL_PLAYER_2,
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_ACCEL_MOTION |
    ENTITY_FLAG_TILE_COLLISION |
    ENTITY_FLAG_DRAW_BITMAP |
    0
  );

  ep->chunk_pos = chunk_pos_from_point(gp, (v2){ 12, 14 }, 0);
  ep->width = CM(200);
  ep->height = CM(400);
  ep->friction = 19.0f;
  ep->bitmap = gp->guy_bitmap;
  ep->health = 100;

  add_entity_to_chunk(gp, ep);

}

internal void
func init_monster(Game *gp) {

  Entity *ep = entity_alloc(gp,
    ENTITY_ORDER_SECOND,
    ENTITY_KIND_MONSTER,
    ENTITY_CONTROL_MONSTER_FOLLOW_AND_ATTACK,
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_ACCEL_MOTION |
    ENTITY_FLAG_TILE_COLLISION |
    ENTITY_FLAG_APPLY_DAMAGE |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DRAW_BITMAP |
    0
  );

  // ep->chunk_pos = chunk_pos_from_point(gp, (v2){150.755142211914062, 65.5780410766601563}, 0);
  ep->chunk_pos = chunk_pos_from_point(gp, (v2){ 16, 14 }, 0);
  ep->width = CM(200);
  ep->height = CM(400);
  ep->friction = 19.0f;
  ep->bitmap = gp->monster_bitmap;
  ep->health = 100;

  add_entity_to_chunk(gp, ep);
}

internal void
func init_frog(Game *gp) {

  Entity *ep = entity_alloc(gp,
    ENTITY_ORDER_SECOND,
    ENTITY_KIND_FROG,
    ENTITY_CONTROL_FROG_FOLLOW,
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_ACCEL_MOTION |
    ENTITY_FLAG_TILE_COLLISION |
    ENTITY_FLAG_DRAW_BITMAP |
    0
  );

  ep->chunk_pos = chunk_pos_from_point(gp, (v2){ 22, 14 }, 0);
  ep->width = CM(200);
  ep->height = CM(400);
  ep->friction = 4.0f;
  ep->bitmap = gp->frog_bitmap;
  ep->health = 100;

  add_entity_to_chunk(gp, ep);
}

internal void
func init_camera(Game *gp) {

  v2 camera_pos = {
    (f32)gp->tiles_per_room_width,
    (f32)gp->tiles_per_room_height,
  };
  camera_pos = scale_v2(camera_pos, 0.5f*TILE_SIZE_METERS);

  gp->camera_chunk_pos = chunk_pos_from_point(gp, camera_pos, 0);

}

internal void
func create_tile_entity(Game *gp, u32 abs_tile_x, u32 abs_tile_y, u32 abs_tile_z, Entity_kind tile_entity_kind, Entity_flags tile_entity_flags) {
  Entity *ep = entity_alloc(gp,
    ENTITY_ORDER_LAST,
    tile_entity_kind,
    ENTITY_CONTROL_NONE,
    tile_entity_flags |
    0
  );

  v2 pos = { (f32)abs_tile_x, (f32)abs_tile_y };
  pos = scale_v2(pos, TILE_SIZE_METERS);
  pos = add_value_v2(pos, 0.5f*TILE_SIZE_METERS);
  ep->chunk_pos = chunk_pos_from_point(gp, pos, abs_tile_z);

  add_entity_to_chunk(gp, ep);

}

internal void
func init_tile_map(Game *gp) {

  u32 screen_x = 0;
  u32 screen_y = 0;

  gp->tiles_per_room_width  = 35;
  gp->tiles_per_room_height = 20;

  u32 tiles_per_width  = gp->tiles_per_room_width;
  u32 tiles_per_height = gp->tiles_per_room_height;

  u32 abs_tile_z = 0;

  b32 door_top    = false;
  b32 door_bottom = false;
  b32 door_left   = false;
  b32 door_right  = false;
  b32 door_up     = false;
  b32 door_down   = false;

  for(u32 screen_index = 0; screen_index < 100; screen_index++) {

    u32 random_choice;

    if(door_up || door_down) {
      random_choice = get_random(gp) % 2;
    } else {
      random_choice = get_random(gp) % 3;
    }

    b32 created_z_door = false;

    if(random_choice == 2) {

      created_z_door = true;

      if(abs_tile_z == 0) {
        door_up = true;
      } else {
        door_down = true;
      }

    } else if(random_choice == 1) {
      door_top = true;
    } else {
      door_right = true;
    }

    u8 *neighbour_grid = push_array(gp->temp_arena, u8, tiles_per_height*tiles_per_width);

    for(u32 tile_y = 0; tile_y < tiles_per_height; tile_y++) {
      for(u32 tile_x = 0; tile_x < tiles_per_width; tile_x++) {

        Entity_kind tile_entity_kind = 0;

        if((tile_x == 0) && (!door_left || (tile_y != (tiles_per_height / 2)))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if((tile_x == (tiles_per_width - 1) && (!door_right || (tile_y != (tiles_per_height / 2))))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if((tile_y == 0) && (!door_bottom || (tile_x != (tiles_per_width / 2)))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if((tile_y == (tiles_per_height - 1) && (!door_top || (tile_x != (tiles_per_width / 2))))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if(tile_entity_kind == ENTITY_KIND_WALL) {
          neighbour_grid[tile_y*tiles_per_width + tile_x] = 1;
        }

      }
    }

    for(u32 tile_y = 0; tile_y < tiles_per_height; tile_y++) {
      for(u32 tile_x = 0; tile_x < tiles_per_width; tile_x++) {
        u32 abs_tile_x = screen_x*tiles_per_width + tile_x;
        u32 abs_tile_y = screen_y*tiles_per_height + tile_y;

        Entity_kind tile_entity_kind = 0;

        if((tile_x == 0) && (!door_left || (tile_y != (tiles_per_height / 2)))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if((tile_x == (tiles_per_width - 1) && (!door_right || (tile_y != (tiles_per_height / 2))))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if((tile_y == 0) && (!door_bottom || (tile_x != (tiles_per_width / 2)))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if((tile_y == (tiles_per_height - 1) && (!door_top || (tile_x != (tiles_per_width / 2))))) {
          tile_entity_kind = ENTITY_KIND_WALL;
        }

        if(tile_x == 10 && tile_y == 6) {
          if(door_up && abs_tile_z < gp->world_chunks_z_count - 1) {
            tile_entity_kind = ENTITY_KIND_DOOR_UP;
          }
          if(door_down && abs_tile_z > 0) {
            tile_entity_kind = ENTITY_KIND_DOOR_DOWN;
          }
        }

        if(tile_entity_kind != 0) {

          Entity_flags tile_entity_flags = 0;

          if(tile_entity_kind == ENTITY_KIND_WALL) {
            neighbour_grid[tile_y*tiles_per_width + tile_x] = 1;

            u8 top_tile_value    = 0;
            u8 right_tile_value  = 0;
            u8 bottom_tile_value = 0;
            u8 left_tile_value   = 0;

            if(tile_x == 0) {
              left_tile_value = 0;
              right_tile_value = neighbour_grid[tile_y*tiles_per_width + tile_x + 1];
            } else if(tile_x == tiles_per_width - 1) {
              right_tile_value = 0;
              left_tile_value = neighbour_grid[tile_y*tiles_per_width + tile_x - 1];
            } else {
              right_tile_value = neighbour_grid[tile_y*tiles_per_width + tile_x + 1];
              left_tile_value = neighbour_grid[tile_y*tiles_per_width + tile_x - 1];
            }

            if(tile_y == 0) {
              bottom_tile_value = 0;
              top_tile_value = neighbour_grid[(tile_y + 1)*tiles_per_width + tile_x];
            } else if(tile_y == tiles_per_height - 1) {
              top_tile_value = 0;
              bottom_tile_value = neighbour_grid[(tile_y - 1)*tiles_per_width + tile_x];
            } else {
              top_tile_value = neighbour_grid[(tile_y + 1)*tiles_per_width + tile_x];
              bottom_tile_value = neighbour_grid[(tile_y - 1)*tiles_per_width + tile_x];
            }

            b32 top_enabled    = (top_tile_value    != 1);
            b32 right_enabled  = (right_tile_value  != 1);
            b32 bottom_enabled = (bottom_tile_value != 1);
            b32 left_enabled   = (left_tile_value   != 1);

            b32 top_left_corner_enabled     = (!left_enabled && !top_enabled);
            b32 top_right_corner_enabled    = (!right_enabled && !top_enabled);
            b32 bottom_right_corner_enabled = (!right_enabled && !bottom_enabled);
            b32 bottom_left_corner_enabled  = (!left_enabled && !bottom_enabled);

            tile_entity_flags =
            (top_enabled                 << ENTITY_FLAG_INDEX_TILE_TOP_ENABLED) |
            (right_enabled               << ENTITY_FLAG_INDEX_TILE_RIGHT_ENABLED) |
            (bottom_enabled              << ENTITY_FLAG_INDEX_TILE_BOTTOM_ENABLED) |
            (left_enabled                << ENTITY_FLAG_INDEX_TILE_LEFT_ENABLED) |
            (top_left_corner_enabled     << ENTITY_FLAG_INDEX_TILE_TOP_LEFT_CORNER_ENABLED) |
            (top_right_corner_enabled    << ENTITY_FLAG_INDEX_TILE_TOP_RIGHT_CORNER_ENABLED) |
            (bottom_right_corner_enabled << ENTITY_FLAG_INDEX_TILE_BOTTOM_RIGHT_CORNER_ENABLED) |
            (bottom_left_corner_enabled  << ENTITY_FLAG_INDEX_TILE_BOTTOM_LEFT_CORNER_ENABLED) |
            0;

          }

          create_tile_entity(gp, abs_tile_x, abs_tile_y, abs_tile_z, tile_entity_kind, tile_entity_flags);
        }

      }
    }

    door_bottom = door_top;
    door_left   = door_right;

    if(created_z_door) {
      door_down = !door_down;
      door_up = !door_up;
    } else {
      door_down = false;
      door_up   = false;
    }

    door_top    = false;
    door_right  = false;

    if(random_choice == 2) {
      abs_tile_z = !abs_tile_z;
    } else if(random_choice == 1) {
      screen_y += 1;
    } else {
      screen_x += 1;
    }

  }

}

internal v2_s32
func tile_pos_from_screen_pos(Game *gp, v2 pos) {
  v2_s32 result = {0};

  v2 abs_camera_pos = gp->camera_chunk_pos.chunk_offset;
  abs_camera_pos =
  add_v2(abs_camera_pos, scale_v2((v2){ (f32)gp->camera_chunk_pos.chunk_x, (f32)gp->camera_chunk_pos.chunk_y }, CHUNK_SIZE_METERS));

  v2 abs_pos = add_v2(pos, abs_camera_pos);

  result = cast_v2_f32_to_s32((scale_v2(abs_pos, 1.0f/TILE_SIZE_METERS)));

  return result;
}

internal void
func add_entity_to_chunk(Game *gp, Entity *ep) {

  Chunk_pos entity_chunk_pos = ep->chunk_pos;
  Chunk *chunk = get_chunk(gp, entity_chunk_pos.chunk_x, entity_chunk_pos.chunk_y, entity_chunk_pos.chunk_z);

  if(chunk->entities_count < ARRLEN(chunk->entities)) {
    chunk->entities[chunk->entities_count++] = ep;
  } else {
    PANIC("too many entities in chunk");
  }

}

internal v2
func screen_pos_from_chunk_pos(Game *gp, Chunk_pos camera_origin_chunk_pos, Chunk_pos chunk_pos) {
  v2 result = {0};

  // TODO jfd 13/03/26: make positions full 3d
  v2_s32 camera_chunk = { camera_origin_chunk_pos.chunk_x, camera_origin_chunk_pos.chunk_y };
  v2_s32 cur_chunk = { chunk_pos.chunk_x, chunk_pos.chunk_y };
  v2 camera_chunk_offset = camera_origin_chunk_pos.chunk_offset;
  v2 cur_chunk_offset = chunk_pos.chunk_offset;

  cur_chunk = sub_v2_s32(cur_chunk, camera_chunk);
  cur_chunk_offset = sub_v2(cur_chunk_offset, camera_chunk_offset);

  result = add_v2(
    cur_chunk_offset,
    scale_v2(
      cast_v2_s32_to_f32(cur_chunk),
      CHUNK_SIZE_METERS
    )
  );

  return result;
}

internal Chunk_pos
func chunk_pos_from_screen_pos(Game *gp, Chunk_pos camera_origin_chunk_pos, v2 screen_pos, s32 z) {
  Chunk_pos result = {0};

  v2 chunk_pos_float = scale_v2(screen_pos, 1.0f/CHUNK_SIZE_METERS);
  chunk_pos_float.x = round_f32(chunk_pos_float.x);
  chunk_pos_float.y = round_f32(chunk_pos_float.y);

  v2 chunk_offset = camera_origin_chunk_pos.chunk_offset;
  chunk_offset = add_v2(chunk_offset, sub_v2(screen_pos, scale_v2(chunk_pos_float, CHUNK_SIZE_METERS)));

  v2_s32 chunk_pos = cast_v2_f32_to_s32(chunk_pos_float);
  chunk_pos.x += (s32)camera_origin_chunk_pos.chunk_x;
  chunk_pos.y += (s32)camera_origin_chunk_pos.chunk_y;

  result.chunk_x = chunk_pos.x;
  result.chunk_y = chunk_pos.y;
  result.chunk_z = z;
  result.chunk_offset = chunk_offset;

  return result;
}

internal Sim_region*
func begin_sim_region(Game *gp, Chunk_pos origin_chunk_pos, f32 width, f32 height, s32 apron) {
  Sim_region *result = push_struct(gp->frame_arena, Sim_region);

  result->origin_chunk_pos = origin_chunk_pos;
  result->width = width;
  result->height = height;

  // TODO jfd 12/03/26: flags on arrays to say they can't grow

  v2 viewport_bottom_left_corner = { width, height };
  viewport_bottom_left_corner = scale_v2(viewport_bottom_left_corner, -0.5f);

  Chunk_pos viewport_bottom_left_corner_chunk_pos = chunk_pos_from_screen_pos(gp, gp->camera_chunk_pos, viewport_bottom_left_corner, origin_chunk_pos.chunk_z);

  s32 chunk_window_x_apron = apron;
  s32 chunk_window_y_apron = apron;

  s32 chunk_window_width  = chunk_window_x_apron + ((origin_chunk_pos.chunk_x - viewport_bottom_left_corner_chunk_pos.chunk_x) << 1);
  s32 chunk_window_height = chunk_window_y_apron + ((origin_chunk_pos.chunk_y - viewport_bottom_left_corner_chunk_pos.chunk_y) << 1);

  /* NOTE jfd 17/03/26: stream entities in to live set */

  for(s32 chunk_window_y = -chunk_window_y_apron; chunk_window_y <= chunk_window_height; chunk_window_y++) {
    for(s32 chunk_window_x = -chunk_window_x_apron; chunk_window_x < chunk_window_width; chunk_window_x++) {

      Chunk_pos cur_chunk_pos = viewport_bottom_left_corner_chunk_pos;
      cur_chunk_pos.chunk_x += chunk_window_x;
      cur_chunk_pos.chunk_y += chunk_window_y;

      Chunk *chunk = get_chunk(gp, cur_chunk_pos.chunk_x, cur_chunk_pos.chunk_y, cur_chunk_pos.chunk_z);

      if(result->chunks_count < ARRLEN(result->chunks)) {
        result->chunks[result->chunks_count++] = chunk;
      } else {
        UNREACHABLE;
      }

    }
  }

  // NOTE jfd 12/03/26:
  // - save pointers to them in an array here for later (????)
  // - load entities from those chunks
  // - store the current chunk and the position the entity is at in that chunk
  // - on the entity during the frame so that we can easily evict it from that chunk later
  // - maybe set the camera relative positions as you load them?

  for(int i = 0; i < result->chunks_count; i++) {
    Chunk *chunk = result->chunks[i];
    for(int j = 0; j < chunk->entities_count; j++) {
      Entity *ep = chunk->entities[j];
      ep->pos = screen_pos_from_chunk_pos(gp, result->origin_chunk_pos, ep->chunk_pos);
      if(result->entities_count < ARRLEN(result->entities)) {
        result->entities[result->entities_count++] = ep;
      } else {
        UNREACHABLE;
      }
    }
    chunk->entities_count = 0;
  }

  return result;
}

internal void
func end_sim_region(Game *gp, Sim_region *sim_region) {
  // NOTE jfd 12/03/26: put all simulated entities in to the new chunks they should occupy according to their screen pos

  for(int entity_index = 0; entity_index < sim_region->entities_count; entity_index++) {
    Entity *ep = sim_region->entities[entity_index];

    if(ep->flags & ENTITY_FLAG_DIE_NOW) {
      ep->free_list_next = gp->free_entity;
      gp->free_entity = ep;
    } else {
      add_entity_to_chunk(gp, ep);
    }

  }

}


shared_function void
func game_update_and_render(Game *gp) {

  f32 t = gp->t;

  if(gp->did_reload) {
    gp->did_reload = false;
    gp->once = false;
  }

  if(is_key_pressed(gp, KBD_KEY_F5)) {
    memory_zero(gp->entities, sizeof(gp->entities));
    init_camera(gp);
    gp->once = true;
  }

  if(gp->once) {
    gp->once = false;

    gp->should_init_player = true;

  }


  { /* run_once */


    if(gp->should_init_player) {
      gp->should_init_player = false;

      init_player_1(gp);
      init_player_2(gp);
    }

  } /* run_once */

  b32 update_camera_location = false;
  Chunk_pos new_camera_chunk_pos = {0};

  s32 camera_viewport_width_tiles = gp->tiles_per_room_width;
  s32 camera_viewport_height_tiles = gp->tiles_per_room_height;

  f32 sim_region_width = gp->meters_per_pixel*gp->render.width;
  f32 sim_region_height = gp->meters_per_pixel*gp->render.height;

  Sim_region *sim_region = begin_sim_region(gp, gp->camera_chunk_pos, sim_region_width, sim_region_height, 2);

  for(Entity_order entity_order = 0; entity_order < ENTITY_ORDER_MAX; entity_order++) {
    for(int entity_index = 0; entity_index < sim_region->entities_count; entity_index++)
    { /* update entities */

      Entity *ep = sim_region->entities[entity_index];

      if(ep->kind == ENTITY_KIND_NONE) {
        continue;
      }

      if(ep->order != entity_order) {
        continue;
      }

      v2 cur_pos = ep->pos;
      v2 new_pos = cur_pos;

      v2 delta_pos = {0};

      switch(ep->control) {
        case ENTITY_CONTROL_PLAYER_1:
        case ENTITY_CONTROL_PLAYER_2:
        {

          { /* get keyboard and mouse input */

            // TODO jfd: mouse movement and clicks
            f32 player_accel = PLAYER_ACCEL;

            ep->accel = (v2){0};

            if(ep->control == ENTITY_CONTROL_PLAYER_1) {
              if(is_key_pressed(gp, KBD_KEY_W)) {
                ep->accel.y += player_accel;
              }
              if(is_key_pressed(gp, KBD_KEY_A)) {
                ep->accel.x -= player_accel;
              }
              if(is_key_pressed(gp, KBD_KEY_S)) {
                ep->accel.y -= player_accel;
              }
              if(is_key_pressed(gp, KBD_KEY_D)) {
                ep->accel.x += player_accel;
              }
            } else {
              if(is_key_pressed(gp, KBD_KEY_UP_ARROW)) {
                ep->accel.y += player_accel;
              }
              if(is_key_pressed(gp, KBD_KEY_LEFT_ARROW)) {
                ep->accel.x -= player_accel;
              }
              if(is_key_pressed(gp, KBD_KEY_DOWN_ARROW)) {
                ep->accel.y -= player_accel;
              }
              if(is_key_pressed(gp, KBD_KEY_RIGHT_ARROW)) {
                ep->accel.x += player_accel;
              }
            }

          } /* get keyboard and mouse input */

          if(is_key_pressed(gp, KBD_KEY_LEFT_SHIFT)) {
            ep->flags |= ENTITY_FLAG_SLOW;
          } else {
            ep->flags &= ~ENTITY_FLAG_SLOW;
          }

          if(ep->damage_received > 0) {
            gp->camera_jitter = true;
            gp->camera_jitter_time = 0.5f;
          }

        } break;

        case ENTITY_CONTROL_FROG_FOLLOW: {

          Entity *player_1_ep = 0;
          for(int i = 0; i < sim_region->entities_count; i++) {
            Entity *cur_ep = sim_region->entities[i];
            if(cur_ep->kind == ENTITY_KIND_PLAYER_1) {
              player_1_ep = cur_ep;
              break;
            }
          }

          if(player_1_ep) {

            v2 delta = sub_v2(player_1_ep->pos, ep->pos);
            f32 dist_sq = dot_v2(delta, delta);
            if(dist_sq <= SQUARE(6.0f)) {
              ep->accel = (v2){0};
            } else {
              ep->accel = scale_v2(norm_v2(delta), FROG_ACCEL);
            }

          }

        } break;

        case ENTITY_CONTROL_MONSTER_FOLLOW_AND_ATTACK: {

          Entity *player_1_ep = 0;
          for(int i = 0; i < sim_region->entities_count; i++) {
            Entity *cur_ep = sim_region->entities[i];
            if(cur_ep->kind == ENTITY_KIND_PLAYER_1) {
              player_1_ep = cur_ep;
              break;
            }
          }

          if(player_1_ep) {
            v2 delta = sub_v2(player_1_ep->pos, ep->pos);
            f32 dist_sq = dot_v2(delta, delta);
            if(dist_sq < SQUARE(25.0f) ) {
              ep->accel = scale_v2(norm_v2(delta), MONSTER_ACCEL);
            } else {
              ep->accel = (v2){0};
            }

            if(dist_sq < SQUARE(4.0f)) {
              if(ep->monster_attack_delay_time <= 0.0f) {
                ep->monster_attack_delay_time = 1.8f;
                player_1_ep->damage_received += MONSTER_ATTACK_DAMAGE;
              } else {
                ep->monster_attack_delay_time -= gp->t;
              }
            } else {
              ep->monster_attack_delay_time = 0;
            }
          }

        } break;

      }

      /* handle entity flags */

      if(ep->flags & ENTITY_FLAG_APPLY_FRICTION) {
        ep->vel = sub_v2(ep->vel, scale_v2(ep->vel, ep->friction*t));
      }

      if(ep->flags & ENTITY_FLAG_ACCEL_MOTION) {

        // v = v0 + at
        // s = s0 + vt - v*drag*t (at^2) / 2

        ep->vel = add_v2(ep->vel, scale_v2(ep->accel, t));


        delta_pos = add_v2(scale_v2(ep->vel, t), scale_v2(ep->accel, t*t*0.5f));

        if(ep->flags & ENTITY_FLAG_SLOW) {
          delta_pos = scale_v2(delta_pos, 1.0e-1f);
        }

        new_pos = add_v2(cur_pos, delta_pos);

        // dp -= 2*dot(dp, normal_vector)*normal_vector

      }

      if(ep->flags & ENTITY_FLAG_TILE_COLLISION) {

        v2_s32 cur_tile_pos = tile_pos_from_screen_pos(gp, cur_pos);
        v2_s32 new_tile_pos = tile_pos_from_screen_pos(gp, new_pos);

        s32 step_in_z = 0;
        b32 didnt_touch_any_tiles = true;

        for(int i = 0; i < sim_region->entities_count; i++) {
          Entity *tile_ep = sim_region->entities[i];

          if(tile_ep == ep) {
            continue;
          }

          if(tile_ep->kind != ENTITY_KIND_WALL && tile_ep->kind != ENTITY_KIND_DOOR_UP && tile_ep->kind != ENTITY_KIND_DOOR_DOWN) {
            continue;
          }

          v2_s32 tile_entity_tile_pos = tile_pos_from_screen_pos(gp, tile_ep->pos);

          if(new_tile_pos.x == tile_entity_tile_pos.x && new_tile_pos.y == tile_entity_tile_pos.y) {

            didnt_touch_any_tiles = false;

            if(tile_ep->kind == ENTITY_KIND_WALL) {

              v2 normal = {0};

              s32 tile_dx = new_tile_pos.x - cur_tile_pos.x;
              s32 tile_dy = new_tile_pos.y - cur_tile_pos.y;

              if(tile_dx > 0 && (tile_ep->flags & ENTITY_FLAG_TILE_LEFT_ENABLED)) {
                normal.x = -1;
              }
              if(tile_dx < 0 && (tile_ep->flags & ENTITY_FLAG_TILE_RIGHT_ENABLED)) {
                normal.x = 1;
              }
              if(tile_dy > 0 && (tile_ep->flags & ENTITY_FLAG_TILE_BOTTOM_ENABLED)) {
                normal.y = -1;
              }
              if(tile_dy < 0 && (tile_ep->flags & ENTITY_FLAG_TILE_TOP_ENABLED)) {
                normal.y = 1;
              }

              if(tile_dx > 0 && tile_dy > 0 && (tile_ep->flags & ENTITY_FLAG_TILE_BOTTOM_LEFT_CORNER_ENABLED)) {
                normal = (v2){ -1, -1 };
              }
              if(tile_dx < 0 && tile_dy > 0 && (tile_ep->flags & ENTITY_FLAG_TILE_BOTTOM_RIGHT_CORNER_ENABLED)) {
                normal = (v2){ 1, -1 };
              }
              if(tile_dx > 0 && tile_dy < 0 && (tile_ep->flags & ENTITY_FLAG_TILE_TOP_LEFT_CORNER_ENABLED)) {
                normal = (v2){ -1, 1 };
              }
              if(tile_dx < 0 && tile_dy < 0 && (tile_ep->flags & ENTITY_FLAG_TILE_TOP_RIGHT_CORNER_ENABLED)) {
                normal = (v2){ 1, 1 };
              }

              if(normal.x != 0.0f && normal.y != 0.0f) {
                // normal = scale_v2(normal, 0.70710678118654752440084436210485f);
                normal = scale_v2(normal, 0.9f);
              }

              delta_pos = sub_v2(delta_pos, scale_v2(normal, 1.f*dot_v2(delta_pos, normal)));

              new_pos = add_v2(cur_pos, delta_pos);

              ep->changed_z = false;

            } else if(tile_ep->kind == ENTITY_KIND_DOOR_UP) {
              if(!ep->changed_z) {
                ep->changed_z = true;
                step_in_z = 1;
              }
            } else if(tile_ep->kind == ENTITY_KIND_DOOR_DOWN) {
              if(!ep->changed_z) {
                ep->changed_z = true;
                step_in_z = -1;
              }
            }

            break;
          }
        }

        if(didnt_touch_any_tiles) {
          ep->changed_z = false;
        }

        ep->pos = new_pos;

        ep->chunk_pos = chunk_pos_from_screen_pos(gp, gp->camera_chunk_pos, ep->pos, ep->chunk_pos.chunk_z + step_in_z);

      } /* ENTITY_FLAG_TILE_COLLISION */

      if(ep->flags & ENTITY_FLAG_BLINK_RED) {
        ep->blink_red_time -= gp->t;

        if(ep->blink_red_time <= 0.0f) {
          ep->flags &= ~ENTITY_FLAG_BLINK_RED;
          ep->flags &= ~ENTITY_FLAG_DRAW_RED_TINT;
        } else {
          ep->red_tint_time -= gp->t;
          if(ep->red_tint_time <= 0.0f) {
            ep->flags ^= ENTITY_FLAG_DRAW_RED_TINT;
            ep->red_tint_time = 0.001f;
          }
        }

      }

      if(!(ep->flags & ENTITY_FLAG_IGNORE_DAMAGE)) {

        if(ep->damage_received > 0) {
          ep->health -= ep->damage_received;
          ep->damage_received = 0;

          ep->flags |= ENTITY_FLAG_BLINK_RED;
          ep->flags |= ENTITY_FLAG_DRAW_RED_TINT;
          ep->blink_red_time = 1.0f;
          ep->red_tint_time = 0.01f;

          if(ep->health <= 0) {
            ep->flags |= ENTITY_FLAG_DIE_NOW;
          }
        }

      }



      if(ep->flags & ENTITY_FLAG_DIE_NOW) {
      }

      if(ep->kind == ENTITY_KIND_PLAYER_1) {

        v2 new_camera_screen_pos = {0};

        if(ep->pos.x < -0.5f*TILE_SIZE_METERS*camera_viewport_width_tiles) {
          new_camera_screen_pos.x -= TILE_SIZE_METERS*camera_viewport_width_tiles;
        }

        if(ep->pos.x > 0.5f*TILE_SIZE_METERS*camera_viewport_width_tiles) {
          new_camera_screen_pos.x += TILE_SIZE_METERS*camera_viewport_width_tiles;
        }

        if(ep->pos.y < -0.5f*TILE_SIZE_METERS*camera_viewport_height_tiles) {
          new_camera_screen_pos.y -= TILE_SIZE_METERS*camera_viewport_height_tiles;
        }

        if(ep->pos.y > 0.5f*TILE_SIZE_METERS*camera_viewport_height_tiles) {
          new_camera_screen_pos.y += TILE_SIZE_METERS*camera_viewport_height_tiles;
        }

        s32 new_camera_chunk_pos_z = ep->chunk_pos.chunk_z;
        new_camera_chunk_pos = chunk_pos_from_screen_pos(gp, gp->camera_chunk_pos, new_camera_screen_pos, new_camera_chunk_pos_z);
        update_camera_location = true;

      }

    } /* update entities */
  }


  { /* camera control */

    gp->pixels_per_meter -= gp->input.scroll_delta.y * 0.01f;

    if(gp->pixels_per_meter < MIN_PIXELS_PER_METER) {
      gp->pixels_per_meter = MIN_PIXELS_PER_METER;
    }

    if(gp->pixels_per_meter > MAX_PIXELS_PER_METER) {
      gp->pixels_per_meter = MAX_PIXELS_PER_METER;
    }

    if(is_key_pressed(gp, KBD_KEY_LEFT_CONTROL) && is_key_pressed(gp, KBD_KEY_0)) {
      gp->pixels_per_meter = PIXELS_PER_METER;
    }

    if(gp->pixels_per_meter != 0.0f) {
      gp->meters_per_pixel = 1.0f / gp->pixels_per_meter;
    }

  } /* camera control */


  { /* draw */

    clear_screen(gp);

    v2 camera_offset = { 0.5f*(f32)gp->render.width, 0.5f*(f32)gp->render.height};

    if(gp->camera_jitter) {

      // TODO jfd 17/03/26: make the camera jitter based on the direction that the attack came from
      f32 x = get_random_f32(gp, -2.0f, 2.0f, 4);
      f32 y = get_random_f32(gp, -2.0f, 2.0f, 4);

      v2 v = { x, y };
      camera_offset = add_v2(camera_offset, v);

      gp->camera_jitter_time -= gp->t;
      if(gp->camera_jitter_time <= 0.0f) {
        gp->camera_jitter = false;
        gp->camera_jitter_time = 0.0f;
      }

    }

    for(int entity_index = 0; entity_index < sim_region->entities_count; entity_index++)
    { /* draw tile entities */

      Entity *ep = sim_region->entities[entity_index];

      if(ep->kind == ENTITY_KIND_NONE) {
        continue;
      }

      switch(ep->kind) {
        default:
        break;

        case ENTITY_KIND_WALL:
        case ENTITY_KIND_DOOR_UP:
        case ENTITY_KIND_DOOR_DOWN:
        {
          Color color = { 0.8f,  0.8f, 0.8f, 1 };
          if(ep->kind == ENTITY_KIND_DOOR_UP) {
            color = (Color){ 0.0f,  0.8f, 0.4f, 1 };
          } else if(ep->kind == ENTITY_KIND_DOOR_DOWN) {
            color = (Color){ 0.96f, 1.0f, 0.0f, 1 };
          }

          v2 tile_screen_pos = scale_v2(ep->pos, gp->pixels_per_meter);
          tile_screen_pos = add_v2(tile_screen_pos, camera_offset);
          tile_screen_pos.y = gp->render.height - tile_screen_pos.y;

          v2 tile_screen_rect_size = scale_v2((v2){ TILE_SIZE_METERS, TILE_SIZE_METERS }, gp->pixels_per_meter);
          v2 half_tile_screen_rect_size = scale_v2(tile_screen_rect_size, 0.5f);
          v2 tile_screen_rect_pos = sub_v2(tile_screen_pos, half_tile_screen_rect_size);

          draw_rect(gp,
            color,
            tile_screen_rect_pos.x,
            tile_screen_rect_pos.y,
            tile_screen_rect_size.x,
            tile_screen_rect_size.y
          );

          #if 0
          Color yellow = { 1.0f, 1.0f, 0.0f, 0.5f };
          draw_rect(gp,
            yellow,
            tile_screen_pos.x- 2.0f,
            tile_screen_pos.y- 2.0f,
            4.0f,
            4.0f
          );
          #endif

        } break;
      }
    } /* draw tile entities */

    for(int entity_index = 0; entity_index < sim_region->entities_count; entity_index++)
    { /* draw entities */

      Entity *ep = sim_region->entities[entity_index];

      if(ep->kind == ENTITY_KIND_NONE) {
        continue;
      }

      if(ep->flags & ENTITY_FLAG_DRAW_BITMAP) {
        v2 entity_screen_pos = scale_v2(ep->pos, gp->pixels_per_meter);
        entity_screen_pos = add_v2(entity_screen_pos, camera_offset);
        entity_screen_pos.y = gp->render.height - entity_screen_pos.y;

        Color tint = {0};
        if(ep->flags & ENTITY_FLAG_DRAW_RED_TINT) {
          tint = (Color){ 1.0f, 0.0f, 0.0f, 0.5f };
        }

        #if 1
        {
          f32 entity_bitmap_x = entity_screen_pos.x - 0.5f*(f32)ep->bitmap.width;
          f32 entity_bitmap_y = entity_screen_pos.y - 0.5f*(f32)ep->bitmap.height;
          draw_bitmap(gp, ep->bitmap, entity_bitmap_x, entity_bitmap_y, tint);
        }
        #endif

        #if 0

        v2 entity_rect_screen_size        = scale_v2((v2){ ep->width, ep->height }, gp->pixels_per_meter);
        v2 half_entity_screen_size        = scale_v2(entity_rect_screen_size, 0.5f);
        v2 entity_rect_screen_pos         = sub_v2(entity_screen_pos, half_entity_screen_size);
        v2 entity_center_rect_screen_pos  = sub_v2(entity_screen_pos, scale_v2(half_entity_screen_size, 0.2f));
        v2 entity_center_rect_screen_size = scale_v2(entity_rect_screen_size, 0.2f);

        draw_rect(gp,
          (Color){ 0.95f, 0.2f, 0.4f, 0.5f },
          entity_rect_screen_pos.x,
          entity_rect_screen_pos.y,
          entity_rect_screen_size.x,
          entity_rect_screen_size.y
        );

        draw_rect(gp,
          (Color){ 0.95f, 0.8f, 0.0f, 0.5f },
          entity_center_rect_screen_pos.x,
          entity_center_rect_screen_pos.y,
          entity_center_rect_screen_size.x,
          entity_center_rect_screen_size.y
        );

        #endif
      }

    } /* draw entities */

    if(update_camera_location) {
      update_camera_location = false;
      gp->camera_chunk_pos = new_camera_chunk_pos;
    }


    #if 0
    {
      Color color = { 1.0f, 0, 0, 1 };

      draw_rect(gp,
        color,
        0.5f*(f32)gp->render.width,
        0,
        1,
        (f32)gp->render.height
      );

      draw_rect(gp,
        color,
        0,
        0.5f*(f32)gp->render.height,
        (f32)gp->render.width,
        1
      );

    }
    #endif

  } /* draw */

  end_sim_region(gp, sim_region);

  arena_clear(gp->frame_arena);

  gp->frame_counter++;

}

shared_function void
func game_get_sound_samples(Game *gp) {

  // TODO jfd: allow sample offsets here for more robust platform options

  debug_silence(gp);
  // TODO jfd 23/02/26: there is some popping in the sound, not sure if it has to do with hot-reloading
  // debug_output_sound(gp);

}


shared_function Game*
func game_init(Platform *pp) {
  ASSERT(GAME_STATE_SIZE < pp->backbuffer_size);

  u64 alloc_size = pp->backbuffer_size;
  u8 *alloc_ptr = (u8*)(pp->backbuffer);

  Game *gp = (Game*)alloc_ptr;
  alloc_ptr += GAME_STATE_SIZE;
  alloc_size -= GAME_STATE_SIZE;

  u64 game_frame_arena_backbuffer_size = MB(5);
  u64 game_temp_arena_backbuffer_size  = MB(5);
  u64 game_main_arena_backbuffer_size = alloc_size - game_frame_arena_backbuffer_size - game_temp_arena_backbuffer_size;

  u8 *game_main_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_main_arena_backbuffer_size;

  u8 *game_frame_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_frame_arena_backbuffer_size;

  u8 *game_temp_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_temp_arena_backbuffer_size;

  ASSERT(alloc_ptr <= ((u8*)pp->backbuffer + pp->backbuffer_size));

  gp->main_arena  = arena_create_ex(game_main_arena_backbuffer_size, true, game_main_arena_backbuffer);
  gp->frame_arena = arena_create_ex(game_frame_arena_backbuffer_size, true, game_frame_arena_backbuffer);
  gp->temp_arena  = arena_create_ex(game_temp_arena_backbuffer_size,  true, game_temp_arena_backbuffer);

  gp->pixels_per_meter = PIXELS_PER_METER;

  gp->did_reload = true;

  gp->guy_bitmap     = load_bitmap(gp, str8_lit("assets/guy.bmp"));
  gp->monster_bitmap = load_bitmap(gp, str8_lit("assets/monster.bmp"));
  gp->frog_bitmap    = load_bitmap(gp, str8_lit("assets/frog.bmp"));

  init_tile_map(gp);

  init_camera(gp);

  init_player_1(gp);
  init_player_2(gp);
  init_monster(gp);
  init_frog(gp);

  return gp;
}


#endif

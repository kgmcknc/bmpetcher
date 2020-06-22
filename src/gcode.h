
#ifndef SRC_GCODE_H
#define SRC_GCODE_H

#include <stdint.h>
#include "bmp_io.h"

#define MAX_GCODE_LINE 256

enum gcode_states {
   START_IMAGE_PASS, MOVE_XY_MIN, SEARCH_X, MOVE_XY_MAX, SEARCH_Y, END_IMAGE_PASS
};

struct gcode_struct {
   FILE* fp;
   char line[MAX_GCODE_LINE];
   double mm_per_pixel;
   double mm_per_color;
   double xy_speed;
   double z_speed;
   double z_mm_step;
   double xy_mm_step;
   uint8_t x_move_pos;
   uint8_t y_move_pos;
   uint8_t cutting;
   double x_pos;
   double y_pos;
   double z_pos;
   double x_max;
   double y_max;
   double offset_height;
   double max_depth;
   double relief_height;
   double current_depth;
   int32_t current_pix_height;
   int32_t x_pix;
   int32_t y_pix;
   int32_t z_pix;
   enum gcode_states state;
};


void create_gcode_file(struct gcode_struct* gcode, char* gcode_file_path);
void bmp_to_gcode(struct image_struct* image, struct gcode_struct* gcode);
void set_new_z_height(struct gcode_struct* gcode, struct image_struct* image);
void move_to_position(struct gcode_struct* gcode);
void update_position(struct gcode_struct* gcode);
void set_new_x(struct gcode_struct* gcode, uint8_t add);
void set_new_y(struct gcode_struct* gcode, uint8_t add);
void check_pixel_height(struct gcode_struct* gcode, struct image_struct* image);
uint8_t check_pixel_cutting(struct gcode_struct* gcode, struct image_struct* image);
void set_new_x_height(struct gcode_struct* gcode, struct image_struct* image);


void move_to_cutting_height(struct gcode_struct* gcode);
void move_to_relief_height(struct gcode_struct* gcode);

#endif //SRC_GCODE_H

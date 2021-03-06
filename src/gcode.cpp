
#include <stdint.h>
#include <stdio.h>
#include "global_settings.h"
#include "main.h"
#include "bmp_io.h"
#include "gcode.h"

// need to do more advanced move decision
// can we not move motor over areas that don't need cutting anymore?
// don't move to end of lines when we don't have anything left to cut on the ends...
// look at rest of line and when we won't cut for the rest of the line, go ahead and find first pixel to cut on the next line...
// maybe do line search and find first/last postion of cut (min/max x) and move to those locations?
// same for y when searching y direction. That way we aren't moving all the way over when there's nothing left to cut...

void create_gcode_file(struct gcode_struct* gcode, char* gcode_file_path){
   FILE* gcode_fp;

   gcode_fp = fopen(gcode_file_path, "wb+");
   if(gcode_fp == 0){
      printf("Couldn't open gcode file\n");
   } else {
      printf("Successfully opened gcode file\n");
   }

   gcode->fp = gcode_fp;
}

void bmp_to_gcode(struct image_struct* image, struct gcode_struct* gcode){
   uint8_t processing = 1;
   
   gcode->x_pix = 0;
   gcode->y_pix = 0;
   gcode->z_pix = image->max_bit_depth_val;
   gcode->xy_speed = 2400.0;
   gcode->z_speed = 2000.0;
   gcode->z_mm_step = 0.4;
   gcode->xy_mm_step = 0.8;
   gcode->x_move_pos = 1;
   gcode->y_move_pos = 1;
   gcode->x_pos = 0.0;
   gcode->y_pos = 0.0;
   gcode->z_pos = 0.0;
   gcode->cutting = 0;
   gcode->max_depth = 2.0;
   gcode->relief_height = 0.30;
   gcode->offset_height = gcode->max_depth + gcode->relief_height;
   gcode->current_depth = gcode->max_depth;
   gcode->current_pix_height = image->max_bit_depth_val;
   gcode->mm_per_pixel = 0.8;
   gcode->mm_per_color = gcode->max_depth/((double) image->max_bit_depth_val);
   gcode->x_max = ((double) image->width)*gcode->mm_per_pixel;
   gcode->y_max = ((double) image->height)*gcode->mm_per_pixel;
   gcode->state = START_IMAGE_PASS;

   fprintf(gcode->fp, ";Starting BMP Gcode file\n");
   fprintf(gcode->fp, ";Image x res %d, y res: %d\n", image->width, image->height);
   fprintf(gcode->fp, ";Max Cut Depth: %lf\n", gcode->max_depth);
   fprintf(gcode->fp, ";z_mm_step: %lf\n", gcode->z_mm_step);
   fprintf(gcode->fp, ";xy_mm_step: %lf\n", gcode->xy_mm_step);
   fprintf(gcode->fp, ";mm per pix: %lf\n", gcode->mm_per_pixel);
   fprintf(gcode->fp, ";mm per color; %lf\n", gcode->mm_per_color);
   fprintf(gcode->fp, ";x max: %lf\n", gcode->x_max);
   fprintf(gcode->fp, ";y max: %lf\n", gcode->y_max);

   while(processing){
      switch(gcode->state){
         case START_IMAGE_PASS : {
            fprintf(gcode->fp, "G21 ;set units to mm\n"); // set units to mm
            fprintf(gcode->fp, "G90 ;set absolute positioning\n"); // set units to mm
            
            // set current position as absolute zero
            fprintf(gcode->fp, "G92 X0.0 Y0.0 Z%lf; set current position as absolute start point\n", gcode->max_depth);
            gcode->state = MOVE_XY_MIN;
            break;
         }
         case MOVE_XY_MIN : {
            // set to raise z all the way up
            //fprintf(gcode->fp, "G01 Z%lf F%lf\n", gcode->offset_height, gcode->speed);
            move_to_relief_height(gcode);
            // set to move to x=0,y=0
            fprintf(gcode->fp, "G01 X0.0 Y0.0 F%lf\n", gcode->xy_speed);
            
            set_new_z_height(gcode, image);
            
            gcode->cutting = 0;
            gcode->x_pix = 0;
            gcode->y_pix = 0;
            gcode->x_pos = 0.0;
            gcode->y_pos = 0.0;
            gcode->x_move_pos = 1;
            gcode->y_move_pos = 1;

            if((gcode->current_depth < gcode->z_mm_step)){
               gcode->state = END_IMAGE_PASS;
            } else {
               gcode->state = SEARCH_X;
            }
            break;
         }
         case SEARCH_X : {
            gcode->x_move_pos = 1;
            gcode->y_move_pos = 1;
            while(gcode->y_pix < (image->height-1)){
               if(gcode->x_move_pos){
                  while(gcode->x_pix < (image->width-1)){
                     gcode->x_pos = (((double) gcode->x_pix) * gcode->mm_per_pixel);
                     check_pixel_height(gcode, image);
                     gcode->x_pix = gcode->x_pix + 1;
                  }
               } else {
                  while(gcode->x_pix > 0){
                     gcode->x_pos = (((double) gcode->x_pix) * gcode->mm_per_pixel);
                     check_pixel_height(gcode, image);
                     gcode->x_pix = gcode->x_pix - 1;
                  }
               }
               check_pixel_height(gcode, image);
               gcode->x_move_pos = (gcode->x_move_pos) ? 0 : 1;
               set_new_y(gcode, 1);
            }
            gcode->state = MOVE_XY_MAX;
            break;
         }
         case MOVE_XY_MAX : {
            // set to raise z all the way up
            //fprintf(gcode->fp, "G01 Z%lf F%lf\n", gcode->offset_height, gcode->speed);
            move_to_relief_height(gcode);
            // set to move to x=max,y=max
            fprintf(gcode->fp, "G01 X%lf Y%lf F%lf\n", gcode->x_max, gcode->y_max, gcode->xy_speed);

            set_new_z_height(gcode, image);

            gcode->cutting = 0;
            gcode->x_move_pos = 0;
            gcode->y_move_pos = 0;
            gcode->x_pix = image->width - 1;
            gcode->y_pix = image->height - 1;
            gcode->x_pos = gcode->x_max;
            gcode->y_pos = gcode->y_max;

            if((gcode->current_depth < gcode->z_mm_step)){
               gcode->state = END_IMAGE_PASS;
            } else {
               gcode->state = SEARCH_Y;
            }
            break;
         }
         case SEARCH_Y : {
            gcode->x_move_pos = 0;
            gcode->y_move_pos = 0;
            while(gcode->x_pix > 0){
               if(gcode->y_move_pos){
                  while(gcode->y_pix < (image->height-1)){
                     check_pixel_height(gcode, image);
                     gcode->y_pix = gcode->y_pix + 1;
                  }
               } else {
                  while(gcode->y_pix > 0){
                     check_pixel_height(gcode, image);
                     gcode->y_pix = gcode->y_pix - 1;
                  }
               }
               check_pixel_height(gcode, image);
               gcode->y_move_pos = (gcode->y_move_pos) ? 0 : 1;
               set_new_x(gcode, 0);
            }
            gcode->state = MOVE_XY_MIN;
            break;
         }
         case END_IMAGE_PASS : {
            processing = 0;
            break;
         }
         default : {
            processing = 0;
            gcode->state = START_IMAGE_PASS;
            break;
         }
      }
   }
}

void set_new_z_height(struct gcode_struct* gcode, struct image_struct* image){
   int32_t new_z;

   gcode->current_depth = gcode->current_depth - gcode->z_mm_step; // decrease float mm down by our z step
   if(gcode->current_depth < 0.0){
      gcode->current_depth = 0.0;
   }

   new_z = (int32_t) (gcode->current_depth/gcode->mm_per_color + 0.5);// check to see if our current z results in the same z_pix offset...
   if(new_z < gcode->current_pix_height){
      // if lower than current pix height, set new pix height
      if(new_z < 0){
         gcode->current_pix_height = 0;
      } else {
         gcode->current_pix_height = new_z;
      }
   }
}

void move_to_position(struct gcode_struct* gcode){

   fprintf(gcode->fp, "G01 X%lf Y%lf Z%lf F%lf\n", gcode->x_pos, gcode->y_pos, gcode->z_pos, gcode->xy_speed);
   
}

void update_position(struct gcode_struct* gcode){
   gcode->x_pos = (((double) gcode->x_pix) * gcode->mm_per_pixel);
   gcode->y_pos = (((double) gcode->y_pix) * gcode->mm_per_pixel);
}

void check_pixel_height(struct gcode_struct* gcode, struct image_struct* image){
   if(check_pixel_cutting(gcode, image)){
      if(gcode->cutting == 0){
         move_to_position(gcode);
         move_to_cutting_height(gcode);
      }
   } else {
      if(gcode->cutting){
         move_to_position(gcode);
         move_to_relief_height(gcode);
      }
   }
}

uint8_t check_pixel_cutting(struct gcode_struct* gcode, struct image_struct* image){
   if(image->pix_data[gcode->y_pix][gcode->x_pix][0] < gcode->current_pix_height){
      return 1;
   } else {
      return 0;
   }
}

void set_new_x(struct gcode_struct* gcode, uint8_t add_pix){
   if(add_pix){
      gcode->x_pos = gcode->x_pos + gcode->xy_mm_step;
   } else {
      gcode->x_pos = gcode->x_pos - gcode->xy_mm_step;
   }
   gcode->x_pix = (int32_t) (gcode->x_pos/gcode->mm_per_pixel + 0.5);
   //printf("x delta step: %lf, %d\n", gcode->x_pos, gcode->x_pix);
}

void set_new_y(struct gcode_struct* gcode, uint8_t add_pix){
   if(add_pix){
      gcode->y_pos = gcode->y_pos + gcode->xy_mm_step;
   } else {
      gcode->y_pos = gcode->y_pos - gcode->xy_mm_step;
   }
   gcode->y_pix = (int32_t) (gcode->y_pos/gcode->mm_per_pixel + 0.5);
   //printf("y delta step: %lf, %d\n", gcode->y_pos, gcode->y_pix);
}

void move_to_cutting_height(struct gcode_struct* gcode){
   fprintf(gcode->fp, "G01 Z%lf F%lf\n", gcode->current_depth, gcode->z_speed);
   gcode->z_pos = gcode->current_depth;
   gcode->cutting = 1;
}

void move_to_relief_height(struct gcode_struct* gcode){
   fprintf(gcode->fp, "G01 Z%lf F%lf\n", gcode->offset_height, gcode->z_speed);
   gcode->z_pos = gcode->offset_height;
   gcode->cutting = 0;
}


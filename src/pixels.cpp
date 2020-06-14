
#include <stdint.h>
#include <stdio.h>
#include "global_settings.h"
#include "main.h"
#include "pixels.h"

void init_pixel(struct pixel_struct* pixel, uint32_t h_res, uint32_t h_off, uint32_t v_res, uint32_t v_off){
   for(int i=0;i<MAX_PIXELS_PER_CLOCK;i++){
      for(int j=0;j<MAX_COLORS_PER_PIXEL;j++){
         pixel->pix_data.data[i][j] = 0;
      }
   }
   pixel->pix_data.valid = 0;
   pixel->pix_data.ready = 0;
   pixel->h_res = h_res;
   pixel->v_res = v_res;
   pixel->h_off = h_off;
   pixel->v_off = v_off;
   pixel->h_count = 0;
   pixel->v_count = 0;
   pixel->line_done = 0;
   pixel->frame_done = 0;
}

void get_pixel(struct pixel_struct* pixel, uint16_t pix_data[SCALER_MAX_V_RES][SCALER_MAX_H_RES][MAX_COLORS_PER_PIXEL]){
   pixel->pix_data.valid = 0;
   for(int i=0;i<MAX_PIXELS_PER_CLOCK;i++){
      for(int c=0;c<MAX_COLORS_PER_PIXEL;c++){
         pixel->pix_data.data[i][c] = pix_data[pixel->v_off + pixel->v_count][pixel->h_off + pixel->h_count + i][c];
      }
      if((pixel->h_count + i) < pixel->h_res){
         pixel->pix_data.valid = pixel->pix_data.valid + 1;
      }
   }
   #ifdef IN_PIXEL_PRINTS
      printf("Current Input Pixel: H: %d, V: %d\n", pixel->h_count, pixel->v_count);
   #endif
   add_pixel_count(pixel, pixel->pix_data.valid);
}

void put_pixel(struct pixel_struct* pixel, uint16_t pix_data[SCALER_MAX_V_RES][SCALER_MAX_H_RES][MAX_COLORS_PER_PIXEL]){
   #ifdef OUT_PIXEL_PRINTS
      printf("Current Output Pixel: H: %d, V: %d\n", pixel->h_count, pixel->v_count);
   #endif
   for(int i=0;i<MAX_PIXELS_PER_CLOCK;i++){
      for(int c=0;c<MAX_COLORS_PER_PIXEL;c++){
         pix_data[pixel->v_off + pixel->v_count][pixel->h_off + pixel->h_count + i][c] = pixel->pix_data.data[i][c];
      }
   }
   add_pixel_count(pixel, pixel->pix_data.valid);
   pixel->pix_data.valid = 0;
}

void add_pixel_count(struct pixel_struct* pixel, uint32_t add_val){
   pixel->h_count = pixel->h_count + add_val;
   if(pixel->h_count >= pixel->h_res){
      pixel->line_done = 1;
      pixel->h_count = 0;
      pixel->v_count = pixel->v_count + 1;
      if(pixel->v_count >= pixel->v_res){
         pixel->v_count = 0;
         pixel->frame_done = 1;
      } else {
         pixel->frame_done = 0;
      }
   } else {
      pixel->line_done = 0;
   }
}
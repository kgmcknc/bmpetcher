
#ifndef SRC_PIXELS_H
#define SRC_PIXELS_H

#include <stdint.h>
#include "global_settings.h"

struct pixel_data_struct {
   uint8_t ready;
   uint16_t data[MAX_PIXELS_PER_CLOCK][MAX_COLORS_PER_PIXEL];
   uint8_t valid;
};

struct pixel_struct {
   uint32_t h_res;
   uint32_t v_res;
   uint32_t h_off;
   uint32_t v_off;
   uint32_t h_count;
   uint32_t v_count;
   uint8_t line_done;
   uint8_t frame_done;
   struct pixel_data_struct pix_data;
};

void init_pixel(struct pixel_struct* pixel, uint32_t h_res, uint32_t h_off, uint32_t v_res, uint32_t v_off);
void get_pixel(struct pixel_struct* pixel, uint16_t pix_data[SCALER_MAX_V_RES][SCALER_MAX_H_RES][MAX_COLORS_PER_PIXEL]);
void put_pixel(struct pixel_struct* pixel, uint16_t pix_data[SCALER_MAX_V_RES][SCALER_MAX_H_RES][MAX_COLORS_PER_PIXEL]);
void add_pixel_count(struct pixel_struct* pixel, uint32_t add_val);

#endif //SRC_PIXELS_H
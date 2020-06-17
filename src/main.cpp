
#include <stdint.h>
#include <stdio.h>
#include "string.h"

#include "global_settings.h"
#include "main.h"
#include "bmp_io.h"
#include "pixels.h"
#include "image.h"
#include "gcode.h"
#include "math.h"

struct image_struct input_image;
struct image_struct output_image;
struct program_struct program;
struct gcode_struct gcode;

int main(int argc, char **argv){
   char file_path[200];

   printf("Starting BMP Test\n\n");
   program.done = 0;

   /*while(!program.done){
      if(!program.processing){
         print_control_list();
         get_program_command(program.command);
         set_command(&program);
      } else {

      }
   }*/

   strcpy(file_path, "./images/baby_yoda.bmp");
   read_bmp(&input_image, file_path);
   
   //init_bmp_image(&output_image, input_image.width, input_image.height, MAX_BITS_PER_COLOR, MAX_COLORS_PER_PIXEL);

   //copy_image(&input_image, &output_image);

   greyscale_image(&input_image);

   invert_image(&input_image);

   flip_image_vertical(&input_image);

   strcpy(file_path, "./images/baby_yoda_code.gcode");
   create_gcode_file(&gcode, file_path);
   bmp_to_gcode(&input_image, &gcode);

   strcpy(file_path, "./images/my_test.bmp");
   write_bmp(&input_image, file_path);
   fclose(gcode.fp);

   return(0);
}

void print_control_list(void){
   printf("List of Commands:\n");
   printf("Open Image File: o - filename\n");
   printf("");
   printf("");
   printf("Enter Command And Press Enter:\n");
}

void get_program_command(char* command){
   printf("Command: ");
   scanf("%c", command[0]);
   printf("\n");
}

void set_command(struct program_struct* program){

}
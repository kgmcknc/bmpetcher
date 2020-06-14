
#ifndef SRC_MAIN_H
#define SRC_MAIN_H

#define MAX_STRING 256

struct program_struct{
   uint8_t done;
   uint8_t processing;
   uint8_t image_loaded;
   char command[MAX_STRING];
};

void print_control_list(void);
void get_program_command(char* command);
void set_command(struct program_struct* program);

#endif // SRC_MAIN_H
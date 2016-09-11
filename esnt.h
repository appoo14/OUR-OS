char *vidptr = (char*)0xb8000;
unsigned int current_loc = 0;
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES
#define LINE_SIZE  BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE 

enum vga_color {
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};

void numprint( int num)
{
       
       int i=0,j=0,*num_temp;
       while(num>0){
           num_temp[j]=num%10;
           num=num/10;
           j++;
       }
       i = --j;
       while(i >= 0){
           vidptr[current_loc++] = num_temp[i]+48;
           vidptr[current_loc++] = 0x07;
           i--;
       }
}
void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
       
}
unsigned int line_position()
{
         unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
         unsigned int loc;
         loc = current_loc % (line_size);
         return loc;
}
unsigned int strlen(const char *str)
{
         unsigned int i=0;
         while(str[i]!='\0')
          i++;
         return i;
}
int strcmp(const char *str1, const char *str2)
{
         unsigned int i=0,len1=strlen(str1),len2=strlen(str2); 
         if(len2!=len1)
             return -1;       
         while(i<len1){
             if(str1[i]!=str2[i])
               return -1;
               i++;
           }
         return 0;
}


int strlcmp(const char *str1, const char *str2,unsigned int l)
{
         unsigned int i=0,len1=l; 
         
         while(i<len1){
             if(str1[i]!=str2[i])
               return -1;
               i++;
           }
         return 0;
}


void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
         move_cursor(current_loc/2); 
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
        move_cursor(current_loc);

}

void ksprint(const char *str)
{
  unsigned int i = 0;
  kprint_newline();
  while (str[i] != '\0') {
    vidptr[current_loc++] = str[i++];
    vidptr[current_loc++] = 0x07;
  }
         move_cursor(current_loc/2); 
}

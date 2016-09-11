#include "keyboard_map.h"
#include "cursor.h"
#include <stddef.h>
#include <stdint.h>
/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES
#define LINE_SIZE  BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE 

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
//special keys
#define ENTER_KEY_CODE 0x1C
#define BACKSPACE_KEY_CODE 0x0E 
#define TAB_KEY 0x0F
#define CAPSLOCK_KEY_CODE 0x3A


int CAPSLOCK_STATUS=0; 

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern void move_cursor(unsigned int position);
extern void inc_cursor(unsigned int i);
extern void dec_cursor(unsigned int i);


extern uint8_t color;
/* current cursor location */
extern unsigned int current_loc;
unsigned int mode = 0;/*
							0: Username
							1: Password
							2: CLI
							3: Calculator
							4: Edit
							5: Shut Down 
							6: logout
					*/	
unsigned int mode4 = 0,modex=0,modeu = 0;						 

/* video memory begins at address 0xb8000 */

extern unsigned int yobaby;						

extern char *vidptr;
char *vidptrcopy;
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

extern const char *str_default1;
extern const char *str_default2;
extern const char *str_password;
extern const char *str_default;
extern const char *user_name_prompt;
extern const char *password_prompt;

unsigned int crsloc()
{
   return (current_loc-1)/2;
}

extern char tester1[150];
extern char tester2[150];
extern char tester3[150];
extern char tester4[150];
extern char tester5[150];

struct textfile {
	char* contents;
};

extern struct textfile1 {
		char* name; 
};
 
struct textfile *txt;
struct textfile1 *txt1;

int filecount = 0;
int currfile = 0;
extern int concount[5];

void txtinit()
{
	int i = 0;
	while(i<100)
	{
		txt1[i].name = "                                                                                                                                                                                                                                                          ";
		txt[i].contents = "                                                                                                                                                                                                                                                          ";
	}
}


void txtpad_init()
{
	int i = 0;
	while(i<5)
	{
		concount[i] = 0;
	}
	modeu= 0;
}



struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		if(str[i]=='\n') {
			kprint_newline();
			i++;
			continue;
		}
		else if(str[i]=='\t'){
			kprint_tab();
			i++;
			continue;
		}
		else if(current_loc!=SCREENSIZE){
			vidptr[current_loc++] = str[i++];
			vidptr[current_loc++] = color;
		}
		else
		{
			clear_screen();
			vidptr[current_loc++] = str[i++];
			vidptr[current_loc++] = color;
		
		}
	}
         move_cursor(current_loc/2); 
}


void kprintl(const char *str,unsigned int n)
{
	unsigned int i = 0, j = 0;
	while (j<n) {
		if(str[i]=='\n') {
			kprint_newline();
			i++;
			continue;
		}
		else if(str[i]=='\t'){
			kprint_tab();
			i++;
			continue;
		}
		else if(current_loc!=SCREENSIZE){
			vidptr[current_loc++] = str[i++];
			vidptr[current_loc++] = color;
		}
		else
		{
			clear_screen();
			vidptr[current_loc++] = str[i++];
			vidptr[current_loc++] = color;
		
		}
	}
         move_cursor(current_loc/2); 
}


void ksprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		if(str[i]=='\n') {
			kprint_newline();
			i++;
			continue;
		}
		else if(str[i]=='\t'){
			kprint_tab();
			i++;
			continue;
		}
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = color;
	}
         move_cursor(current_loc/2); 
}

void numprint( int num)
{
       
       int i=0,j=0,*num_temp;
       kprint("                    >");
       if(num == 0)
       {
       		vidptr[current_loc++] = 48;
       		vidptr[current_loc++] = color;
       }
       else{
       		if(num < 0)
       		{
       			vidptr[current_loc++] = '-';
       			vidptr[current_loc++] = color;
       			num*=-1;
       		}	
       		while(num>0){
           		num_temp[j]=num%10;
           		num=num/10;
           		j++;
       		}
   		
       i = --j;
       while(i >= 0){
           vidptr[current_loc++] = num_temp[i]+48;
           vidptr[current_loc++] = color;
           i--;
       }
   }
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
        move_cursor(current_loc);

}


void kprint_tab(void)
{
	unsigned int i;
	for (i = 0; i < 5; ++i)
	{
		vidptr[current_loc++] = ' ';
		vidptr[current_loc++] = color;
	}

}

void clear_screen(void)
{
	unsigned int i = 0;
	current_loc = i;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = color;
	}
	move_cursor(current_loc);
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

void terminal_chgbckd(char* command)
{
	if(strcmp("light green",command)==0)
		color = make_color((color & 0x0f), COLOR_LIGHT_GREEN);
	else if(strcmp("green",command)==0)
		color = make_color((color & 0x0f), COLOR_GREEN);
	else if(strcmp("light blue",command)==0)
		color = make_color((color & 0x0f), COLOR_LIGHT_BLUE);
	else if(strcmp("blue",command)==0)
		color = make_color((color & 0x0f), COLOR_BLUE);
	else if(strcmp("white",command)==0)
		color = make_color((color & 0x0f), COLOR_WHITE);
	else if(strcmp("black",command)==0)
		color = make_color((color & 0x0f), COLOR_BLACK);
	else if(strcmp("red",command)==0)
		color = make_color((color & 0x0f), COLOR_RED);
	else if(strcmp("light red",command)==0)
		color = make_color((color & 0x0f), COLOR_LIGHT_RED);
	else if(strcmp("brown",command)==0)
		color = make_color((color & 0x0f), COLOR_BROWN);
	else if(strcmp("cyan",command)==0)
		color = make_color((color & 0x0f), COLOR_CYAN);
	else if(strcmp("light cyan",command)==0)
		color = make_color((color & 0x0f), COLOR_LIGHT_CYAN);
	
	
	unsigned int i = 1;
	for(;i<160*25;i+=2)
		vidptr[i] = color;
}


void terminal_chgfckd(char* command)
{
	if(strcmp("light green",command)==0)
		color = make_color(COLOR_LIGHT_GREEN, (color & 0xf0) >> 4);
	else if(strcmp("green",command)==0)
		color = make_color(COLOR_GREEN, (color & 0xf0) >> 4);
	else if(strcmp("light blue",command)==0)
		color = make_color(COLOR_LIGHT_BLUE, (color & 0xf0) >> 4);
	else if(strcmp("blue",command)==0)
		color = make_color(COLOR_BLUE, (color & 0xf0) >> 4);
	else if(strcmp("white",command)==0)
		color = make_color(COLOR_WHITE, (color & 0xf0) >> 4);
	else if(strcmp("black",command)==0)
		color = make_color(COLOR_BLACK, (color & 0xf0) >> 4);
	else if(strcmp("red",command)==0)
		color = make_color(COLOR_RED, (color & 0xf0) >> 4);
	else if(strcmp("light red",command)==0)
		color = make_color(COLOR_LIGHT_RED, (color & 0xf0) >> 4);
	else if(strcmp("brown",command)==0)
		color = make_color(COLOR_BROWN, (color & 0xf0) >> 4);
	else if(strcmp("cyan",command)==0)
		color = make_color(COLOR_CYAN, (color & 0xf0) >> 4);
	else if(strcmp("light cyan",command)==0)
		color = make_color(COLOR_LIGHT_CYAN, (color & 0xf0) >> 4);
	
	
	unsigned int i = 1;
	for(;i<160*25;i+=2)
		vidptr[i] = color;
}

int fact(int a)
{
	int n = 1;
	while(a!=0)
		n = n*(a--);
	return n;
}

int opt(char op,int a,int b)
{
 int c;
 if(op == '+')
    c=a+b;
 else if(op == '-')
    c=a-b;
 else if(op == '*')
    c=a*b;
 else if(op == '/')
    c=a/b;
 else if(op == 'm')
 	c = a%b;
 else if(op == 'f')
    c=fact(a);
 return c;
}

void programs()
{
	char *command,*cdr;
    unsigned int loc,i=0,j=0,temploc;
        if (mode != 1 && mode != 21)
        {
        	         loc=line_position();
        	         loc=loc-42;
        	         temploc=loc/2;
        	         loc=current_loc-loc;
        	         while(j<temploc){
        	              command[i]=vidptr[loc];
        	              i++;
        	              j++;
        	              loc=loc+2;
        	         }
        		   command[i]='\0';
        }
        
        else
        {
        	         
        	         loc=line_position();
        	         loc=loc-42;
        	         temploc=loc/2;
        	         loc=current_loc-loc;
        	         while(j<temploc){
        	              command[i]=vidptrcopy[loc];
        	              i++;
        	              j++;
        	              loc=loc+2;
        	         }
        		   command[i]='\0';
        }
	   	if(mode == 0)
	   	{
	   		if((strcmp(command,"root")) == 0)
	   		{
	   			str_default = str_default1;
	   			mode = 1;
	   			modeu = 0;
	   		}
	   		else if((strcmp(command,"guest")) == 0)
	   		{
	   			clear_screen();
	   			kprint("Welcome Guest!");
	   			str_default = str_default2;
	   			modeu = 1;
	   			
	   			mode = 2;
	   		}
	   		else
	   		{
	   			kprint_newline();
	   			kprint("User doesn't exist!");
	   			
	   		}
	   	}
	   	else if(mode == 1)
	   	{
	   		if((strcmp(command,"havvss")) == 0)
	   		{
	   			clear_screen();
	   			kprint("Welcome root!");
	   			str_default = str_default1;
	   			
	   			
	   			mode = 2;
	   		}
	   		else
	   		{
	   			kprint_newline();
	   			kprint("Wrong password!");
	   			
	   			mode = 1;
	   		}
	   	}
	   	else if (mode == 21)
	   	{
	   		mode = 2;
	   	}
	   	else if(mode == 2){
	   			   		 if((strcmp(command,"logout")) == 0)
	   		            	mode = 6;
	   		            else if((strcmp(command,"shut down")) == 0)
	   		            	{
	   		            		mode = 5;
	   		            		clear_screen();
	   		            	}
	   		            else if((strcmp(command,"info")) == 0)
	   		            {
	   		            	clear_screen();
	   		            	mode = 21;
	   		            	kprint("|------------------------------------------------------------------------------||-----------------------------------OUR_OS_1.1---------------------------------||------------------------------------------------------------------------------||                                                                              ||                                                                              ||  <<DEVELOPED BY>>                                                            ||      Abhyuday Bharat, Harish T, Vishnuram                                    ||                                                                              ||  <<KEY FEATURES>>                                                            ||                                                                              ||     # Command line interface                                                 ||     # Stand-alone Kernel                                                     ||     # Keyboard enabled                                                       ||     # Multiple User Account support                                          ||     # Various color theme support                                            ||                                                                              ||  <<IN-BUILT APPLICATIONS>>                                                   ||     # Calculator                                                             ||     # Text Editor                                                            ||                                                                              ||                                                                              ||------------------------------------------------------------------------------|");
	   		      			modex = 1;      	
	   		            }
	   		            else if((strcmp(command,"help")) == 0)
	   		            {
	   		            	clear_screen();
	   		            	mode = 21;
	   		            	kprint("|------------------------------------------------------------------------------||---------------------------------COMMANDS_HELP--------------------------------||------------------------------------------------------------------------------||                                                                              ||                                                                              ||  <<COMMANDS>>                                                                ||                                                                              ||       # info                                                                 ||              //Provides with basic information of the OS                     ||       # calc                                                                 ||              //Opens the in-built CALCULATOR application                     ||       # txtpad                                                               ||              //Opens the in-built TEXT EDITOR application                    ||       # bcolor <color>                                                       ||              //Changes the background color to <color>                       ||       # fcolor <color>                                                       ||              //Changes the font color to <color>                             ||       # help                                                                 ||              //Display help                                                  ||       # logout                                                               ||              //Logout user                                                   ||       # shut down                                                            ||              //Shuts down the OS                                             ||                                                                              ||------------------------------------------------------------------------------|");
	   		            	modex = 1;      	
	   		            }	
	   		            else if((strcmp(command,"calc")) == 0)
	   		            {
	   		            	mode = 3;
	   		            	clear_screen();
	   		            	kprint("- - - - - - - - - - - - - - - - - CALCULATOR v1.0 - - - - - - - - - - - - - - -");
	   		          		kprint_newline();
	   		          	}
	   		          	else if((strcmp(command,"txtpad")) == 0)
	   		          	{
	   		          		mode = 4;
	   		            	clear_screen();
	   		            	kprint("- - - - - - - - - - - - - - - - - TXTPAD v1.0 - - - - - - - - - - - - - - - -");
	   		          		kprint_newline();
	   		          		mode4 = 0;
	   		          	}
	   		          	else if((strlcmp(command,"bcolor ",7)) == 0)
	   		            {
	   		            	command = command + 7;
	   		            	terminal_chgbckd(command);
	   		            }
	   		            else if((strlcmp(command,"fcolor ",7)) == 0)
	   		            {
	   		            	command = command + 7;
	   		            	terminal_chgfckd(command);
	   		            }
	   		          else if((strcmp(command,"clear")) == 0)
	   		            {
	   		            	clear_screen();
	   		            }
	   		            
	   		          else {
	   						current_loc=current_loc+(LINE_SIZE-32-(temploc*2));
	   		                kprint( "ERROR :BAD COMMAND ");
	   			  		}	
	   	}
	   	else if (mode == 5)
	   	{
	   		color = make_color(COLOR_BLACK,COLOR_BLACK);
	   		clear_screen();
	   	}
	   	else if (mode == 4)
	   	{
	   		if (mode4 == 0)
	   		{
	   			if((strlcmp(command,"edit ",5)) == 0)
	   			{
	   				if(modeu == 0)
	   				mode4 = 1;
	   				else
	   				{
	   					kprint_newline();
	   					kprint("Editing not allowed in Guest mode!");
	   					kprint_newline();
	   				}	
	   			}	
	   			else if((strlcmp(command,"view ",5)) == 0)
	   				mode4 = 2;
	   			else if((strcmp(command,"exit")) == 0)
	   				{
	   					mode = 2;
	   					clear_screen();
	   				}
	   			else if(strcmp(command,"help")==0)
         		{
         			clear_screen();
         			kprint("|------------------------------------------------------------------------------||-----------------------------------TXTPAD_HELP--------------------------------||------------------------------------------------------------------------------||                                                                              ||                                                                              ||                                                                              ||  <<COMMANDS>>                                                                ||                                                                              ||       # edit <FILE>                                                          ||              Appends to the content of<FILE>                                 ||       # view <FILE>                                                          ||              View the contents of <FILE>                                     ||       # 00end00                                                              ||              To end EDIT or VIEW operation                                   ||       # exit                                                                 ||              To exit the text editor                                         ||       # AVAILABLE FILES                                                      ||              a b c d e                                                       ||       # help                                                                 ||              To display help for textpad                                     ||                                                                              ||                                                                              ||                                                                              ||------------------------------------------------------------------------------|");
         			modex = 1;
         		}

	   			if (mode4 == 1)
	   			{
	   				kprint("\n\nStart typing!\n");
	   				int j = 5,i = 0,f = 0;
	   				char* tempname;
	   				while(command[j]!='\0')
	   				{
	   					tempname[i] = command[j];
	   					i++;
	   					j++;
	   				}
	   				tempname[i] = '\0';
	   				if(strcmp(tempname,"a") == 0)
	   				{
	   					mode4 = 11;
	   					
	   				}	
	   				else if(strcmp(tempname,"b") == 0)
	   					mode4 = 12;
	   				else if(strcmp(tempname,"c") == 0)
	   					mode4 = 13;
	   				else if(strcmp(tempname,"d") == 0)
	   					mode4 = 14;
	   				else if(strcmp(tempname,"e") == 0)
	   					mode4 = 15;
	   				return;
	   			}
	   			if (mode4 == 2)
	   			{
	   				int j = 5,i = 0,f = 0;
	   				char* tempname;
	   				while(command[j]!='\0')
	   				{
	   					tempname[i] = command[j];
	   					i++;
	   					j++;
	   				}
	   				tempname[i] = '\0';
	   				if(strcmp(tempname,"a") == 0)
	   				{
	   					kprint_newline();
	   					kprint(tester1);
	   					
	   				}	
	   				else if(strcmp(tempname,"b") == 0)
	   				{
	   					kprint_newline();
	   					kprint(tester2);
	   					
	   				}
	   				else if(strcmp(tempname,"c") == 0)
	   				{
	   					kprint_newline();
	   					kprint(tester3);
	   					
	   				}
	   				else if(strcmp(tempname,"d") == 0)
	   				{
	   					kprint_newline();
	   					kprint(tester4);
	   					
	   				}
	   				else if(strcmp(tempname,"e") == 0)
	   				{
	   					kprint_newline();
	   					kprint(tester5);
	   					
	   				}
	   				else
	   				{
	   					kprint_newline();
	   					kprint("                    >");
	   					kprint("File not available!");
	   				}
	   				kprint_newline();
	   				mode4 = 0;
	   			}
	   		}
	   		if (mode4 == 11)
	   		{
	   			if(strcmp(command,"00end00")==0)
	   			{
	   				tester1[concount[0]] = '\0';
	   				mode4 = 0;
	   				kprint("\n                    >END\n");
	   			}
	   			else
	   			{
	   		
	   				int i = 0;
	   				while(command[i]!='\0')
	   				{
	   					tester1[concount[0]] = command[i];
	   					i++;
	   					concount[0]++;
	   				}
	   				tester1[concount[0]] = '\n';
	   				i++;
	   				concount[0]++;
	   				
	   				
	   			}
	   			return;
	   		}

	   		if (mode4 == 12)
	   		{
	   			if(strcmp(command,"00end00")==0)
	   			{
	   				tester2[concount[1]] = '\0';
	   				mode4 = 0;
	   				kprint("\n                    >END\n");
	   			}
	   			else
	   			{
	   		
	   				int i = 0;
	   				while(command[i]!='\0')
	   				{
	   					tester2[concount[1]] = command[i];
	   					i++;
	   					concount[1]++;
	   				}
	   				tester2[concount[1]] = '\n';
	   				i++;
	   				concount[1]++;
	   				
	   				
	   			}
	   			return;
	   		}
	   		if (mode4 == 13)
	   		{
	   			if(strcmp(command,"00end00")==0)
	   			{
	   				tester3[concount[2]] = '\0';
	   				mode4 = 0;
	   				kprint("\n                    >END\n");
	   			}
	   			else
	   			{
	   		
	   				int i = 0;
	   				while(command[i]!='\0')
	   				{
	   					tester3[concount[2]] = command[i];
	   					i++;
	   					concount[2]++;
	   				}
	   				tester3[concount[2]] = '\n';
	   				i++;
	   				concount[2]++;
	   				
	   				
	   			}
	   			return;
	   		}
	   		if (mode4 == 14)
	   		{
	   			if(strcmp(command,"00end00")==0)
	   			{
	   				tester4[concount[3]] = '\0';
	   				mode4 = 0;
	   				kprint("\n                    >END\n");
	   			}
	   			else
	   			{
	   		
	   				int i = 0;
	   				while(command[i]!='\0')
	   				{
	   					tester4[concount[3]] = command[i];
	   					i++;
	   					concount[3]++;
	   				}
	   				tester4[concount[3]] = '\n';
	   				i++;
	   				concount[3]++;
	   				
	   				
	   			}
	   			return;
	   		}
	   		if (mode4 == 15)
	   		{
	   			if(strcmp(command,"00end00")==0)
	   			{
	   				tester5[concount[4]] = '\0';
	   				mode4 = 0;
	   				kprint("\n                    >END\n");
	   			}
	   			else
	   			{
	   		
	   				int i = 0;
	   				while(command[i]!='\0')
	   				{
	   					tester5[concount[4]] = command[i];
	   					i++;
	   					concount[4]++;
	   				}
	   				tester5[concount[4]] = '\n';
	   				i++;
	   				concount[4]++;
	   				
	   				
	   			}
	   			return;
	   		}

	   	}
	   	else if (mode == 3)
        {
        	if(command[0] >='0' && command[0] <= '9' ){
            int j=0,a=0,b=0,f=1,k,check=1;
            int num;
            while(check == 1){
                if(command[j]=='\0')
                {
                   kprint( "\nERROR :BAD COMMAND ");
                   return;
                }

                else if(command[j] == '+' || command[j] == '-' || command[j] == '*' || command[j] == '/' || command[j] == 'f' || command[j] == 'm' )
                   check=0;
                else if ((command[j] < '0' || command[j] > '9') && (command[j]!='f'||command[j]!='m'))
                {
                   kprint( "\nERROR :BAD COMMAND ");
                   return;
                }
	
                j++;
            }    
            k=j-2;
            while(k>=0){ 
               num=command[k];
               num=num-48;
               a=a+(num*f);
               f=f*10;
               k--;
             }   
            unsigned char op=command [--j];
            if (op != 'f')
            {
            	while(command[j]!='\0' )
            	    j++;
            	f=1;
            	k=--j; 
            	check=1;
            	while(check){
    
            	   num=command[k];
            	   num=num-48;
            	   b=b+((num)*f);
            	   f=f*10;
            	   k--;
            	   if(command[k] == '+' || command[k] == '-' || command[k] == '*' || command[k] == '/' || command[k] == 'm')
            	       check=0;
            	   else if ((command[k] < '0' || command[k] > '9') && (command[j]!='m'))
            	   {
            	      kprint( "\nERROR :BAD COMMAND ");
            	      return;
            	   }
            	 }
            }  
            kprint_newline();
            int result ;
            result=opt(op,a,b);
            numprint(result);
         }
         else if(strcmp(command,"help")==0)
         {
         	clear_screen();
         	kprint("|------------------------------------------------------------------------------||--------------------------------CALCULATOR_HELP-------------------------------||------------------------------------------------------------------------------||                                                                              ||                                                                              ||  <<OPERATIONS>>                                                              ||                                                                              ||       # ADDITION (+)                                                         ||              syntax:- <no1>+<no2>           //ex: 2+2                        ||       # SUBTRACTION (-)                                                      ||              syntax:- <no1>-<no2>           //ex: 5-3                        ||       # MULTIPLICATION (*)                                                   ||              syntax:- <no1>*<no2>           //ex: 2*3                        ||       # DIVISION (/)                                                         ||              syntax:- <no1>/<no2>           //ex: 8/4                        ||       # FACTORIAL (f)                                                        ||              syntax:- <no>f                 //ex: 5f                         ||       # MODULUS (m)                                                          ||              syntax:- <no1>m<no2>           //ex: 8m3                        ||                                                                              ||                                                                              ||                                                                              ||                                                                              ||                                                                              ||------------------------------------------------------------------------------|");
         	modex = 1;
         }
         else if(strcmp(command,"exit")==0)
         {
         	clear_screen();
         	mode = 2;
         } 
        }
                        
}

void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;
    
	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
            if(line_position() != 42 && mode != 7 && mode != 21 && mode != 22&& modex != 1 && modex != 2)
            {
                programs();
			}
			if(mode != 7 && mode != 21 && mode != 22&& modex != 1)
				kprint_newline();
			if(mode!=21 && mode!=22 && mode4 != 3&& modex != 1)
			{
							if(current_loc>SCREENSIZE - LINE_SIZE)
								clear_screen();

							if(mode == 2)
				                kprint(str_default);
			}
			else if (mode == 21)
				mode = 22;
			else if (mode == 22)
			{
				clear_screen();
				mode = 2;
				kprint(str_default);
			}
			
            if(mode == 1)
                kprint(password_prompt);
            if(mode == 0)
                kprint(user_name_prompt);
            if(mode == 5)
            {
            	clear_screen();
   				kprint("\n\n\n\n\n- - - - - - - - - - - - - - - HAVE A GOOD DAY - - - - - - - - - - - - - - - - -");
            }
            if(mode == 3 && modex != 1)
            {
            	kprint("                    >");
            }
            if(mode == 4)
            {
            	kprint("                    >");
            }
            if(mode == 6)
            {
            	kprint("\nTHANK YOU FOR TRYING OUR-OS\n");
            	mode = 7;
            }
            else if(mode == 7)
            {
            	clear_screen();
            	const char *str = "- - - - - - - - - - - - - - - - - - OUR-OS - - - - - - - - - - - - - - - - - - - ";
    
   				const char *credit = "\n\tWelcome to OUR-OS !\n\t-OS Project for CSE222 course (Dr. Sakkaravarthi R) in VIT-C\n\n\t-made by:\n\t  Harish,Abhyuday,Vishnu Ram,Vishnu Ramesh,Savio,Sumedh\n";
    kprint(str);
				kprint_newline();
				kprint(credit);
				kprint_newline();
				kprint_newline();
				kprint_newline();
    			kprint(user_name_prompt);
            	mode = 0;
            }
            if(modex == 1)
            {
           		modex = 2;
           		return;
            }
           	if(modex == 2)
           		modex = 0;

            
            
			return;
		}	
        else if(keycode == BACKSPACE_KEY_CODE && (mode == 2 || mode == 4 || mode == 3 || mode == 0|| mode == 1)){
                 unsigned int loc,temp_loc;
                 loc = line_position();
                 temp_loc=current_loc-(loc-38); 
                 if(loc > 42){    
                         vidptr[--current_loc]= color;
                         vidptr[--current_loc]=' ';
                 }  
                 
                 else if(vidptr[temp_loc] != ':'){
                        current_loc = current_loc - 44;
                 }
                 dec_cursor(1);
                   
                 
        }
		                else if(keycode == CAPSLOCK_KEY_CODE ){
		                              if(CAPSLOCK_STATUS==0)
		                                 CAPSLOCK_STATUS = 1;
		                              else
		                                 CAPSLOCK_STATUS = 0;
		                  
		                } 
		                else if(keycode == TAB_KEY && (mode == 2 || mode == 4 || mode == 3)){
		                       current_loc = current_loc + 20;
		                       inc_cursor(2);
		                } 
		   
		                else if(mode == 1 || mode == 5 || mode4 == 2 || mode == 21){  
		                         

		                         char data;   
		                         unsigned int loc;
		                         data=keyboard_map[(unsigned char) keycode];
		                         if(CAPSLOCK_STATUS==1)
		                              data=data-32;  
		                         loc = line_position();
		                         if(loc > 157)
		                               current_loc = current_loc + 44;
				         vidptrcopy[current_loc++] = data;
				         vidptrcopy[current_loc++] = color;

		                } 
		                else {
		                	char data;   
		                         unsigned int loc;
		                         data=keyboard_map[(unsigned char) keycode];
		                         if(CAPSLOCK_STATUS==1)
		                              data=data-32;  
		                         loc = line_position();
		                         if(loc > 157)
		                               current_loc = current_loc + 44;
				         vidptr[current_loc++] = data;
				         vidptr[current_loc++] = color;
		                         inc_cursor(1);
		                }
		               
	}
        
}


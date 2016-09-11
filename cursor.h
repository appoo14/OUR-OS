extern void write_port(unsigned short port, unsigned char data);
extern unsigned int crsloc();

void move_cursor(unsigned int position)
{
    // cursor LOW port to vga INDEX register
    write_port(0x3D4, 0x0F);
    write_port(0x3D5, (unsigned char)(position&0xFF));
    // cursor HIGH port to vga INDEX register
    write_port(0x3D4, 0x0E);
    write_port(0x3D5, (unsigned char )((position>>8)&0xFF));
}
 
void inc_cursor(unsigned int i)
{
    unsigned int loc = crsloc();
    loc=loc+i-1;
    move_cursor(loc);
    
}
 
void dec_cursor(unsigned int i)
{
    unsigned int loc = crsloc();
    loc=loc-i+1;
    move_cursor(loc);   
}

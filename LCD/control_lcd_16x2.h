#ifndef control_lcd_16x2
#define control_lcd_16x2

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char *s);
void typeChar(char val);
void print_lcd_automatico(float refer, float inter, int timer, char menu[6], int indiceMenu);
void print_lcd_manual(float refer, float inter, int timer);
void print_lcd_preaquencendo(float refer, float inter);
void print_lcd_esfriando(float refer, float inter);

#endif /* control_lcd_16x2 */
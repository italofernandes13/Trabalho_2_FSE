#ifndef _UART_
#define _UART_

void initUart();
void closeUart();
int sendFloat(char cmd[],float f);
float requestFloat(char cmd[]);
int requestSignal(char cmd[]);
int requestInt(char cmd[]);
int sendInt(char cmd[], int x);

#endif
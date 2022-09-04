#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART

#include "../CRC/crc16.h"

int uart0_filestream = -1;

void initUart(){
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); // Open in non blocking read/write mode
    if (uart0_filestream == -1){
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else{
        printf("UART inicializada!\n");
    }
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);
}

void closeUart(){
    printf("UART finalizada!\n");
    close(uart0_filestream);
}

int checkCrc(char conteudoRecebido[]){
    short crc = calcula_CRC(conteudoRecebido, 7);

    // printf("%i %i\n", (crc & 0xFF), conteudoRecebido[7]);
    // printf("%i %i\n", ((crc >> 8) & 0xFF), conteudoRecebido[8]);

    if ((crc & 0xFF) == conteudoRecebido[7] && ((crc >> 8) & 0xFF) == conteudoRecebido[8])
        return 1;

    return 0;
}


int sendFloat(char cmd[], float f){
    printf("%f\n",f);
    unsigned char tx_buffer[50];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = cmd[0];
    *p_tx_buffer++ = cmd[1];
    *p_tx_buffer++ = cmd[2];
    *p_tx_buffer++ = cmd[3];
    *p_tx_buffer++ = cmd[4];
    *p_tx_buffer++ = cmd[5];
    *p_tx_buffer++ = cmd[6];

    *p_tx_buffer++ = *((int *)&f) & 0xFF;
    *p_tx_buffer++ = (*((int *)&f) >> 8) & 0xFF;
    *p_tx_buffer++ = (*((int *)&f) >> 16) & 0xFF;
    *p_tx_buffer++ = (*((int *)&f) >> 24) & 0xFF;

    short crc = calcula_CRC(&tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
    *p_tx_buffer++ = crc & 0xFF;
    *p_tx_buffer++ = (crc >> 8) & 0xFF;

    if (uart0_filestream != -1){
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
        if (count < 0){
            return 0;
        }
        else{
            return 1;
        }
    }

    usleep(700000);
}

float requestFloat(char cmd[]){

    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = cmd[0];
    *p_tx_buffer++ = cmd[1];
    *p_tx_buffer++ = cmd[2];
    *p_tx_buffer++ = cmd[3];
    *p_tx_buffer++ = cmd[4];
    *p_tx_buffer++ = cmd[5];
    *p_tx_buffer++ = cmd[6];

    short crc = calcula_CRC(&tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
    *p_tx_buffer++ = crc & 0xFF;
    *p_tx_buffer++ = (crc >> 8) & 0xFF;

    // printf("Buffers de memória criados!\n");

    if (uart0_filestream != -1){
        // printf("Escrevendo caracteres na UART ...");
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
        if (count < 0){
            return 0;
            // printf("UART TX error\n");
        }
        else{
            // printf("escrito.\n");
        }
    }

    usleep(700000);

    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1){
        unsigned char rx_buffer[9];
        int rx_length = read(uart0_filestream, &rx_buffer, 9); // Filestream, buffer to store in, number of bytes to read (max)

        int tentativa = 0;

        for (tentativa = 0; tentativa < 5; tentativa++){

            if (checkCrc(rx_buffer)){
                break;
            }
            else{
                requestFloat(cmd);
            }
        }

        if (tentativa == 5){
            // printf("Erros ao lidar com CRC");
            return 0;
        }

        if (rx_length < 0){
            return 0;
            // printf("Erro na leitura.\n"); // An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0){
            return 0;
            // printf("Nenhum dado disponível.\n"); // No data waiting
        }
        else{
            unsigned char floats[4] = {rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6]};
            float f = *((float *)&floats);
            return f;
        }
    }
}

int requestSignal(char cmd[]){
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = cmd[0];
    *p_tx_buffer++ = cmd[1];
    *p_tx_buffer++ = cmd[2];
    *p_tx_buffer++ = cmd[3];
    *p_tx_buffer++ = cmd[4];
    *p_tx_buffer++ = cmd[5];
    *p_tx_buffer++ = cmd[6];
    *p_tx_buffer++ = cmd[7];

    short crc = calcula_CRC(&tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));

    *p_tx_buffer++ = crc & 0xFF;
    *p_tx_buffer++ = (crc >> 8) & 0xFF;

    // printf("Buffers de memória criados!\n");

    if (uart0_filestream != -1){
        // printf("Escrevendo caracteres na UART ...");
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));

        if (count < 0){
            // printf("UART TX error\n");
            return 0;
        }
    }

    usleep(700000);
    //----- CHECK FOR ANY RX BYTES -----

    if (uart0_filestream != -1){
        unsigned char rx_buffer[10];

        int rx_length = read(uart0_filestream, &rx_buffer, 10); // Filestream, buffer to store in, number of bytes to read (max)
        int tentativa = 0;

        for (tentativa = 0; tentativa < 5; tentativa++){
            if (checkCrc(rx_buffer)){
                break;
            }
            else{
                requestSignal(cmd);
            }
        }

        if (tentativa == 5){
            // printf("Erros ao lidar com CRC");
            return 0;
        }
        if (rx_length < 0){
            // printf("Erro na leitura.\n"); // An error occured (will occur if there are no bytes)
            return 0;
        }
        else if (rx_length == 0){
            // printf("Nenhum dado disponível.\n"); // No data waiting
            return 0;
        }
        else{
            int x = rx_buffer[3] + (rx_buffer[4] << 8) + (rx_buffer[5] << 16) + (rx_buffer[6] << 24);
            if (x==cmd[7])
                return 1;
        }
    }
}

int requestInt(char cmd[]){
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = cmd[0];
    *p_tx_buffer++ = cmd[1];
    *p_tx_buffer++ = cmd[2];
    *p_tx_buffer++ = cmd[3];
    *p_tx_buffer++ = cmd[4];
    *p_tx_buffer++ = cmd[5];
    *p_tx_buffer++ = cmd[6];

    short crc = calcula_CRC(&tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));

    *p_tx_buffer++ = crc & 0xFF;
    *p_tx_buffer++ = (crc >> 8) & 0xFF;

    // printf("Buffers de memória criados!\n");

    if (uart0_filestream != -1){

        // printf("Escrevendo caracteres na UART ...");
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));

        if (count < 0){
            // printf("UART TX error\n");
            return 0;
        }
    }

    usleep(700000);
    //----- CHECK FOR ANY RX BYTES -----

    if (uart0_filestream != -1){
        unsigned char rx_buffer[9];

        int rx_length = read(uart0_filestream, &rx_buffer, 9); // Filestream, buffer to store in, number of bytes to read (max)
        int tentativa = 0;


        for (tentativa = 0; tentativa < 5; tentativa++){
            if (checkCrc(rx_buffer)){
                break;
            }
            else{
                requestInt(cmd);
            }
        }

        if (tentativa == 5){
            // printf("Erros ao lidar com CRC");
            return -1;
        }
        if (rx_length < 0){
            return -1;
            // printf("Erro na leitura.\n"); // An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0){
            return -1;
            // printf("Nenhum dado disponível.\n"); // No data waiting
        }
        else{
            int x = rx_buffer[3] + (rx_buffer[4] << 8) + (rx_buffer[5] << 16) + (rx_buffer[6] << 24);
            return x;
        }
    }
}

int sendInt(char cmd[], int x){
    unsigned char tx_buffer[50];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = cmd[0];
    *p_tx_buffer++ = cmd[1];
    *p_tx_buffer++ = cmd[2];
    *p_tx_buffer++ = cmd[3];
    *p_tx_buffer++ = cmd[4];
    *p_tx_buffer++ = cmd[5];
    *p_tx_buffer++ = cmd[6];

    *p_tx_buffer++ = x & 0xFF;
    *p_tx_buffer++ = (x >> 8) & 0xFF;
    *p_tx_buffer++ = (x >> 16) & 0xFF;
    *p_tx_buffer++ = (x >> 24) & 0xFF;

    short crc = calcula_CRC(&tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));

    *p_tx_buffer++ = crc & 0xFF;
    *p_tx_buffer++ = (crc >> 8) & 0xFF;


    if (uart0_filestream != -1){
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
        // printf("\nXXXXX enviando: %d\n", count);

        if (count < 0){
            return 0;
        }
        else{
            return 1;
        }

    }

    usleep(700000);
}



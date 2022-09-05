#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <time.h>

#include "UART/uart.h"
#include "LCD/control_lcd_16x2.h"
// #include "bme/bme280.h"
// #include "bme/bmeutils.h"
#include "PID/pid.h"

// Comandos gerais
unsigned char solicitaTempInterna[7] = {0x01, 0x23, 0xC1, 4, 1, 9, 0};
unsigned char solicitaTempReferencia[7] = {0x01, 0x23, 0xC2, 4, 1, 9, 0};
unsigned char leComandoUsuario[7] = {0x01, 0x23, 0xC3, 4, 1, 9, 0};
unsigned char sinalControleInt[7] = {0x01, 0x16, 0xD1, 4, 1, 9, 0};

unsigned char estadoDoSistemaLigar[8] = {0x01, 0x16, 0xD3, 4, 1, 9, 0, 1};
unsigned char estadoDoSistemaDesligar[8] = {0x01, 0x16, 0xD3, 4, 1, 9, 0, 0};

unsigned char EnviaEstadodeFuncionamentoLigar[8] = {0x01, 0x16, 0xD5, 4, 1, 9, 0, 0};
unsigned char EnviaEstadodeFuncionamentoDesligar[8] = {0x01, 0x16, 0xD5, 4, 1, 9, 0, 1};
unsigned char EnviavalordoTemporizador[7] = {0x01, 0x16, 0xD6, 4, 1, 9, 0};

// Variaveis gerais
float tempInterna, tempReferencia, tempoFunc, tempAmbiente = 27.0;
double sinalPID;
int timer = 0, modo = 0, indiceMenu = 0, tempoPreDefinido = 0, preAquencendo = 0;
char menu[6] = {"MBPFC"};

#define I2C_ADDR 0x27 // I2C device address

// Define some device constants
#define LCD_CHR 1  // Mode - Sending data
#define LCD_CMD 0  // Mode - Sending command
#define LINE1 0x80 // 1st line
#define LINE2 0xC0 // 2nd line

// Define-se os pinos para cada dispositivo
const int PWMpinRes = 4; // WiringPI GPIO 23
const int PWMpinVet = 5; // WiringPI GPIO 24


void ligaForno(){
    if (requestSignal(estadoDoSistemaLigar)){
        printf("Forno ligado!\n");
            preAquencendo = 3;
    }
    else
        printf("Erro ao ligar forno :(\n");
}

void desligaForno(){
    if (requestSignal(estadoDoSistemaDesligar)){
        printf("Forno desligado!\n");
    }
    else
        printf("Erro ao desligar forno :(\n");
}

void alimentoMenu(){
    if(indiceMenu==0){
        // batata 30°C e o tempo 15
        tempReferencia = 30.0;
        tempoPreDefinido = 15;
        pid_atualiza_referencia(tempReferencia);
        indiceMenu = 1;
        printf("aqui %d\n", indiceMenu);
    }
    else if(indiceMenu==1){
        // peixe 40°C e o tempo 25
        tempReferencia = 40.0;
        tempoPreDefinido = 25;
        pid_atualiza_referencia(tempReferencia);
        indiceMenu = 2;
        printf("aqui %d\n", indiceMenu);
    }
    else if(indiceMenu==2){
        // frango 45°C e o tempo 30
        tempReferencia = 45.0;
        tempoPreDefinido = 30;
        pid_atualiza_referencia(tempReferencia);
        indiceMenu = 3;
        printf("aqui %d\n", indiceMenu);
    }
    else if(indiceMenu==3){
        // carne 55°C e o tempo 50
        tempReferencia = 50.0;
        tempoPreDefinido = 50;
        pid_atualiza_referencia(tempReferencia);
        indiceMenu = 4;
        printf("aqui %d\n", indiceMenu);
    }
    else if(indiceMenu==4){
        tempReferencia = requestFloat(solicitaTempReferencia);
        pid_atualiza_referencia(tempReferencia);
        indiceMenu = 0;
        printf("aqui %d\n", indiceMenu);
    }
}

int main(){
    
    // Variaveis que vao armazenar o tempo
    time_t t = time(NULL);   
    struct tm tm = *localtime(&t);

    pid_configura_constantes(30.0, 0.2, 400.0);

    initUart();
    desligaForno();

    lcd_init(); // setup LCD
    ClrLcd();
    
    // Tratamos os dispositivos conectados ao GPIO
    if (softPwmCreate(PWMpinRes, 0, 100) == 0)
        printf("PWM do resistor criado!\n");

    if (softPwmCreate(PWMpinVet, 0, 100) == 0)
        printf("PWM da ventoinha criado!\n");

    tempReferencia = requestFloat(solicitaTempReferencia);
    pid_atualiza_referencia(tempReferencia);

    while (1){

        tempInterna = requestFloat(solicitaTempInterna);

        if(preAquencendo == 1){
            print_lcd_preaquencendo(tempReferencia, tempInterna);  
        }
        else if(preAquencendo == 2){
            print_lcd_esfriando(tempAmbiente, tempInterna);  
        }
        else if(indiceMenu == 0){
            tempReferencia = requestFloat(solicitaTempReferencia);
            tempoFunc = sendInt(EnviavalordoTemporizador, timer);
            print_lcd_manual(tempReferencia, tempInterna, timer);    
        }
        else if((indiceMenu > 0 && preAquencendo == 0)){
            tempoFunc = sendInt(EnviavalordoTemporizador, tempoPreDefinido - timer);
            print_lcd_manual(tempReferencia, tempInterna, tempoPreDefinido - timer);   
            if(tempoPreDefinido - timer <= 0){
                printf("Forno parado!\n");
                pid_atualiza_referencia(tempAmbiente);
                preAquencendo = 2;
                timer = 0;
            }       
        }
        else if(indiceMenu > 0 && preAquencendo == 3){
            tempoFunc = sendInt(EnviavalordoTemporizador, tempoPreDefinido - timer);
            print_lcd_automatico(tempReferencia, tempInterna, tempoPreDefinido, menu, indiceMenu);    
        }   

        // Verificamos se os dados estão consistentes no terminal
        int comandoUsuario = requestInt(leComandoUsuario);
        printf("Tempo %d s\n", timer);
        printf("Tempo Funcionamento %d s\n", tempoFunc);
        printf("Temp interna: %f\n", tempInterna);
        printf("Temp Referencia: %f\n", tempReferencia);
        printf("Comando usuario: %d\n", comandoUsuario);

        if(comandoUsuario == 1){
            ligaForno();
        }
        else if(comandoUsuario == 2){
            desligaForno();
            ClrLcd();
            break;
        }
        else if(comandoUsuario == 3){
            printf("Forno Aquecendo!\n");
            preAquencendo = 1;
            modo = 1;
            timer = 0;
        }

        else if(comandoUsuario == 4){
            printf("Forno parado!\n");
            pid_atualiza_referencia(tempAmbiente);
            preAquencendo = 2;
            timer = 0;
        }            
        else if(comandoUsuario == 7)
            alimentoMenu();

        if (modo == 1){
            printf("Modo Aquecimento\n");
        
            // Tratamento dos modos de operacao
            sinalPID = pid_controle(tempInterna);
            if (sinalPID > -40 && sinalPID < 0)
                sinalPID = -40;
            sendInt(sinalControleInt,sinalPID);
            printf("Sinal PID: %lf, preAquecendo %d\n", sinalPID, preAquencendo);

            if (sinalPID < 0){
                softPwmWrite(PWMpinRes, 0);
                delay(0.7);
                softPwmWrite(PWMpinVet, sinalPID * -1);
                delay(0.7);
            }
            else if (sinalPID > 0){
                softPwmWrite(PWMpinVet, 0);
                delay(0.7);
                softPwmWrite(PWMpinRes, sinalPID);
                delay(0.7);
            }
            if(preAquencendo == 2){
                if(tempInterna<=tempAmbiente){
                    
                    modo = 0;
                    preAquencendo = 3;
                }
            }
            else if(tempInterna==tempReferencia){
                preAquencendo = 0;
            }
            if(preAquencendo == 0){
                timer++;
            }
        }
    FILE *fpt;
    fpt = fopen("experimento.csv", "a+");
    fprintf(fpt, "%02d/%02d/%d;%02d:%02d:%02d;%0.2lf;%0.2lf;%0.2lf;%0.2lf\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tempInterna, tempAmbiente, tempReferencia, sinalPID);
    // fprintf(fpt, "%0.2lf;%0.2lf;%0.2lf;%0.2lf\n", tempInterna, tempAmbiente, tempReferencia, sinalPID);
    fclose(fpt);
    }   

    closeUart();
    return 0;
}
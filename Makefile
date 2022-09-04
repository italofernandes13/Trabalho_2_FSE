raspbian: 

	gcc main.c UART/uart.c CRC/crc16.c LCD/control_lcd_16x2.c PID/pid.c -I ../ -lwiringPi -o bin && ./bin


linux: 

	gcc main.c UART/uart.c CRC/crc16.c LCD/control_lcd_16x2.c PID/pid.c -I ../ -lwiringPi -o bin && ./bin


clean:

	rm -v bin

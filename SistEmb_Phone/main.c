#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"

int i;
char num;
uint8_t d_uint8Dato;
uint32_t uint32Retardo;

/*Rutina de Inicio de pantalla LCD 16x2*/
void LCD_INIT(){
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    GPIO_PORTK_DATA_R = 0x38;                                               // Lineas de escritura LCD
    GPIO_PORTM_DATA_R = 0x00;                                               // Enable -> 0
    SysCtlDelay(33333);                                                     // Retraso de 5[ms]
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    /*LIMPIA LCD*/
    GPIO_PORTK_DATA_R = 0x01;                                               // Lineas de escritura LCD
    GPIO_PORTM_DATA_R = 0x00;                                               // Enable -> 0
    SysCtlDelay(33333);                                                     // Retraso de 5[ms]
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    /*CURSOR LCD PARPADEA*/
    GPIO_PORTK_DATA_R = 0x0F;                                               // Lineas de escritura LCD
    GPIO_PORTM_DATA_R = 0x00;                                               // Enable -> 0
    SysCtlDelay(33333);                                                     // Retraso de 5[ms]
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1

    GPIO_PORTM_DATA_R = 0x06;                                               // Enable -> 1, RS -> 1
}

WRITE_LCD(num){
    GPIO_PORTK_DATA_R = num;
    GPIO_PORTM_DATA_R = 0x02;                                               // Enable -> 0, RS -> 1
    GPIO_PORTM_DATA_R = 0x06;                                               // Enable -> 1, RS -> 1
}

/*Rutina de inicio de puertos UART*/
/*UART_INIT(void){
    SYSCTL_RCGCUART_R |= 0X0001;                                            // Habilitar UART0
    SYSCTL_RCGCGPIO_R |= 0X0001;                                            // Habilitar Puerto A
    uint32Retardo = SYSCTL_RCGCGPIO_R;                                      // Retardo para habilitar periféricos
    UART0_CTL_R &= ~0X0001;                                                 // Deshabilitar UART
    UART0_IBRD_R = 27;                                                      // IBDR = int(50000000/16*115200)) = int(27.1267)
    UART0_FBRD_R = 8;                                                       // FBRD = round(0.1267*64 =8)
    UART0_LCRH_R = 0X0060;                                                  // 8-BITS, Habilitar FIFO
    UART0_CTL_R = 0X0301;                                                   // Habilitar RXE, TXE Y UART
    GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R&0XFFFFFF00)+0X00000011;  // UART
    GPIO_PORTA_AHB_AMSEL_R &= ~0X03;                                        // Deshabilitar función analógica en PA0-1
    GPIO_PORTA_AHB_AFSEL_R |= 0X03;                                         // Habilitar función alterna en PA0-1
    GPIO_PORTA_AHB_DEN_R |= 0X03;                                           // Habilitar función I/O Digital
}*/

/*void UART0_READ(void){
    while((UART0_FR_R&0X0010) != 0);                                        // Esperar a que RXFE sea 0

    d_uint8Dato = ((char)(UART0_DR_R&0xff));
    //return((char)(UART0_DR_R&0xff));
}*/

/*void UART0_WRITE(char dato){
    while((UART0_FR_R&0X0020) != 0);                                        // Esperar a que TXFF sea 0

    UART0_DR_R = dato;
}*/

/* DECODIFICADOR TECLADO MATRICIAL*/
void KYBRD(){
    SysCtlDelay(50000);

    for(i = 1; i <= 4; i++){
        switch(i){
            case 1:
                GPIO_PORTQ_DATA_R = 0x01;

                if(GPIO_PORTH_AHB_DATA_R == 0x01)
                    num = '1';

                else if(GPIO_PORTH_AHB_DATA_R == 0x02)
                    num = '2';

                else if(GPIO_PORTH_AHB_DATA_R == 0x04)
                    num = '3';

                else if(GPIO_PORTH_AHB_DATA_R == 0x08)
                    num = 'A';

                else
                    GPIO_PORTK_DATA_R = 0x00;
            break;

            case 2:
                GPIO_PORTQ_DATA_R = 0x02;

                if(GPIO_PORTH_AHB_DATA_R == 0x01)
                    num = '4';

                else if(GPIO_PORTH_AHB_DATA_R == 0x02)
                    num = '5';

                else if(GPIO_PORTH_AHB_DATA_R == 0x04)
                    num = '6';

                else if(GPIO_PORTH_AHB_DATA_R == 0x08)
                    num = 'B';

                else
                    GPIO_PORTK_DATA_R = 0x00;
            break;

            case 3:
                GPIO_PORTQ_DATA_R = 0x04;

                if(GPIO_PORTH_AHB_DATA_R == 0x01)
                    num = '7';

                else if(GPIO_PORTH_AHB_DATA_R == 0x02)
                    num = '8';

                else if(GPIO_PORTH_AHB_DATA_R == 0x04)
                    num = '9';

                else if(GPIO_PORTH_AHB_DATA_R == 0x08)
                    num = 'C';

                else
                    GPIO_PORTK_DATA_R = 0x00;
            break;

            case 4:
                GPIO_PORTQ_DATA_R = 0x08;

                if(GPIO_PORTH_AHB_DATA_R == 0x01)
                    num = '*';

                else if(GPIO_PORTH_AHB_DATA_R == 0x02)
                    num = '0';

                else if(GPIO_PORTH_AHB_DATA_R == 0x04)
                    num = '#';

                else if(GPIO_PORTH_AHB_DATA_R == 0x08)
                    num = 'D';

                else
                    GPIO_PORTK_DATA_R = 0x00;
            break;
        }
    }
}

void main(void){
    // Inicializacion del UART
//    uint32_t g_ui32SysClock;
//    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
//                                             SYSCTL_OSC_MAIN |
//                                             SYSCTL_USE_PLL |
//                                             SYSCTL_CFG_VCO_480), 50000000);

    // Se usa or para no sobrescribir otro registro que se este usando
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R14 |                              // Port Q
                         SYSCTL_RCGCGPIO_R12 |                              // Port N
                         SYSCTL_RCGCGPIO_R11 |                              // Port M
                         SYSCTL_RCGCGPIO_R9 |                               // Port K
                         SYSCTL_RCGCGPIO_R8 |                               // Port J
                         SYSCTL_RCGCGPIO_R7;                                // Port H

    while((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R12) == 0x00);
    /*---------- Entradas Teclado Matricial ----------*/
    GPIO_PORTH_AHB_DIR_R |= 0x00;                                           // BITS 0 - 3 Puerto H ENTRADAS
    GPIO_PORTH_AHB_DEN_R |= 0x0F;                                           // BITS 0 - 3 DIGITAL
    /*---------------- Botones Tarjeta ---------------*/
    GPIO_PORTJ_AHB_DIR_R |= 0x00;                                           // BITS 1 - 0 Puerto J ENTRADAS
    GPIO_PORTJ_AHB_DEN_R |= 0x03;                                           // BITS 1 - 0 DIGITAL
    GPIO_PORTJ_AHB_PUR_R |= 0x03;                                           // Resistencias PULL-UP
    /*------------- Lineas de Datos LCD --------------*/
    GPIO_PORTK_DIR_R |= 0xFF;                                               // Puerto K SALIDAS
    GPIO_PORTK_DEN_R |= 0xFF;                                               // BITS DIGITALES
    /*------------ Lineas de Control LCD -------------*/
    GPIO_PORTM_DIR_R |= 0x07;                                               // BITS 0 - 2 Puerto M SALIDAS
    GPIO_PORTM_DEN_R |= 0x07;                                               // BITS 0 - 2 DIGITAL
    /*----------------- LEDS Tarjeta -----------------*/
    GPIO_PORTN_DIR_R |= 0x03;                                               // BITS 1 - 0 Puerto N SALIDAS
    GPIO_PORTN_DEN_R |= 0x03;                                               // BITS 1 - 0 DIGITAL
    /*------ Lineas de Control Teclado Matricial -----*/
    GPIO_PORTQ_DIR_R |= 0x0F;                                               // BITS Puerto Q SALIDAS
    GPIO_PORTQ_DEN_R |= 0x0F;                                               // BITS DIGITALES

//    UART_INIT();                                                            // Inicia comunicación UART0
    /*PRUEBA PUERTO UART0*/
//    UART0_WRITE('A');
//    UART0_WRITE('T');
    /*Fin de prueba UART0*/
    LCD_INIT();                                                             // Inicia LCD

    while(1){
        KYBRD();
        WRITE_LCD(num);
    }

}

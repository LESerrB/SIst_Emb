#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

uint8_t d_uint8Dato;
uint32_t uint32Retardo;

UART_INI(void){
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
}

// Esperar hasta que se reciba un dato
char UART_Lee_dato(void){
    while((UART0_FR_R&0X0010) != 0);                                        // Esperar a que RXFE sea 0

    d_uint8Dato = ((char)(UART0_DR_R&0xff));
    //return((char)(UART0_DR_R&0xff));
}

char UART0_Escribe_dato(char dato){
    while((UART0_FR_R&0X0020) != 0);                                        // Esperar a que TXFF sea 0

    UART0_DR_R = dato;
}

int main(void){
    uint32_t g_ui32SysClock;
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 50000000);

    UART_INI();
    UART0_Escribe_dato('A');
    UART0_Escribe_dato('T');
    UART0_Escribe_dato(0x0d);
    UART0_Escribe_dato(0x0a);

    while(1){
        UART_Lee_dato();
        UART0_Escribe_dato(d_uint8Dato);
    }
}

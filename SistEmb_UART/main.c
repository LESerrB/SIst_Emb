#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

uint8_t d_uint8Dato;
uint32_t uint32Retardo;

UART_INIT(void){
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0 |                               // Habilitar UART0
                         SYSCTL_RCGCUART_R2;                                // Habilitar UART2
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;                                // Habilitar Puerto A
    uint32Retardo = SYSCTL_RCGCGPIO_R;                                      // Retardo para habilitar periféricos
    /* UART 0 Configurations */
    UART0_CTL_R &= UART_CTL_UARTEN;                                         // Deshabilitar UART
    UART0_IBRD_R = 27;                                                      // IBDR = int(50000000/16*115200)) = int(27.1267)
    UART0_FBRD_R = 8;                                                       // FBRD = round(0.1267*64 =8)
    UART0_LCRH_R = UART_LCRH_WLEN_8;                                        // 8-BITS, Habilitar FIFO
    UART0_CTL_R = UART_CTL_RXE |                                            // Habilitar RXE, TXE Y UART
                  UART_CTL_TXE |
                  UART_CTL_UARTEN;
    /* UART 2 Configurations */
    UART2_CTL_R &= UART_CTL_UARTEN;                                         // Deshabilitar UART
    UART2_IBRD_R = 27;                                                      // IBDR = int(50000000/16*115200)) = int(27.1267)
    UART2_FBRD_R = 8;                                                       // FBRD = round(0.1267*64 =8)
    UART2_LCRH_R = UART_LCRH_WLEN_8;                                        // 8-BITS, Habilitar FIFO
    UART2_CTL_R = UART_CTL_RXE |                                            // Habilitar RXE, TXE Y UART
                  UART_CTL_TXE |
                  UART_CTL_UARTEN;
    GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R&0X00FFFF00)+0X11000011;  // UART 0 y 3
    GPIO_PORTA_AHB_AMSEL_R &= ~(0X00C3);                                    // Deshabilitar función analógica en PA0-1
    GPIO_PORTA_AHB_AFSEL_R |= 0X00C3;                                       // Habilitar función alterna en PA0-1
    GPIO_PORTA_AHB_DEN_R |= 0X00C3;                                         // Habilitar función I/O Digital
}

// Recive información desde la terminal
// CMD -> Rx
char UART0_Lee_dato(void){
    while((UART0_FR_R & 0X0010) != 0);                                      // Esperar a que RXFE sea 0

    d_uint8Dato = ((char)(UART0_DR_R&0xff));
    return((char)(UART0_DR_R & 0xff));
}

// Envía informacion de la tarjeta a la terminal
// Tx -> CMD
void UART0_Escribe_dato(char dato){
    while((UART0_FR_R & 0X0020) != 0);                                      // Esperar a que TXFF sea 0

    UART0_DR_R = dato;
}
// Transmite al modem
// Tx -> SIM800L
void UART2_Transmit(char dato){
    while((UART2_FR_R & 0X0020) != 0);

    UART2_DR_R = dato;
}
// Recive del modem
// SIM800L -> Rx
char UART2_Recive(void){
    while((UART2_FR_R & 0X0010) != 0);

    return((char)(UART2_DR_R & 0xff));
}

int main(void){
    uint32_t g_ui32SysClock;
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 50000000);

    UART_INIT();
    UART0_Escribe_dato('S');
    UART0_Escribe_dato('y');
    UART0_Escribe_dato('s');
    UART0_Escribe_dato('R');
    UART0_Escribe_dato('d');
    UART0_Escribe_dato('y');
    UART0_Escribe_dato(0x0d);
    UART0_Escribe_dato(0x0a);

    //UART3_Transmit('A');

    while(1){
        //UART0_Escribe_dato(UART0_Lee_dato());
        UART2_Transmit('A');

    }
}

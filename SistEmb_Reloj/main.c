#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"

uint32_t HOLA;

void flip(void){
    HOLA = NVIC_ST_CTRL_R;
    GPIO_PORTN_DATA_R ^= 0x03;
}

void main(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;
    HOLA = 0x1234;
    HOLA = 0x5678;

    GPIO_PORTN_DIR_R = 0x03;
    GPIO_PORTN_DEN_R = 0x03;

    NVIC_ST_RELOAD_R = 2000000;
    NVIC_ST_CTRL_R = 0x03;

    while(1);
}

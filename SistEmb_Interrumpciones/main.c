#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
/**
 * main.c
 */

void PushJ0(void){
    GPIO_PORTJ_AHB_ICR_R = 0x01;                                             // LIMPIO BANDERA DE PJ0
    GPIO_PORTN_DATA_R ^= 0x03;
}

void main(void){
    // Se usa or para no sobrescribir otro registro que se este usando
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12 | SYSCTL_RCGCGPIO_R8;
    //SYSCTL_RCGCGPIO_R |= 0x1100;
    while((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R12) == 0x00);

    GPIO_PORTN_DIR_R |= 0x03;                                               // BITS 1 Y 0 DE PORT N SALIDAS
    GPIO_PORTN_DEN_R |= 0x03;                                               // BITS 1 Y 0 DIGITAL

    NVIC_EN1_R |= 0x80000;                                                  // ACTIVA LA INTERRUPCION EN EL PUERTO J

    GPIO_PORTJ_AHB_DIR_R |= 0x00;                                           // BITS 1 Y 0 DE PORT N SALIDAS
    GPIO_PORTJ_AHB_IEV_R |= 0x00;                                           // FLANCO DE BAJADA
    GPIO_PORTJ_AHB_IS_R |= 0x00;                                            // FLANCO 0/NIVEL 1
    GPIO_PORTJ_AHB_DEN_R |= 0x03;                                           // BITS 1 Y 0 DIGITAL
    GPIO_PORTJ_AHB_IM_R |= 0x01;                                            // ENMASCARAMIENTO PARA INTERRUMPIR
    GPIO_PORTJ_AHB_PUR_R |= 0x03;

    while(1){
        GPIO_PORTN_DATA_R = GPIO_PORTJ_AHB_DATA_R;
    }
    //return 0;
}

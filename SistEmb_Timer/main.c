#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/sysctl.h"

void intT7TA (void){
    TIMER7_ICR_R = 0x01;                        // LIMPIA BANDERA DE ATENCION A INTERRUPCION
    GPIO_PORTF_AHB_DATA_R ^= 0x10;              // TOGGLE EN EL BIT 4 DEL PUERTO F
}

int main (void){
    // CONFIGURAR PUERTOS
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;    // HABILITA RELOJ DEL PUERTO F

    // ESPERA A QUE ESTE LISTO (PREGUNTA POR EL REGISTRO PR)
    while((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R5) == 0x00);

    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R7;  // CONECTA RELOJ DEL TIMER 7

    while((SYSCTL_PRTIMER_R & SYSCTL_RCGCGPIO_R7) == 0x00);

    GPIO_PORTF_AHB_DIR_R = 0x10;                // PF4 SALIDA
    GPIO_PORTF_AHB_DEN_R = 0x10;                // HABILITA PF4

    // CONFIGURACIÓN DEL TIMER
    TIMER7_CTL_R = 0x0;                         // APAGA TIMER A
    TIMER7_CFG_R = 0x4;                         // TIMER A 16 BITS
    TIMER7_TAMR_R = 0x2;                        // MODO PERIODICO
    TIMER7_TAILR_R = 0x7A12;                    // TIEMPO PARA 1/2 SEG
    TIMER7_TAPR_R = 255;                        // PREESCALADOR A 256
    TIMER7_IMR_R = 0x1;                         // HABILITA INTERRUPCION
    TIMER7_ICR_R = 0x1;                         // LIMPIA BANDERA DE INTERRUPCION
    NVIC_EN3_R = 0x10;                          // HABILITA INTERRUPCIÓN 100
    TIMER7_CTL_R = 0x1;                         // HABILITA TIMER

    while(1){

    }
}

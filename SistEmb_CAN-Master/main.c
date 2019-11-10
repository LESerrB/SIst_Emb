/////////MASTER/////////////
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
//#include "driverlib/sysctl.h"

uint8_t MNUM_Tx, MNUM_Rx, j=0;
uint16_t ID_Tx, ID_Rx, MSK_Rx;
uint64_t DatoTx[8], DatoRx;

void Inicializacion(void){
    SYSCTL_RCGCGPIO_R|=0x2;                 // Reloj Puerto B

    while((SYSCTL_PRGPIO_R&0x2)==0){

    }
//------------ PUERTO B CAN0 PB4 Rx PB5 Tx ------------------------//
    GPIO_PORTB_AHB_CR_R=0x30;
    GPIO_PORTB_AHB_AFSEL_R=0x30;            // PB4 y PB5 funcion alterna
    GPIO_PORTB_AHB_PCTL_R=0x880000;         // Funcion CAN a los pines PB4-PB5 y funci?n I2C a PA0-PA1, PA6-PA7
    GPIO_PORTB_AHB_DIR_R=0x20;              // PB5 Salida Tx y PB4 Entrada Rx
    GPIO_PORTB_AHB_DEN_R=0x30;              // Hab funcion digital PB4 y PB5
    //SYSTICK
    NVIC_ST_CTRL_R=0x4;                     // Uso del reloj a 16 [Mhz]
}

void CAN_Inic(void){
    SYSCTL_RCGCCAN_R=0x1;                   // Reloj modulo 0 CAN

    while((SYSCTL_PRCAN_R&0x1)==0){

    }
    // Bit Rate= 1 Mbps CAN clock=25 [Mhz]
    CAN0_CTL_R=0x41;                        // Deshab. modo prueba, Hab. cambios en la config. y hab. inicializacion

    while(CAN0_IF1CRQ_R & 0x8000);          // Espera a que se termine una acci?n de lectura/escritura

    CAN0_BIT_R=0x4940;                      // TSEG2=0 TSEG1=2 SJW=0 BRP=4
    // Lenght Bit time=[TSEG2+TSEG1+3]*tq
    // =[(Phase2-1)+(Prop+Phase1-1)+3]*tq
    CAN0_CTL_R&=~0x41;                      // Hab. cambios en la config. y deshab. inicializacion
}

/*void PLL_Inic(uint32_t freq){
    // 0) configure the system to use RCC2 for advanced features
    // such as 400 MHz PLL and non-integer System Clock Divisor
    SYSCTL_RCC2_R |= 0x80000000;
    // 1) bypass PLL while initializing
    SYSCTL_RCC2_R |= 0x800;
    // 2) select the crystal value and oscillator source
    SYSCTL_RCC_R &= ~0x7C0;                 // clear XTAL field
    SYSCTL_RCC_R += 0x540;                  // configure for 16 MHz crystal
    SYSCTL_RCC2_R &= ~0x70;                 // clear oscillator source field
    SYSCTL_RCC2_R += 0x0;                   // configure for main oscillator source
    // 3) activate PLL by clearing PWRDN
    SYSCTL_RCC2_R &= ~0x2000;
    // 4) set the desired system divider and the system divider least significant bit
    SYSCTL_RCC2_R |= 0x40000000;            // use 400 MHz PLL
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) // clear system clock divider field
    + (freq<<22);                           // configure for 80 MHz clock
    // 5) wait for the PLL to lock by polling PLLLRIS

    while((SYSCTL_RIS_R&0x40)==0){

    };

    // 6) enable use of PLL by clearing BYPASS
    SYSCTL_RCC2_R &= ~0x800;
}*/

void Inter_Set(void){
    CAN0_CTL_R|=0x2;                        // Hab de interrupci?n
    NVIC_EN1_R|=((1<<(39-32)) & 0xFFFFFFFF);// Hab de interrucpci?n
}
//-------------------------------------------------------
//%%%%%%%%%%%%%%%%%%%% SYSTICK %%%%%%%%%%%%%%%%%%%%
//-------------------------------------------------------
/*void SysTick(retraso){
    NVIC_ST_RELOAD_R=retraso;
    NVIC_ST_CURRENT_R=0x0;
    NVIC_ST_CTRL_R=0x5;

    while((NVIC_ST_CTRL_R&0x10000)==0){

    }

    NVIC_ST_CTRL_R=0x0;
}*/

void CAN_Tx(uint8_t RMT, uint64_t Dato, uint16_t ID, uint8_t ObjNo){
    while(CAN0_IF1CRQ_R & 0x8000);          // Espera a que se termine una acci?n de lectura/escritura

    CAN0_IF1CMSK_R=0xB3;                    // WRNRD: Tx datos de CANIF->CAN message object (MNUM)
    //ARB: Tx ID+DIR+XTD+MSGVAL del message object a los registros de interfaz
    //CONTROL: Datos de control IFnMCTL->registros de interfaz
    //DATAA: Tx bytes 0-3 en el mensaje objeto->CANIFNDAn
    //DATAB: Tx bytes 4-7 en el mensaje objeto->CANIFNDAn
    CAN0_IF1ARB2_R=0x8000|(ID<<2);          // MSGVAL hab, XTD: 11 bit Standard, DIR para Rx y ID
    CAN0_IF1MCTL_R|=0x8;                    // DLC: 8 bytes en el data frame

    if(RMT==0){
        CAN0_IF1ARB2_R|=0x2000; //DIR para Tx
        //-------------------Escritura de datos---------------------//
        CAN0_IF1DA1_R = Dato & 0x000000000000FFFF;       // Byte 0-1
        CAN0_IF1DA2_R = (Dato & 0x00000000FFFF0000)>>16; // Byte 2-3
        CAN0_IF1DB1_R = (Dato & 0x0000FFFF00000000)>>32; // Byte 4-5
        CAN0_IF1DB2_R = (Dato & 0xFFFF000000000000)>>48; // Byte 6-7
    }

    CAN0_IF1CMSK_R |= 0x4;                  // Hab NEWDAT/TXRQST: Como WRNRD=1; una Tx es solicitada
    CAN0_IF1CRQ_R = ObjNo;                  // No. del identificador para indicar la prioridad

    while(CAN0_IF1CRQ_R & 0x8000);          // Espera a que se termine una acci?n de lectura/escritura
}

void CAN_Rx_Set(uint8_t RMT, uint16_t Filt, uint16_t ID, uint8_t ObjNo){
    while(CAN0_IF2CRQ_R & 0x8000);          // Espera a que se termine una acci?n de lectura/escritura

    CAN0_IF2CMSK_R=0xF3;                    // WRNRD: Tx datos de CANIF->CAN message object (MNUM)
    //MASK: Tx IDMASK+DIR+MXTD del mensaje objeto->registros de interfaz
    //ARB: Tx ID+DIR+XTD+MSGVAL del mensaje objeto->registros de interfaz
    //CONTROL: Datos de control IFnMCTL->registros de interfaz
    //DATAA: Tx bytes 0-3 en el mensaje objeto->CANIFNDAn
    //DATAB: Tx bytes 4-7 en el mensaje objeto->CANIFNDAn
    CAN0_IF2MSK1_R=0x0;
    CAN0_IF2MSK2_R=Filt<<2;                 // Aplicaci?n del enmascaramiento
    CAN0_IF2ARB2_R=0x8000|(ID<<2);          // MSGVAL hab, XTD: 11 bit Standard, DIR para Rx y ID
    CAN0_IF2MCTL_R|=0x1088;                 // UMASK: Filtro hab, EOB (end of Buffer): Mensaje ?nico, DLC: 8 bytes en el data frame

    if(RMT){
        CAN0_IF2ARB2_R|=0x2000;             // DIR para Tx
        CAN0_IF2MCTL_R|=0x200;              // RMTEN: Solicitud de Tx Remota
    }

    CAN0_IF2CRQ_R=ObjNo;                    // No. de identificador para alojar el objeto mensaje
}

uint64_t CAN_Rx(uint8_t ObjNo){
    uint64_t Dato=0;

    CAN0_IF2CMSK_R = 0x73;                  // Hab MASK, ARB, CONTROL, DATAA, DATAB
    CAN0_IF2CRQ_R = ObjNo;                  // No. de identificador para alojar el objeto mensaje

    while(CAN0_IF2CRQ_R & 0x8000);          // Espera a que se termine una acci?n de lectura/escritura

    if(CAN0_IF2MCTL_R & 0x8000){            // Si NEWDAT bit est? hab, hay un nuevo dato en los registros de datos
        //------------Obtenci?n de los nuevos datos-----------------//
        Dato|=(0x000000000000FFFF & CAN0_IF2DA1_R);                     // Byte 0-1
        Dato|=(0x00000000FFFF0000 & (CAN0_IF2DA2_R<<16));               // Byte 2-3
        Dato|=(0x0000FFFF00000000 & ((uint64_t)(CAN0_IF2DB1_R)<<32));   // Byte 4-5
        Dato|=(0xFFFF000000000000 & ((uint64_t)(CAN0_IF2DB2_R)<<48));   // Byte 6-7
        CAN0_IF2CMSK_R |= 0x4;                                          // Hab NEWDAT para limpiar el NEWDAT de IFnMCTL
    }

    if(CAN0_IF2MCTL_R & 0x4000){                                        // Si MSGLST bit est? hab, hubo un mensaje perdido
        CAN0_IF2MCTL_R &= ~0x4000;                                      // Limpieza del MSGLST bit
        return 0xFFFFFF;                                                // Indicador de un error
    }

    CAN0_IF2CMSK_R |= 0x8;                                              // Hab CLRINTPND para limpiar el INTPND de IFnMCTL
    CAN0_IF2CRQ_R =ObjNo;                                               // No. de identificador para ejecutar las acciones anteriores
    return Dato;
}

void Inter(void){
    uint8_t MNUM;
    uint64_t temp;

    MNUM=CAN0_INT_R;                        // Lectura del apuntador de interrupciones
    CAN0_STS_R&=~0x10;                      // Limpieza del bit de recepcion?
    DatoRx=CAN_Rx(MNUM);                    // Recepci?n de datos
    temp = DatoRx;

    if(temp > 30)
        DatoTx[0] = 1;                      // enciende led

    else
        DatoTx[0] = 0;                      // apaga led

    CAN0_IF1MCTL_R|=0x400;                  // Hab de INTPND al recibir mensaje
    CAN_Tx(0,DatoTx[0], 0x4, 0x1);          // Transmisi?n de datos
}

void main(void){
    Inicializacion();
    CAN_Inic();
    //PLL_Inic(15);                          // Reloj del sistema a 25 [Mhz]
    Inter_Set();

    MNUM_Tx=0x2;
    MNUM_Rx=0x5;
    ID_Tx=0x7;
    ID_Rx=0x4;
    MSK_Rx=0xF;

    while(1){

    }
}

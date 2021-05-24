#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

uint8_t d_uint8Dato;

uint32_t uint32Retardo;

char comand[50],
     SMS_msg[200],
     num,
     PhNum[10],
     temp[2],
     UARTMsg[50];

int j = 0, cont, EDO = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Configure the UART 0 and 2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
UART_Init(void){
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0 |                               // Enable UART0
                         SYSCTL_RCGCUART_R2;                                // Enable UART2

    while((SYSCTL_PRUART_R & 0x05) == 0);                                   // Wait until the register is active

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;                                // Enable Puerto A

    while((SYSCTL_PRGPIO_R & 0x01) == 0);                                   // Wait until the register is active

    uint32Retardo = SYSCTL_RCGCGPIO_R;                                      // Delay to enable peripherals

    UART0_CTL_R &= ~(UART_CTL_UARTEN);                                      // Disable UART 0
    UART2_CTL_R &= ~(UART_CTL_UARTEN);                                      // Disable UART 2

    GPIO_PORTA_AHB_AMSEL_R &= ~(0X00C3);                                    // Disable analog function in PA [0, 1, 6, 7]
    GPIO_PORTA_AHB_AFSEL_R |= 0X00C3;                                       // Enable altern function in PA [0, 1, 6, 7]
    GPIO_PORTA_AHB_PUR_R &= ~(0xC3);                                        // Pull-up res
    GPIO_PORTA_AHB_PDR_R &= ~(0xC3);                                        // Pull-down res
    GPIO_PORTA_AHB_DIR_R &= ~(0x81);                                        // Input PA [0, 6]
    GPIO_PORTA_AHB_DIR_R |= 0x42;                                           // Output PA [1, 7]
    GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R & 0x00FFFF00)            // UART 0 & 2
                             + 0x11000011;
    GPIO_PORTA_AHB_DEN_R |= 0X00C3;                                         // Enable I/O Digital function

    /* UART 0 Configurations */
    UART0_IBRD_R = (UART0_IBRD_R & ~0xFFFF) + 325;                          // IBDR = int(50000000/16*9600)) = int(325.5208)
    UART0_FBRD_R = (UART0_FBRD_R & ~0x3F) + 33;                             // FBRD = round(0.5208*64) = round(33)
    UART0_LCRH_R = UART_LCRH_WLEN_8 |                                       // 8-BITS
                   UART_LCRH_FEN;                                           // Enable FIFO
    UART0_CTL_R &= ~(UART_CTL_HSE);                                         // Disable HSE
    UART0_CTL_R |= UART_CTL_RXE |                                           // Enable RXE, TXE Y UART
                   UART_CTL_TXE |
                   UART_CTL_UARTEN;
    /* UART 2 Configurations */
    UART2_IBRD_R = (UART2_IBRD_R & ~0xFFFF) + 325;                          // IBDR = int(50000000/16*9600)) = int(325.5208)
    UART2_FBRD_R = (UART2_FBRD_R & ~0x3F) + 33;                             // FBRD = round(0.5208*64) = round(33)
    UART2_LCRH_R = UART_LCRH_WLEN_8 |                                       // 8-BITS
                   UART_LCRH_FEN;                                           // Enable FIFO
    UART2_CTL_R &= ~(UART_CTL_HSE);                                         // Disable HSE
    UART2_CTL_R |= UART_CTL_RXE |                                           // Enable RXE, TXE Y UART
                   UART_CTL_TXE |
                   UART_CTL_UARTEN;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Terminal -> Rx
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
char UART0_Lee_dato(void){
    while((UART0_FR_R & 0X0010) != 0);                                      // Waits RXFE = 0 (Receptor register not empty)

    return((char)(UART0_DR_R & 0xff));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Tx -> Terminal
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void UART0_Escribe_dato(char dato){
    while((UART0_FR_R & 0X0020) != 0);                                      // Waits TXFF = 0 (Transmiter register not full)

    UART0_DR_R = dato;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Tx -> SIM800L
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void UART2_Transmit(char dato){
    while((UART2_FR_R & 0X0020) != 0);

    UART2_DR_R = dato;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// SIM800L -> Rx
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
char UART2_Recive(void){
    while((UART2_FR_R & 0X0010) != 0);

    d_uint8Dato = ((char)(UART2_DR_R & 0xff));
    return (d_uint8Dato);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Sends AT comand to SIM800L
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void Trans_CMD(char Msg[50]){
    int i;

    for(i = 0; i < strlen(Msg); i++)
        UART2_Transmit(Msg[i]);

    UART2_Transmit(0X0D);   // <CR>
    UART2_Transmit(0X0A);   // <LF>
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Configure the SM800L modem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void Mdm_Init(){
    char TrsMsg[20];

    strcpy(UARTMsg, "AT+IPR=9600");                                         // Set baud rate to 9600
    Trans_CMD(UARTMsg);
    UART2_Recive();
    SysCtlDelay(1000);

    strcpy(TrsMsg, "ATE0");                                                 // Eliminates echo from the modem
    Trans_CMD(TrsMsg);
    UART2_Recive();
    SysCtlDelay(1000);

    strcpy(TrsMsg, "AT+COPS=3,0");                                          // Net configure
    Trans_CMD(TrsMsg);
    UART2_Recive();
    SysCtlDelay(1000);

    strcpy(TrsMsg, "AT+CMGF=1");                                            // Configure the modem for SMS text mode
    Trans_CMD(TrsMsg);
    UART2_Recive();
    SysCtlDelay(1000);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// LCD 16x2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void LCD_Init(void){
    //int i;
    // Enable Ports
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11 |                              // Port M
                         SYSCTL_RCGCGPIO_R9;                                // Port K

    /*--------------- Data Lines LCD -----------------*/
    GPIO_PORTK_DIR_R |= 0xFF;                                               // Port K OUT
    GPIO_PORTK_DEN_R |= 0xFF;                                               // DIGITAL BITS
    /*-------------- Control lines LCD ---------------*/
    GPIO_PORTM_DIR_R |= 0x07;                                               // BITS [0..2] Port M OUT
    GPIO_PORTM_DEN_R |= 0x07;                                               // BITS [0..2] DIGITAL
    /*-------- Configuration 16 CHAR 2 LINES ---------*/
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    GPIO_PORTK_DATA_R = 0x38;                                               // Write lines LCD
    SysCtlDelay(50000);                                                     // Delay 5[ms]
    GPIO_PORTM_DATA_R = 0x00;                                               // Enable -> 0
    SysCtlDelay(50000);                                                     // Delay 5[ms]
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    /*------------------ Clear LCD -------------------*/
    GPIO_PORTK_DATA_R = 0x01;                                               // Write lines LCD
    GPIO_PORTM_DATA_R = 0x00;                                               // Enable -> 0
    SysCtlDelay(50000);                                                     // Delay 5[ms]
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    /*--------------- Blink LCD cursor ---------------*/
    GPIO_PORTK_DATA_R = 0x0E;                                               // Write lines LCD
    GPIO_PORTM_DATA_R = 0x00;                                               // Enable -> 0
    SysCtlDelay(50000);                                                     // Delay 5[ms]
    GPIO_PORTM_DATA_R = 0x04;                                               // Enable -> 1
    GPIO_PORTM_DATA_R = 0x06;                                               // Enable -> 1, RS -> 1
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Write a character to LCD
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void WRITE_LCD(char input_char){
    GPIO_PORTK_DATA_R = input_char;
    GPIO_PORTM_DATA_R = 0x02;                                               // Enable -> 0, RS -> 1
    GPIO_PORTM_DATA_R = 0x06;                                               // Enable -> 1, RS -> 1
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Sends SMS report from the module
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void SendSMSReport(char report[30]){
    int i;

    char ConfigCMD[25] = "AT+CMGS=\"5530159182\"";

    Trans_CMD(ConfigCMD);                                                   // Send the comand and phone number to call

    for(i = 0; i < strlen(report); i++)                                     // Send the status of the system
        UART2_Transmit(report[i]);

    UART2_Transmit(0x1A);   // ^Z
    UART2_Transmit(0X0D);   // <CR>
    UART2_Transmit(0X0A);   // <LF>
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Move cursor left and continue writing
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
int ReadSMS(void){
    int cycle = 0;

    strcpy(UARTMsg, "AT+CMGR=1");                                       // Read SMS in index 1
    Trans_CMD(UARTMsg);

    while(cycle == 0){
        UART2_Recive();
        UART0_Escribe_dato(d_uint8Dato);

        if(d_uint8Dato == 'O'){
            UART2_Recive();
            UART0_Escribe_dato(d_uint8Dato);

            if(d_uint8Dato == 'n'){
                strcpy(UARTMsg, "AT+CMGD=1");                           // Delete SMS in index 1
                Trans_CMD(UARTMsg);
                cycle = 1;
            }
        }
    }

    return cycle;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Move cursor left and continue writing
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void DelChr(void){
    GPIO_PORTM_DATA_R = 0x04;                                       // Enable -> 1
    GPIO_PORTK_DATA_R = 0x13;                                       // Write lines LCD
    GPIO_PORTM_DATA_R = 0x00;                                       // Enable -> 0
    SysCtlDelay(33333);                                             // Delay 5[ms]
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Phone keyborad
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void Key_Init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R14 |                              // Port Q
                         SYSCTL_RCGCGPIO_R7;                                // Port H
    /*---------- Entradas Teclado Matricial ----------*/
    GPIO_PORTH_AHB_DIR_R |= 0x00;                                           // BITS 0 - 3 Puerto H ENTRADAS
    GPIO_PORTH_AHB_DEN_R |= 0x0F;                                           // BITS 0 - 3 DIGITAL
    /*------ Lineas de Control Teclado Matricial -----*/
    GPIO_PORTQ_DIR_R |= 0x0F;                                               // BITS Puerto Q SALIDAS
    GPIO_PORTQ_DEN_R |= 0x0F;                                               // BITS DIGITALES
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// 4x4 Keyboard
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void Kybd4x4(void){
    int line;

    for(line = 1; line < 5; line++){
        if(line == 1){
            GPIO_PORTQ_DATA_R = 0x01;

            if(GPIO_PORTH_AHB_DATA_R == 0x01){
                SysCtlDelay(5000000);
                WRITE_LCD('1');
                PhNum[j] = '1';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x02){
                SysCtlDelay(5000000);
                WRITE_LCD('4');
                PhNum[j] = '4';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x04){
                SysCtlDelay(5000000);
                WRITE_LCD('7');
                PhNum[j] = '7';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x08){
                SysCtlDelay(5000000);
                strcpy(UARTMsg, "ATD");
                strcat(UARTMsg, PhNum);
                strcat(UARTMsg, ";");
                Trans_CMD(UARTMsg);
                j = 0;
                GPIO_PORTN_DATA_R = 0x03;
            }
        }

        else if(line == 2){
            GPIO_PORTQ_DATA_R = 0x02;

            if(GPIO_PORTH_AHB_DATA_R == 0x01){
                SysCtlDelay(5000000);
                WRITE_LCD('2');
                PhNum[j] = '2';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x02){
                SysCtlDelay(5000000);
                WRITE_LCD('5');
                PhNum[j] = '5';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x04){
                SysCtlDelay(5000000);
                WRITE_LCD('8');
                PhNum[j] = '8';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x08){
                SysCtlDelay(5000000);
                WRITE_LCD('0');
                PhNum[j] = '0';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }
        }

        else if(line == 3){
            GPIO_PORTQ_DATA_R = 0x04;

            if(GPIO_PORTH_AHB_DATA_R == 0x01){
                SysCtlDelay(5000000);
                WRITE_LCD('3');
                PhNum[j] = '3';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x02){
                SysCtlDelay(5000000);
                WRITE_LCD('6');
                PhNum[j] = '6';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x04){
                SysCtlDelay(5000000);
                WRITE_LCD('9');
                PhNum[j] = '9';
                j++;
                GPIO_PORTN_DATA_R = 0x03;

                if(j > 10)
                    j = 0;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x08){
                SysCtlDelay(5000000);
                strcat(UARTMsg, "ATH");
                Trans_CMD(UARTMsg);
                GPIO_PORTN_DATA_R = 0x03;
            }
        }

        else if(line == 4){
            GPIO_PORTQ_DATA_R = 0x08;

            if(GPIO_PORTH_AHB_DATA_R == 0x01){
                SysCtlDelay(5000000);
                strcat(UARTMsg, "ATA");
                Trans_CMD(UARTMsg);
                GPIO_PORTN_DATA_R = 0x03;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x02){
                SysCtlDelay(5000000);
                DelChr();
                j--;
                GPIO_PORTN_DATA_R = 0x03;
            }

            else if(GPIO_PORTH_AHB_DATA_R == 0x04){
                SysCtlDelay(5000000);
                WRITE_LCD('#');
                GPIO_PORTN_DATA_R = 0x03;
            }
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// ADC initialization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void ADC_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x10;                                              // 1) Activate clock for port E

    while((SYSCTL_PRGPIO_R & 0x10) == 0);

    GPIO_PORTE_AHB_DIR_R &= ~0x10;                                          // 2) Make PE4 analog input
    GPIO_PORTE_AHB_AFSEL_R |= 0x10;                                         // 3) Enable alternate function on PE4
    GPIO_PORTE_AHB_DEN_R &= ~0x10;                                          // 4) Disable digital I/O on PE4
    GPIO_PORTE_AHB_AMSEL_R |= 0x10;                                         // 5) Enable analog function on PE4

    SYSCTL_RCGCADC_R |= 0x01;                                               // 6) Activate ADC0

    while((SYSCTL_PRADC_R & 0x01) == 0);                                    // Delay for the clock enable

    ADC0_PC_R = 0x01;                                                       // 7) Configure for 125K samp/s^2
    ADC0_SSPRI_R = 0x0123;                                                  // 8) SS3 is highest priority
    ADC0_ACTSS_R &= ~0x0008;                                                // 9) Disable SS3
    ADC0_EMUX_R &= ~0xF000;                                                 // 10) SEQ3 is software trigger
    ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 9;                       // 11) Clear SS3 field and set channel AIN9 (PE4)
    ADC0_SSCTL3_R = 0x0006;                                                 // 12) No TS0 D0; Yes IE0 END0
    ADC0_IM_R &= ~0x0008;                                                   // 13) Disable SS3 interrupts
    ADC0_ACTSS_R |= 0x0008;                                                 // 14) Enable SS3
    ADC0_ISC_R = 0x0008;                                                    // Clear RIS flag from ADC0
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Temperature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
int ADC0_Temp(void){
    uint32_t result;
    int V_temp;

    ADC0_PSSI_R = 0X0008;                                                   // Initiate SS3

    while((ADC0_RIS_R & 0X08) == 0){};                                      // Wait for conversion done

    result = (ADC0_SSFIFO3_R & 0xFFF);                                      // Read 12-Bit result
    ADC0_ISC_R = 0x0008;                                                    // Acknowledge completion
    V_temp = (result*0.082);
    result = ((100*V_temp)/100);
    temp[1] = ((result/10)+48);                                             // Temperature in BCD (Decimals)
    temp[0] = ((result%10)+48);                                             // Temperature in BCD (Units)
    return result;
}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
int main(void){
    // Enable Ports
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12 |                              // Port N
                         SYSCTL_RCGCGPIO_R8;                                // Port J

    uint32_t g_ui32SysClock;
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480),
                                             50000000);

    char LCDMsg[20] = "Temperatura: ",
         RecMsg[100],
         SMS[5];

    int SisTemp,
        k,
        comp = 1,
        u = 0;

    /*----------------- Board Buttons ----------------*/
    GPIO_PORTJ_AHB_DIR_R |= 0x00;                                           // BITS [1..0] Port J IN
    GPIO_PORTJ_AHB_DEN_R |= 0x03;                                           // BITS [1..0] DIGITAL
    GPIO_PORTJ_AHB_PUR_R |= 0x03;                                           // PULL-UP res
    /*------------------ Board LEDS ------------------*/
    GPIO_PORTN_DIR_R |= 0x03;                                               // BITS [1..0] Port N OUT
    GPIO_PORTN_DEN_R |= 0x03;                                               // BITS [1..0] DIGITAL

    /*################################################*/
    /*############ Initial configurations ############*/
    /*################################################*/
    UART_Init();
    Mdm_Init();
    LCD_Init();
    Key_Init();
    ADC_Init();
    /*################################################*/
    while((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R12) == 0x00);                 // Delay to ensure register function

    GPIO_PORTM_DATA_R = 0x06;                                               // Enable -> 1, RS -> 1

    for(k = 0; k < strlen(LCDMsg); k++){
        WRITE_LCD(LCDMsg[k]);
        SysCtlDelay(1000);
    }

    while(1){
        switch(EDO){
            /********************************************/
            /* Initial state                            *
             * Check if the module has receive a SMS    *
             * and if the message text is the string    *
             * "On"                                     */
            /********************************************/
            case 0:{
                GPIO_PORTN_DATA_R = 0x00;
                UART2_Recive();
                UART0_Escribe_dato(d_uint8Dato);
                RecMsg[u] = d_uint8Dato;
                u++;

                if(RecMsg[u-1] == '+'){
                    u = 0;

                    do{
                        UART2_Recive();
                        UART0_Escribe_dato(d_uint8Dato);
                        RecMsg[u] = d_uint8Dato;
                        u++;
                    }while(RecMsg[u-1] != ':');

                    for(cont = 0; cont < 5; cont++)
                        SMS[cont] = RecMsg[cont];

                    comp = strcmp(SMS, "CMTI:");

                    if(comp == 0){
                        GPIO_PORTN_DATA_R = 0x02;
                        EDO = ReadSMS();
                    }
                }

            }break;
            /********************************************/
            /* System Ready to work                     *
             * The message was received successfully and*
             * begin to measure the temperature to      *
             * display in the LCD                       */
            /********************************************/
            case 1:{
                GPIO_PORTN_DATA_R = 0x01;
                GPIO_PORTM_DATA_R = 0x06;                                               // Enable -> 1, RS -> 1
                SisTemp = ADC0_Temp();
                WRITE_LCD(temp[1]);
                SysCtlDelay(100000);
                WRITE_LCD(temp[0]);
                SysCtlDelay(100000);
                WRITE_LCD('C');
                /*------------- Move cursor --------------*/
                for(k = 0; k < 4; k++){
                    GPIO_PORTM_DATA_R = 0x04;                                           // Enable -> 1
                    GPIO_PORTK_DATA_R = 0x13;                                           // Write lines LCD
                    GPIO_PORTM_DATA_R = 0x00;                                           // Enable -> 0
                    SysCtlDelay(500000);
                }
                //EDO = 2;
                SysCtlDelay(1000000);
            }break;
            /********************************************/
            /* System read keyboard                     *
             * Read if a key is pressed in the matrix   *
             * keyboard                                 */
            /********************************************/
            case 2:{
                Kybd4x4();
                EDO = 1;
            }
            /********************************************/
            /* Not implemented yet, for future use      */
            /********************************************/
            case 3:{
                EDO = 0;
            }
        }

    }
}

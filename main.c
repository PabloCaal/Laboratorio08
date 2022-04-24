/* 
 * File:   main.c
 * Author: pjcaa
 *
 * Created on 19 de abril de 2022, 12:01 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT        // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF                   // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF                  // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF                  // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF                     // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                    // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF                  // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF                   // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF                  // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF                    // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V               // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF                    // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 500000

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
uint8_t Van1 = 0;                           // Declaramos la variable con el n�mero de 3 d�gitos a mostrar en los displays
int VALOR[3] = {0, 0, 0}, voltaje;          // En este arreglo se dividir� el n�mero de 3 d�gitos en unidades, decenas y centenas
uint8_t Bandera_multiplexado = 0;           // Variable de bandera para multiplexado de displays

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);                           // Prototipo de funci�n de configuraci�n
void separacion(int a, int b[]);            // Prototipo de funci�n de separaci�n del n�mero de 3 d�gitos
uint8_t tabla(int a);                       // Prototipo de funci�n de tabla de conversi�n para displays
void mostrar(int a[]);                      // Prototipo de funci�n para mostrar valores en displays

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){                      // Verificaci�n de interrupci�n del m�dulo ADC
        if(ADCON0bits.CHS == 0){            // Verificaci�n de canal AN0
            PORTB = ADRESH;                 // Mostrar el resgitro ADRESH en PORTC
        }
        else if (ADCON0bits.CHS == 1){      // Verificaci�n de canal AN1
            Van1 = ADRESH;                  // Mostrar el resgitro ADRESH en PORTD
        }
        PIR1bits.ADIF = 0;                  // Limpieza de bandera de interrupci�n
    } 
    else if(INTCONbits.T0IF){               // Verificaci�n de interrupci�n del TIMER0 
        mostrar(VALOR);                     // Llamado a funci�n para mostrar en displays
        Bandera_multiplexado++;             // Incremento de variable de bandera para multiplexado
        if(Bandera_multiplexado>2){         // Si la bandera es mayor a 2 reiniciar
            Bandera_multiplexado = 0;
        }        
        INTCONbits.T0IF = 0;                // Limpieza de bandera de interrupci�n del TIMER0
        TMR0 = 254;                         // Ingreso de n�mero correspondiente para retardo del TIMER0
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void){
    setup();    
    while(1){        
        voltaje = (Van1)*(0.0196)*100;          // Transformaci�n de voltaje de binario a decimal
        separacion(voltaje, VALOR);             // Llamado de funci�n de separaci�n de d�gitos del n�mero
        if(ADCON0bits.GO == 0){                 // Si no hay proceso de conversi�n           
            if(ADCON0bits.CHS == 0b0000)        // Cannal AN0 activo
                ADCON0bits.CHS = 0b0001;        // Cambio de canal
            else if(ADCON0bits.CHS == 0b0001)   // Canal AN1 activo
                ADCON0bits.CHS = 0b0000;        // Cambio de canal
            __delay_us(40);                     // Delay de tiempo de adquisici�n
            ADCON0bits.GO = 1;                  // Ejecuci�n de proceso de conversi�n
        } 
    }
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSEL = 0b00000011;             // AN0 y AN1 como I/O anal�gicas
    ANSELH = 0;                     // I/O digitales
    TRISA = 0b00000011;             // AN0 y AN1 como entradas
    TRISB = 0x00;                   // Habilitaci�n de PORTB como salida
    TRISC = 0x00;                   // Habilitaci�n de PORTC como salida
    TRISD = 0x00;                   // Habilitaci�n de PORTD como salida
    PORTA = 0x00;                   // Limpieza del PORTA
    PORTB = 0x00;                   // Limpieza del PORTB
    PORTC = 0x00;                   // Limpieza del PORTC
    PORTD = 0x00;                   // Limpieza del PORTD

     // Configuracion interrupciones
    PIR1bits.ADIF = 0;              // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;              // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;            // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;             // Habilitamos int. globales
    INTCONbits.T0IE = 1;            // Habilitaci�n de interrupciones del TIMER0
    INTCONbits.T0IF = 0;            // Limpieza bandera de interrupci�n del TIMER0
      
    // Configuraci�n ADC
    ADCON0bits.ADCS = 0b01;         // Fosc/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selecci�n de canal AN0
    ADCON0bits.CHS = 0b0001;        // Selecci�n de canal AN1
    ADCON1bits.ADFM = 0;            // Configuraci�n de justificado a la izquierda
    ADCON0bits.ADON = 1;            // Habilitaci�n del modulo ADC
    __delay_us(40);                 // Display de sample time
        
    // Configuraci�n de TIMER0
    OPTION_REGbits.T0CS = 0;        // Configuraci�n del TIMER0 como temporizador
    OPTION_REGbits.PSA = 0;         // Configuraci�n del Prescaler para el TIMER0 y no para el Watchdog timer
    OPTION_REGbits.PS = 0b110;      // Prescaler de 1:128 PS<2:0> = 110
    
    /* C�lculo del valor a ingresar al TIMER0 para que tenga retardo de 2 ms
	; N = 256 - (Temp/(4 x Tosc x Presc))
	; N = 256 - (2 ms/(4 x (1/500 kHz) x 128))
	; N = 254
    */
    
    TMR0 = 254;
 
    // Configuraci�n del oscilador interno del PIC
    OSCCONbits.IRCF = 0b011;        // Configuraci�n de frecuencia de oscilador a 500kHz
    OSCCONbits.SCS = 1;             // Oscilador interno
}

/*------------------------------------------------------------------------------
 * FUNCIONES 
 ------------------------------------------------------------------------------*/
// Funci�n de separaci�n de un n�mero en unidades, decenas y centenas
void separacion(int a, int b[]){      
    b[1] = a/100;                               // Centenas
    b[2] = (a-(100*(b[1])))/10;                 // Decenas
    b[3] = (a-(100*(b[1])+10*(b[2])));          // Unidades
    return;
}

// Funci�n para mostrar los valores en los distintos displays
void mostrar(int a[]){
    PORTD = 0x00;
    PORTC = 0x00;
    switch(Bandera_multiplexado){
        case 0:                                 // Display de centenas
            PORTDbits.RD2 = 1;                  // Activaci�n de bit RD2
            PORTC = tabla(a[1])+(0x80);         // Conversi�n de d�gito de centenas
            break;
        case 1:                                 // Display de decenas
            PORTDbits.RD1 = 1;                  // Activaci�n de bit RD1
            PORTC = tabla(a[2]);                // Conversi�n de d�gito de decenas 
            break;  
        case 2:                                 // Display de unidades
            PORTDbits.RD0 = 1;                  // Activaci�n de bit RD0
            PORTC = tabla(a[3]);                // Conversi�n de d�gito de unidades
            break;
        default:
            break;
    }
    return;
}

// Funci�n de tabla para conversi�n de n�meros para mostrar en display de 7 segmentos
uint8_t tabla(int a){
    uint8_t valor = 0;
    switch(a){
        case 0:                 // N�mero = 0
            valor = 0x3F;       // Valor para mostrar 0 en display de 7 segmentos
            break;
        case 1:                 // N�mero = 1
            valor = 0x06;       // Valor para mostrar 1 en display de 7 segmentos
            break;
        case 2:                 // N�mero = 2
            valor = 0x5B;       // Valor para mostrar 2 en display de 7 segmentos
            break;
        case 3:                 // N�mero = 3
            valor = 0x4F;       // Valor para mostrar 3 en display de 7 segmentos
            break;
        case 4:                 // N�mero = 4
            valor = 0x66;       // Valor para mostrar 4 en display de 7 segmentos
            break;
        case 5:                 // N�mero = 5
            valor = 0x6D;       // Valor para mostrar 5 en display de 7 segmentos
            break;
        case 6:                 // N�mero = 6
            valor = 0x7D;       // Valor para mostrar 6 en display de 7 segmentos
            break;
        case 7:                 // N�mero = 7
            valor = 0x07;       // Valor para mostrar 7 en display de 7 segmentos
            break;
        case 8:                 // N�mero = 8
            valor = 0x7F;       // Valor para mostrar 8 en display de 7 segmentos
            break;
        case 9:                 // N�mero = 9
            valor = 0x6F;       // Valor para mostrar 9 en display de 7 segmentos
            break;
    }
    return valor;               // Retorno de valor ingresador para mostrar en display
}


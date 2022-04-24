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
uint8_t Van1 = 0;                           // Declaramos la variable con el número de 3 dígitos a mostrar en los displays
int VALOR[3] = {0, 0, 0}, voltaje;          // En este arreglo se dividirá el número de 3 dígitos en unidades, decenas y centenas
uint8_t Bandera_multiplexado = 0;           // Variable de bandera para multiplexado de displays

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);                           // Prototipo de función de configuración
void separacion(int a, int b[]);            // Prototipo de función de separación del número de 3 dígitos
uint8_t tabla(int a);                       // Prototipo de función de tabla de conversión para displays
void mostrar(int a[]);                      // Prototipo de función para mostrar valores en displays

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){                      // Verificación de interrupción del módulo ADC
        if(ADCON0bits.CHS == 0){            // Verificación de canal AN0
            PORTB = ADRESH;                 // Mostrar el resgitro ADRESH en PORTC
        }
        else if (ADCON0bits.CHS == 1){      // Verificación de canal AN1
            Van1 = ADRESH;                  // Mostrar el resgitro ADRESH en PORTD
        }
        PIR1bits.ADIF = 0;                  // Limpieza de bandera de interrupción
    } 
    else if(INTCONbits.T0IF){               // Verificación de interrupción del TIMER0 
        mostrar(VALOR);                     // Llamado a función para mostrar en displays
        Bandera_multiplexado++;             // Incremento de variable de bandera para multiplexado
        if(Bandera_multiplexado>2){         // Si la bandera es mayor a 2 reiniciar
            Bandera_multiplexado = 0;
        }        
        INTCONbits.T0IF = 0;                // Limpieza de bandera de interrupción del TIMER0
        TMR0 = 254;                         // Ingreso de número correspondiente para retardo del TIMER0
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void){
    setup();    
    while(1){        
        voltaje = (Van1)*(0.0196)*100;          // Transformación de voltaje de binario a decimal
        separacion(voltaje, VALOR);             // Llamado de función de separación de dígitos del número
        if(ADCON0bits.GO == 0){                 // Si no hay proceso de conversión           
            if(ADCON0bits.CHS == 0b0000)        // Cannal AN0 activo
                ADCON0bits.CHS = 0b0001;        // Cambio de canal
            else if(ADCON0bits.CHS == 0b0001)   // Canal AN1 activo
                ADCON0bits.CHS = 0b0000;        // Cambio de canal
            __delay_us(40);                     // Delay de tiempo de adquisición
            ADCON0bits.GO = 1;                  // Ejecución de proceso de conversión
        } 
    }
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSEL = 0b00000011;             // AN0 y AN1 como I/O analógicas
    ANSELH = 0;                     // I/O digitales
    TRISA = 0b00000011;             // AN0 y AN1 como entradas
    TRISB = 0x00;                   // Habilitación de PORTB como salida
    TRISC = 0x00;                   // Habilitación de PORTC como salida
    TRISD = 0x00;                   // Habilitación de PORTD como salida
    PORTA = 0x00;                   // Limpieza del PORTA
    PORTB = 0x00;                   // Limpieza del PORTB
    PORTC = 0x00;                   // Limpieza del PORTC
    PORTD = 0x00;                   // Limpieza del PORTD

     // Configuracion interrupciones
    PIR1bits.ADIF = 0;              // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;              // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;            // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;             // Habilitamos int. globales
    INTCONbits.T0IE = 1;            // Habilitación de interrupciones del TIMER0
    INTCONbits.T0IF = 0;            // Limpieza bandera de interrupción del TIMER0
      
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;         // Fosc/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selección de canal AN0
    ADCON0bits.CHS = 0b0001;        // Selección de canal AN1
    ADCON1bits.ADFM = 0;            // Configuración de justificado a la izquierda
    ADCON0bits.ADON = 1;            // Habilitación del modulo ADC
    __delay_us(40);                 // Display de sample time
        
    // Configuración de TIMER0
    OPTION_REGbits.T0CS = 0;        // Configuración del TIMER0 como temporizador
    OPTION_REGbits.PSA = 0;         // Configuración del Prescaler para el TIMER0 y no para el Watchdog timer
    OPTION_REGbits.PS = 0b110;      // Prescaler de 1:128 PS<2:0> = 110
    
    /* Cálculo del valor a ingresar al TIMER0 para que tenga retardo de 2 ms
	; N = 256 - (Temp/(4 x Tosc x Presc))
	; N = 256 - (2 ms/(4 x (1/500 kHz) x 128))
	; N = 254
    */
    
    TMR0 = 254;
 
    // Configuración del oscilador interno del PIC
    OSCCONbits.IRCF = 0b011;        // Configuración de frecuencia de oscilador a 500kHz
    OSCCONbits.SCS = 1;             // Oscilador interno
}

/*------------------------------------------------------------------------------
 * FUNCIONES 
 ------------------------------------------------------------------------------*/
// Función de separación de un número en unidades, decenas y centenas
void separacion(int a, int b[]){      
    b[1] = a/100;                               // Centenas
    b[2] = (a-(100*(b[1])))/10;                 // Decenas
    b[3] = (a-(100*(b[1])+10*(b[2])));          // Unidades
    return;
}

// Función para mostrar los valores en los distintos displays
void mostrar(int a[]){
    PORTD = 0x00;
    PORTC = 0x00;
    switch(Bandera_multiplexado){
        case 0:                                 // Display de centenas
            PORTDbits.RD2 = 1;                  // Activación de bit RD2
            PORTC = tabla(a[1])+(0x80);         // Conversión de dígito de centenas
            break;
        case 1:                                 // Display de decenas
            PORTDbits.RD1 = 1;                  // Activación de bit RD1
            PORTC = tabla(a[2]);                // Conversión de dígito de decenas 
            break;  
        case 2:                                 // Display de unidades
            PORTDbits.RD0 = 1;                  // Activación de bit RD0
            PORTC = tabla(a[3]);                // Conversión de dígito de unidades
            break;
        default:
            break;
    }
    return;
}

// Función de tabla para conversión de números para mostrar en display de 7 segmentos
uint8_t tabla(int a){
    uint8_t valor = 0;
    switch(a){
        case 0:                 // Número = 0
            valor = 0x3F;       // Valor para mostrar 0 en display de 7 segmentos
            break;
        case 1:                 // Número = 1
            valor = 0x06;       // Valor para mostrar 1 en display de 7 segmentos
            break;
        case 2:                 // Número = 2
            valor = 0x5B;       // Valor para mostrar 2 en display de 7 segmentos
            break;
        case 3:                 // Número = 3
            valor = 0x4F;       // Valor para mostrar 3 en display de 7 segmentos
            break;
        case 4:                 // Número = 4
            valor = 0x66;       // Valor para mostrar 4 en display de 7 segmentos
            break;
        case 5:                 // Número = 5
            valor = 0x6D;       // Valor para mostrar 5 en display de 7 segmentos
            break;
        case 6:                 // Número = 6
            valor = 0x7D;       // Valor para mostrar 6 en display de 7 segmentos
            break;
        case 7:                 // Número = 7
            valor = 0x07;       // Valor para mostrar 7 en display de 7 segmentos
            break;
        case 8:                 // Número = 8
            valor = 0x7F;       // Valor para mostrar 8 en display de 7 segmentos
            break;
        case 9:                 // Número = 9
            valor = 0x6F;       // Valor para mostrar 9 en display de 7 segmentos
            break;
    }
    return valor;               // Retorno de valor ingresador para mostrar en display
}


/* 
 * File:   mainE2.c
 * Author: Luiz Felix
 *
 * Created on 14 de Dezembro de 2020, 23:40
 */

#define _XTAL_FREQ 4000000 // Oscilador de 4 MHz

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#pragma config FOSC = HS // Oscilador externo
#pragma config WDT = OFF // Watchdog Timer desligado
#pragma config MCLRE = OFF // Master Clear desabilitado
long contador;
float temperatura;

void setupADC(void) {
    TRISA = 0b00000001; // Habilita pino A0 como entrada

    ADCON2bits.ADCS = 0b110; // Clock do AD: Fosc/64
    ADCON2bits.ACQT = 0b010; // Tempo de aquisição: 4 Tad
    ADCON2bits.ADFM = 0b1; // Formato: à direita
    ADCON1bits.VCFG = 0b01; // Tensões de referência: Vss e Pino AN3

    ADCON0 = 0; // Seleciona o canal AN0
    ADCON0bits.ADON = 1; // Liga o AD
}

void inicializa_RS232() {
    RCSTA = 0X90; // Habilita porta serial, recepção de 8 bits em modo continuo, assíncrono.
    int valor;
    TXSTA = 0X24; // modo assíncrono, transmissão 8 bits.
    valor = (int) (((_XTAL_FREQ / 9600) - 16) / 16); // valor para gerar o baud rate
    SPBRG = valor; // esse registrador, carregado com o ?valor? calculado, define o baud rate
    //(pois corre se o risco de ter uma interrupção escrita e leitura ao mesmo tempo)
}

void escreve(char valor) {
    TXIF = 0; // limpa flag que sinaliza envio completo.
    TXREG = valor; // Envia caractere desejado à porta serial
    while (TXIF == 0); // espera caractere ser enviado
}

void imprime(char frase[]) {
    int indice = 0; // índice da cadeia de caracteres
    int tamanho = 0;
    tamanho = strlen(frase); // tamanho total da cadeia a ser impressa
    while (indice < tamanho) { // verifica se todos foram impressos
        escreve(frase[indice]); // Chama rotina que escreve o caractere
        indice++; // incrementa índice
    }
}

void main(void) {
    TRISB = 0X02; // configura portB B1 (pino RX) como entrada
    TRISB = 0b00000010;
    LATB = 0b01000000;
    inicializa_RS232();
    setupADC();
    float aux = 0; // para evitar ficar repetindo o mesmo valor na serial
    char text[100];
    while (1) { // Inicia loop infinito
        ADCON0bits.GO = 1; // Inicia a conversão A/D
        while (!ADCON0bits.GO) { // Aguarda fim da conversão
        }
        contador = (ADRESH * 0x100) + ADRESL; // Transfere valor para variável
        temperatura = ((1.5 * 100 * contador) / 1023.0); // Calcula temperatura

        if (temperatura < 30 && temperatura != aux) {
            sprintf(text, "Temperatura Baixa %.1f C \n\r", temperatura);
            imprime(text);
            aux = temperatura;
        }
        
        if ((temperatura >= 30 && temperatura < 100) && temperatura != aux) {
            sprintf(text, "Temperatura Adequada %.1f C \n\r", temperatura);
            imprime(text);
            aux = temperatura;
        }
        
        if (temperatura >= 100) {
            if (temperatura != aux) {
                sprintf(text, "Temperatura muito alta %.1f C \n\r", temperatura);
                imprime(text);
                aux = temperatura;
            }
            
            if (LATB == 0b00001000) {
                LATB = 0b10000000;
            }
            __delay_ms(50);
            LATB = LATB >> 1;

        }
    }
    return;
}

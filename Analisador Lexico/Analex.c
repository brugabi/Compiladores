#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Analex.h"

#define TAM_LEXEMA 31
#define TAM_NUM 30

void error(char msg[], int contLinha) {
    printf("%s%d\n", msg, contLinha);
    exit(1);
}


TOKEN AnaLex (FILE *fd) {
    int estado = 0;
    char lexema[TAM_MAX_LEXEMA] = "";
    int tamL = 0;
    char digitos[TAM_NUM] = "";
    int tamD = 0;
    TOKEN t;

    while (1){
        char c = fgetc(fd);
        switch (estado){
        case 0:
            if (c == ' ' || c == '\t') estado = 0;
            //SINAIS
            else if (c == '+') { estado = 1; t.cat = SN; t.codSN = ADICAO; return t;}
            else if (c == '-') { estado = 2; t.cat = SN; t.codSN = SUBTRACAO; return t;}
            else if (c == '*') { estado = 3; t.cat = SN; t.codSN = MULTI; return t;}
            else if (c == '/') { estado = 4;} //divisao ou comentariio
            else if (c == '>') { estado = 7;} //maior ou maior igual
            else if (c == '<') { estado = 10;} //menor ou menor igual
            else if (c == '=') {estado = 13;} //atribuicao ou igualdade
            else if (c == '(') { estado = 16; t.cat = SN; t.codSN = ABRE_PAR; return t;}
            else if (c == ')') { estado = 17; t.cat = SN; t.codSN = FECHA_PAR; return t;}
            else if (c == '[') { estado = 18; t.cat = SN; t.codSN = ABRE_COL; return t;}
            else if (c == ']') { estado = 19; t.cat = SN; t.codSN = FECHA_COL; return t;}
            else if (c == '|') {estado = 20;} //talvez operador ou
            else if (c == '&') { estado = 22;} // operador and ou endereco
            else if (c == ',') { estado = 25; t.cat = SN; t.codSN = VIRGULA; return t;}
            else if (c == '!') { estado = 26;} //diferenca ou negacao
            else if (c == EOF) { t.cat = FIM_ARQ; return t; } // FIM DO ARQUIVO
            else if (c == '\n') contLinha++; //FIM DE LINHA
            //ID
            else if (c == '_') {
                estado = 29;
                lexema[tamL++] = c;
                lexema[tamL] = '\0';
            }
            else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                estado = 30;
                lexema[tamL++] = c;
                lexema[tamL] = '\0';
            }
            //DIGITO
            else if (c >= '0' && c <='9') {
                estado = 32;
                digitos[tamD++] = c;
                digitos[tamD]= '\0';
            }
            //CHAR
            else if (c == '\'')estado = 37; //pode ser um char, um \n ou um \0
            //STRING
            else if (c == '"') estado = 43; //ESTADO PARA STRING
            else error("CARACTER INVALIDO NA LINHA ", contLinha);
            break;
        case 4: //COMENTARIO OU DIVISAO
            if (c == '/') estado = 5; // COMENTARIO
            else {
                //DIVISAO
                estado = 6;
                ungetc(c,fd);
                t.cat = SN;
                t.codSN = DIV;
                return t;
            }
            break;
        case 5: //COMENTARIO
            if (c == '\n') {
                estado = 0;
                ungetc(c,fd);
            }
            break;
        case 7:
            if (c == '=') {estado = 8; t.cat = SN; t.codSN = MAIOR_IGUAL; return t;} //MAIOR IGUAL
            else {estado = 9; ungetc(c,fd); t.cat = SN; t.codSN = MAIOR; return t;} //MAIOR
            break;
        case 10:
            if (c == '=') {estado = 11; t.cat = SN; t.codSN = MENOR_IGUAL; return t;} //MENOR IGUAL
            else {estado = 12; ungetc(c,fd); t.cat = SN; t.codSN = MENOR; return t;} //MENOR
            break;
        case 13: //ATRIB OU IGUALDADE
            if (c == '=') {estado = 14; t.cat = SN; t.codSN = IGUALDADE; return t;} //IGUALDADE
            else {estado = 15; ungetc(c,fd); t.cat = SN; t.codSN = ATRIB; return t;} //ATRIBUICAO
            break;
        case 20:
            if (c == '|') {estado = 21; t.cat = SN; t.codSN = OP_OR; return t;} //OPERADOR OR
            else error("CARACTER INVALIDO NA LINHA ", contLinha);
            break;
        case 22:
            if (c == '&') {estado = 23; t.cat = SN; t.codSN = OP_AND; return t;} //OPERADOR AND
            else { // E COMERCIAL
                estado = 24;
                ungetc(c, fd);
                t.cat = SN;
                t.codSN = ECOM;
                return t;
            }
            break;
        case 26:
            if (c == '=') {estado = 27; t.cat = SN; t.codSN = DIF; return t;} //DIFERENCA
            else {estado = 28; ungetc(c,fd); t.cat = SN; t.codSN = NEGACAO; return t;} //NEGACAO
            break;
        //CASOS DE ID
        case 29: //UNDERLINE OPCIONAL
            if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                estado = 30;
                lexema[tamL++] = c;
                lexema[tamL] = '\0'; 
            }
            else {
                estado = 29;
                lexema[tamL++] = c;
                lexema[tamL] = '\0'; 
            }
            break;
        case 30: //CASO NECESSÁRIO DE LETRA
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
                lexema[tamL++] = c;
                lexema[tamL] = '\0'; 
            }
            else{
                estado = 31; //ACABOU O ID
                ungetc(c, fd);
                int checkPR = is_PR(lexema);
                //VERIFICAR PALAVRA RESERVADA
                if (checkPR >= 0) {
                    t.cat = PALAVRAS_RESERVADAS;
                    t.codPR = checkPR;
                    return t;
                }
                else {t.cat = ID;}
                strcpy(t.lexema, lexema);
                return t;
            }
            break;
        case 32: //INT OU REAL
            if (c >= '0' && c <= '9') {
                estado = 32;
                digitos[tamD++] = c;
                digitos[tamD]= '\0';
            }
            else if (c == '.') { //REAL
                estado=34;
                digitos[tamD++] = c;
                digitos[tamD]= '\0';
            }
            else { //ACABOU O INT
                estado = 33;
                ungetc(c, fd);
                t.cat = CT_I;
                t.valINT = atoi(digitos);
                return t;
            }
            break;
        case 34: //CASO DO REAL
            if (c >= '0' && c <= '9') {
                estado = 35;
                digitos[tamD++] = c;
                digitos[tamD]= '\0';
            }
            break;
        case 35:
            if (c >= '0' && c <= '9') {
                estado = 35;
                digitos[tamD++] = c;
                digitos[tamD]= '\0';
            }
            else { //ACABOU O REAL
                estado = 36;
                ungetc(c, fd);
                t.cat = CT_R;
                t.valREAL = atof(digitos);
                return t; 
            }
            break;
        case 37: //CHAR
            if (c == '\\') { estado = 40; } //CHAR COM \n ou \0
            else if (isprint(c) && c != '\'') { //CHAR COM QUALQUER CACATER ASCII MENOS O APOSTOFRO
                estado = 38;
                lexema[tamL++] = c;
                lexema[tamL] = '\0'; 
            }
            break;
        case 38: 
            if(c =='\'') { //ACABOU O CHAR COM 1 CARACTER
            estado = 39;
            t.cat = CT_C;
            t.charcon = lexema[0];
            return t;
            }
            else error("ERROR: LEXEMA CHAR INVALIDO NA LINHA NA LINHA: ", contLinha);
            break;
        case 40:
            if (c == 'n') estado = 41;  //CHAR COM \n
            else if (c == '0') estado = 42; //CHAR COM \0
            else error("ERROR: LEXEMA CHAR INVALIDO NA LINHA NA LINHA: ", contLinha);
            break;
        case 41: 
            if (c =='\'') { //ACABOU O CHAR COM \n
                estado=45; 
                t.cat = CT_C;
                t.charcon = '\n';
                return t;
                } 
            else error("ERROR: LEXEMA CHAR INVALIDO NA LINHA NA LINHA: ", contLinha);
            break;
        case 42: 
            if (c =='\'') {
                estado=46; //ACABOU O CHAR COM \0
                t.cat = CT_C;
                t.charcon = '\0'; 
                return t;
                }
            else error("ERROR: LEXEMA CHAR INVALIDO NA LINHA NA LINHA:", contLinha);
            break;
        case 43: //STRING
            if (c == '"') {
                estado = 44;
                t.cat = LT;
                strcpy(t.lexema, lexema);
                return t;
            }
            else if(isprint(c) && c != '\n') { //ACABOU A STRING
                estado = 43;
                lexema[tamL++] = c;
                lexema[tamL] = '\0';
            }
            else error("ERROR: LEXEMA STRING INVALIDO NA LINHA NA LINHA:", contLinha);
            break;
        }
    }
    
}

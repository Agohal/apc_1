/* Universidade de Brasilia
 * Instituto de Ciencias Exatas
 * Departamento de Ciencia da Computacao
 * 
 * Algoritmos e Programacao de Computadores - 1/2018
 * 
 * Aluno (a): Alberto Tavares Duarte Neto
 * Matricula: 180011707]
 * Turma: A
 * Versao do compilador: -------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

/* Mensagem do menu */
#define MENU "1 - Jogar\n2 - Configuracoes\n3 - Ranking\n4 - Instrucoes\n5 - Sair\n\nEscolha uma opcao: "

/* Escolhas menu*/
#define MENU_JOGAR '1'
#define MENU_CONFIG '2'
#define MENU_RANKING '3'
#define MENU_INSTRUC '4'
#define MENU_SAIR '5'

/* Mensagem Instrucoes */
#define INSTRUC_STRING "essas sao as intrucoes poc\n\n5 - Sair\n\nEscolha uma opcao: "

/* kgbit, getch implementation */
#ifndef _WIN32
    int kbhit() {
        struct termios oldt, newt;
        int ch, oldf;
        tcgetattr(STDIN_FILENO,&oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        if(ch != EOF) {
            ungetc(ch,stdin);
            return 1;
        }
        return 0;
    }
    int getch(void) {
        int ch;
        struct termios oldt;
        struct termios newt;
        tcgetattr(STDIN_FILENO,&oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#else
    #include <conio.h>
#endif

/* Macro for clearing in both Windows and Linux */
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

char tabuleiro[10][135];
int altura, largura;
int probX, probF;
int velocidade;

void limparTela() {
    system(CLEAR);
}

void print_matriz() {
	/*TODO*/
}

int jogar() {
	/*TODO*/
}

int instruc() {
	int input;	
	
	while(1) {	
		limparTela();
		
		printf("%s", INSTRUC_STRING);
		input = getch();
	

		switch(input) {
			case MENU_SAIR:
				return 1;
		}
	}
	
}

int menu() {
	/* Mostra o menu principal do jogo*/
	int input;
    limparTela();
    
    printf("%s", MENU);
	input = getch();
	
	switch(input) {
		case MENU_JOGAR:
			return 1;
		case MENU_CONFIG:
			return 1;
		case MENU_RANKING:
			return 1;
		case MENU_INSTRUC:
			return instruc();
		case MENU_SAIR:
			return 0;
	}
    return 0;
}

int main() {
    int continuar = 1;
    int enter;
    
    do {
        /* Mensagem de Boas vindas */
        limparTela();
        printf("Bem vindo ao jogo Boladeixon!\nAperte enter para entrar no menu");        
       enter = getch(); 
    }
    while(enter != 10);
    
    while(continuar) {
        continuar = menu();
    }
    
    srand(time(0));
    return 0;
}
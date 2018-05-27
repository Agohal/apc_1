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

/* Tamanho tabuleiro */
#define TAMANHO_X 135
#define TAMANHO_Y 10

/* Mensagem do menu */
#define MENU "1 - Jogar\n2 - Configuracoes\n3 - Ranking\n4 - Instrucoes\n5 - Sair\n\nEscolha uma opcao: "

/* Escolhas menu*/
#define MENU_JOGAR '1'
#define MENU_CONFIG '2'
#define MENU_RANKING '3'
#define MENU_INSTRUC '4'
#define MENU_SAIR '5'

/* Controles jogo */
#define CIMA 'w'
#define CIMA_UPPER 'W'
#define BAIXO 's'
#define BAIXO_UPPER 'S'
#define DIREITA 'd'
#define DIREITA_UPPER 'D'
#define ESQUERDA 'a'
#define ESQUERDA_UPPER 'A'
#define TIRO 107

/* Cores */
#define VERMELHO "\033[22;31m"
#define VERDE "\033[22;32m"
#define AZUL "\033[22;34m"
#define AMARELO "\033[01;33m"
#define MAGENTA "\033[22;35m"
#define CYAN "\033[22;36m"
#define LIMITE "\033[01;36m" /* Limites do mapa, cor: Light Cyan */
#define BRANCO "\033[01;37m" /* Padrao */ 

/* Objetos: inimigos, combustível e vazio */

/* TODO
#define INIMIGO_FRACO {'X',VERMELHO,1,1}
#define INIMIGO_MEDIO {'X',AMARELO,2,1}
#define INIMIGO_FORTE {'X',MAGENTA,3,1}
#define JOGADOR {'+',CYAN,1,1}
#define COMBUSTIVEL {'F',VERDE,1,-1}
*/
#define VAZIO_SPRITE 32

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

typedef struct {
	/* Objeto que faz parte do tabuleiro */
	char sprite;
	int vidas; /* -1 significa indestrutível */
	/* TODO: mudar nome da variavel 'matar' */
	int matar; /* caso 1, faz o jogador perder o jogo em contato
					caso -1, faz o jogador ganhar +40 de combustível em contato 
					caso 0, e espaço vazio*/
}Obj;

Obj tabuleiro[TAMANHO_X][TAMANHO_Y];
int probX, probF;
int velocidade = 50;

void limparTela() {
    system(CLEAR);
}

void ini_tile_vazio(int pos_x, int pos_y) {
	/* inicializa tile com vazio 
	   vazio.vidas e vazio.matar sao, por padrao, 0 */
	tabuleiro[pos_x][pos_y].sprite = VAZIO_SPRITE;
	tabuleiro[pos_x][pos_y].vidas = 0;
	tabuleiro[pos_x][pos_y].matar = 0;
}

void ini_tabuleiro() {
	/* inicializa tabuleiro com vazios, sendo matar, vidas = 0 */
	int x, y;
	for(x = 0; x < TAMANHO_X; x++) {
		for(y = 0; y < TAMANHO_Y; y++) {
			ini_tile_vazio(x, y);
		}
	}
}

void atirar(int pos_x_jgdr, int pos_y_jgdr) {
	/* spawna tiro um tile na frente do jogador */
	if(pos_x_jgdr+1 <= TAMANHO_X) {
		tabuleiro[pos_x_jgdr+1][pos_y_jgdr].sprite = '>'; /* Obj tiro TODO: MACRO */
		tabuleiro[pos_x_jgdr+1][pos_y_jgdr].vidas = 1;
		tabuleiro[pos_x_jgdr+1][pos_y_jgdr].matar = 1;
	}
}

void atualizar_pos(int pos_x_atual, int pos_y_atual, int nova_pos_x, int nova_pos_y ) {
	/* coloca objeto em nova posicao e troca posição atual por vazio 
		TODO: checkar nova posicao por colisao e caso necessario chamar funcao colisao */
	tabuleiro[nova_pos_x][nova_pos_y].sprite = tabuleiro[pos_x_atual][pos_y_atual].sprite;
	tabuleiro[nova_pos_x][nova_pos_y].vidas = tabuleiro[pos_x_atual][pos_y_atual].vidas;
	tabuleiro[nova_pos_x][nova_pos_y].matar = tabuleiro[pos_x_atual][pos_y_atual].matar;
	ini_tile_vazio(pos_x_atual, pos_y_atual);
}

void input_jogo(int input, int *pos_x_jgdr, int *pos_y_jgdr) {
	/* Recebe o input do jogador e faz a jogada de acordo 
		TODO: caso jogador nao de input ou aperte algum botao aleatorio */
	switch(input) {
		case(TIRO):
			atirar(*pos_x_jgdr, *pos_y_jgdr);
			
		/* movimentacao y-axis */	
		if((CIMA || CIMA_UPPER) && *pos_y_jgdr+1 < TAMANHO_Y) {
			atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr, *pos_y_jgdr+1);
			*pos_y_jgdr+=1;
		}

		else if((BAIXO || BAIXO_UPPER) && *pos_y_jgdr-1 > 0) {
			atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr, *pos_y_jgdr-1);
			*pos_y_jgdr-=1;
		}
			
		/* movimentacao x-axis */	
		else if((DIREITA || DIREITA_UPPER) && *pos_x_jgdr+1 < TAMANHO_Y) {
			atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr+1, *pos_y_jgdr);
			*pos_x_jgdr+=1;
		}
			
		else if((ESQUERDA || ESQUERDA_UPPER) && *pos_x_jgdr-1 >=0) {
			atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr-1, *pos_y_jgdr);
			*pos_x_jgdr-=1;
		}
			
	}
}

void print_matriz() {
	/* printa limite superior */
	int i, j;
	limparTela();
	
	for(i = 0; i < TAMANHO_X; i++) {
		printf(LIMITE);
		printf("#");
	}
	
	printf("\n");
	printf(BRANCO);
	for(i = 0; i < TAMANHO_Y; i++) {
		for(j = 0; j < TAMANHO_X; j++) {
			printf("%c", tabuleiro[TAMANHO_X][TAMANHO_Y].sprite);
		}
		printf("\n");
	}
	
	/* printa limite inferior */
	for(i = 0; i < TAMANHO_X; i++) {
		printf(LIMITE);
		printf("#");
	}
}

int colisao() {
	/* TODO */
	return 0;
}

int jogar() {
	/* inicia um novo jogo */
	/* TODO: ponteiro com posicao do jogador */
	int jogador_pos[1][2] = {{0, TAMANHO_Y/2}};
	
	ini_tabuleiro();
	print_matriz();
	getch();
	return 0;
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
			return jogar();
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
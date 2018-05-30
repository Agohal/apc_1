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
#define TAMANHO_Y 135
#define TAMANHO_X 10-1

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

/* Quantidade combustivel extra por F */
#define COMBS_EXTRA 40

/* Cores */
#define VERMELHO "\033[22;31m"
#define VERDE "\033[22;32m"
#define AZUL "\033[22;34m"
#define AMARELO "\033[01;33m"
#define MAGENTA "\033[22;35m"
#define CYAN "\033[22;36m"
#define COR_LIMITE "\033[01;36m" /* Limites do mapa, cor: Light Cyan */
#define BRANCO "\033[01;37m" /* Padrao */ 

/* TODO
#define INIMIGO_FRACO {'X',VERMELHO,1,1}
#define INIMIGO_MEDIO {'X',AMARELO,2,1}
#define INIMIGO_FORTE {'X',MAGENTA,3,1}
#define JOGADOR {'+',CYAN,1,1}
#define COMBUSTIVEL {'F',VERDE,1,-1}
*/
#define VAZIO_SPRITE ' '

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
	int vida; /* cada 'hit' sao -50 caso tipo seja 0. Pro jogador se chama combustivel. Caso 0, objeto e destruido */
	int tipo; /* Positivo: destruido em contato
						  Negativo: Nao e destruido em contato
						  0: tile vazio 
						  1 - Tiro, 2 - Combustivel Extra, 3 - Jogador 
						  -1 - Inimigo Fraco, -2 - Inimigo Medio, -3 - Inimigo Forte*/
}Obj; 

Obj tabuleiro[TAMANHO_Y][TAMANHO_X];
int combustivel;
int probX, probF;
int velocidade = 50;

/* Forward declaration */
void atualizar_pos(Obj* obj_atual, Obj* obj_destino);

void limparTela() {
    system(CLEAR);
}

void ini_tile_vazio(Obj* tile) {
	/* inicializa tile com vazio 
	   vazio.vida e vazio.tipo sao, por padrao, 0 */
	tile->sprite = VAZIO_SPRITE;
	tile->vida = 0;
	tile->tipo = 0;
}

void ini_tabuleiro() {
	/* inicializa tabuleiro com tiles vazios, sendo tipo = 0, vida = 0 */
	int x, y;
	for(y = 0; y < TAMANHO_Y; y++) {
		for(x = 0; x < TAMANHO_X; x++) {
			ini_tile_vazio(&(tabuleiro[y][x]));
		}
	}
}

void atirar(int pos_x_jgdr, int pos_y_jgdr) {
	/* spawna tiro um tile na direita do jogador */
	if(pos_y_jgdr+1 <= TAMANHO_Y) {
		tabuleiro[pos_y_jgdr+1][pos_x_jgdr].sprite = '>'; /* Obj tiro TODO: MACRO */
		tabuleiro[pos_y_jgdr+1][pos_x_jgdr].vida = 50;
		tabuleiro[pos_y_jgdr+1][pos_x_jgdr].tipo = 1;
	}
}

void spawn(int pos_x, int pos_y, int tipo) {
	char sprite;
	int vida;
	if(tipo == 2) {
		sprite = 'F';
		vida = 50;
	}
	
	else if(tipo == -1) {
		sprite = 'X';
		int nivel_inimigo = rand() % 4;
		switch(nivel_inimigo) {
			case 0:
				vida = 50;
			case 1:
				vida = 50;
			case 2:
				tipo = -1;
				vida = 100;
			case 3:
				tipo = -2;
				vida = 150;
		}
	}
}

char* cor_obj(int tipo) {
	/* retorna cor do objeto do input */
	if(tipo == -1) {
		return VERMELHO;
	}
	else if(tipo == -2) {
		return AMARELO;
	}
	else if(tipo == -3) {
		return MAGENTA;
	}
	else if(tipo == 2) {
		return VERDE;
	}
	
	return BRANCO;
}

void print_tabul(char* limite) {
	/* printa limite superior */
	int i, j;
	limparTela();
	
	printf(COR_LIMITE"%s", limite);
	
	/* printa tabuleiro */
	printf("\n");
	printf(BRANCO);
	for(i = 0; i < TAMANHO_X; i++) {
		for(j = 0; j < TAMANHO_Y; j++) {
			/*printf("%s%c", cor_obj(tabuleiro[i][j].tipo), tabuleiro[j][i].sprite );*/
			printf("%c", tabuleiro[j][i].sprite);
		}
		printf("\n");
	}
	
	/* printa limite inferior */
	printf(COR_LIMITE"%s", limite);
}

void colisao(Obj* obj_atual, Obj* obj_destino) {
	if(obj_atual->tipo == 2 && obj_destino->tipo == 3) {
		ini_tile_vazio(obj_atual);
		combustivel+=COMBS_EXTRA;
	}
	else if(obj_atual->tipo == 3 && obj_destino->tipo == 2) {
		ini_tile_vazio(obj_destino);
		combustivel+=COMBS_EXTRA;
		/* obj atual toma lugar do obj destino */
		atualizar_pos(obj_atual, obj_destino);
	}
	else {
		obj_atual->vida -= 50;
		obj_destino->vida -= 50;
		if(obj_destino->vida <= 0) {
			atualizar_pos(obj_atual, obj_destino);
		}
	}
}

void atualizar_pos(Obj* obj_atual, Obj* obj_destino) {
	/* coloca objeto em nova posicao e troca posição atual por vazio 
		TODO: checkar nova posicao por colisao e caso necessario chamar funcao colisao */
	
	if(obj_destino->tipo != 0) {
		colisao(obj_atual, obj_destino);
	}
	else {
		obj_destino->sprite = obj_atual->sprite;
		obj_destino->vida = obj_atual->vida;
		obj_destino->tipo = obj_atual->tipo;
		ini_tile_vazio(obj_atual);
	}
}

void input_jogo(int input, int *pos_x_jgdr, int *pos_y_jgdr) {
	/* Recebe o input do jogador e faz a jogada de acordo 
		TODO: caso jogador nao de input ou aperte algum botao aleatorio */
		if(input == TIRO) {
			atirar(*pos_x_jgdr, *pos_y_jgdr);
		}
		
		/* movimentacao y-axis */	
		else if((input == CIMA || input == CIMA_UPPER)) {
			atualizar_pos(&tabuleiro[*pos_y_jgdr][*pos_x_jgdr], &tabuleiro[*pos_x_jgdr-1][*pos_y_jgdr]);
			*pos_x_jgdr-=1;
		}

		else if((input == BAIXO || input == BAIXO_UPPER)) {
			atualizar_pos(&tabuleiro[*pos_y_jgdr][*pos_x_jgdr], &tabuleiro[*pos_x_jgdr+1][*pos_y_jgdr]);
			*pos_x_jgdr+=1;
		}
			
		/* movimentacao x-axis */	
		else if((input == DIREITA || input == DIREITA_UPPER)) {
			atualizar_pos(&tabuleiro[*pos_y_jgdr][*pos_x_jgdr], &tabuleiro[*pos_x_jgdr][*pos_y_jgdr+1]);
			*pos_y_jgdr+=1;
		}
			
		else if((input == ESQUERDA || input == ESQUERDA_UPPER)) {
			atualizar_pos(&tabuleiro[*pos_y_jgdr][*pos_x_jgdr], &tabuleiro[*pos_x_jgdr][*pos_y_jgdr-1]);
			*pos_y_jgdr-=1;
		}
		else{
			printf("ODKSAJFIOASDFOLIUSADHF\n\nAODJKSAOIFJSAFD\n\nPAFKOADSF");
		}
			
}

void mover_tabul(int input, int pos_x_jgdr, int pos_y_jgdr){
	/* move todos objetos do tabuleiro na ordem tiros, jogador e inimigos */
	
	/* movimentacao jogador */
	if(input != -1) {
		input_jogo(input, &pos_x_jgdr, &pos_y_jgdr);
	}
}

int jogar() {
	/* inicia um novo jogo */
	int input, i;
	combustivel = 400;
	ini_tabuleiro();
	printf("AIOSJDOASJDOAJSD\n");
	
	/* inicializacao string de limite superior/inferior do tabuleiro */
	char limite[TAMANHO_Y+1];
	for(i = 0; i < TAMANHO_Y; i++) {
		limite[i] = '#';
	}
	limite[TAMANHO_Y] = '\0';
	
	/* Posicao inicial do jogador: [1][4] por padrao, vida inicial: 400 */
	tabuleiro[1][4].sprite = '+';
	tabuleiro[1][4].vida = 50;
	tabuleiro[1][4].tipo = 1;
	
	/* variaveis para identificar posicao e estado do jogador */
	int pos_jogador_x = 4;
	int pos_jogador_y = 1;
	Obj* jogador = &tabuleiro[1][4];

	
	print_tabul(limite);
	/*comecar();*/
	while(combustivel > 0) {
		usleep(100000);
		
		/* verifica caso botao seja pressionado e guarda a tecla */
		if(kbhit()) {
			input = getch();
		}
		else {
			input = -1;
		}
		mover_tabul(input, pos_jogador_x, pos_jogador_y);
		print_tabul(limite);
		
		/* atualiza ponteiro do jogador para nova posicao */
		jogador = &tabuleiro[pos_jogador_y][pos_jogador_x];
	}
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
	
	srand(time(0));
    
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
    

    return 0;
}
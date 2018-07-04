/* Universidade de Brasilia
* Instituto de Ciencias Exatas
* Departamento de Ciencia da Computacao
* 
* Algoritmos e Programacao de Computadores - 1/2018
* 
* Aluno (a): Alberto Tavares Duarte Neto
* Matricula: 180011707
* Turma: A
* Versao do compilador: 5.4.0
* 
* Descricao: Jogo baseado no River Raid, porem horizontal, com inimigos que matam por colisao e nao atiram e combustivel extra em formato de F.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

/* Tamanho tabuleiro (sem contar limite superior e inferior) */
#define TAMANHO_Y 135
#define TAMANHO_X 10-2

/* Cores */
#define VERMELHO "\033[22;31m"
#define VERDE "\033[22;32m"
#define AZUL "\033[22;34m"
#define AMARELO "\033[01;33m"
#define MAGENTA "\033[22;35m"
#define CYAN "\033[22;36m"
#define COR_LIMITE "\033[01;36m" /* Limites do mapa, cor: Light Cyan */
#define BRANCO "\033[01;37m" 

/* Tipos de objetos */
#define TIPO_INIMIGO_T -2
#define TIPO_INIMIGO_X -1
#define TIPO_VAZIO 0
#define TIPO_TIRO 1
#define TIPO_COMBS_XTRA 2
#define TIPO_BOMBA 3
#define TIPO_JOGADOR 4

/* Segundo valor retornado de Enter na tabela ASCII */
#define ENTER 10

/* Mensagem do menu */
#define MENU_MSG "1 - Jogar\n2 - Configuracoes\n3 - Ranking\n4 - Instrucoes\n5 - Sair\n\nEscolha uma opcao: "

/* Mensagem do menu jogar */
#define MENU_JOGAR_MSG "1 - Modo casual\n2 - Modo rankeado\n5 - Sair\n\nEscolha uma opcao:"

/* Mensagem das instrucoes */
#define INSTRUC_MSG VERMELHO"X - inimigo fraco, morre com 1 tiro.\n"AMARELO"X - inimigo medio, morre com 2 tiros.\n"MAGENTA"X - inimigo forte, morre com 3 tiros.\n"VERDE"F - combustivel, da +40 de combustivel para o jogador. Obs.: e destruido com 1 tiro.\n\n"BRANCO"Controles:\nTodo movimento gasta 2 de combustivel, sendo que ficar parado gasta 1.\nW anda para cima.\nS anda para baixo.\nA anda para esquerda.\nD anda para direita.\nK atira > gastando 3 de combustivel.\n\nO objetivo do jogo e ganhar o maximo de pontos possiveis sobrevivendo e matando inimigos.\nCaso encoste em um inimigo ou fique sem combustivel, o jogo acaba.\n\n\n5 - Sair\n\nEscolha uma opcao:"

/* Escolhas menu*/
/* Menu Principal */
#define MENU_JOGAR '1'
#define MENU_CONFIG '2'
#define MENU_RANKING '3'
#define MENU_INSTRUC '4'
#define MENU_SAIR '5'
/* Menu Jogar */
#define MODO_CASUAL '1'
#define MODO_RANKEADO '2'

/* Controles do jogo 
Upper sao as teclas maiusculas para evitar problemas caso capslock esteja ativado */
#define CIMA 'w'
#define CIMA_UPPER 'W'
#define BAIXO 's'
#define BAIXO_UPPER 'S'
#define DIREITA 'd'
#define DIREITA_UPPER 'D'
#define ESQUERDA 'a'
#define ESQUERDA_UPPER 'A'
#define TIRO 107
#define TIRO_UPPER 75

/* Implementacao kbhit, getch */
#ifndef _WIN32
    int kbhit() {
        /* Retorna 1 caso alguma tecla do teclado esteja sendo pressionada
        * Retorna 0 caso nao esteja */
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
        /* Retorna tecla do teclado pressionada */
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

/* Usado para limpar a tela em ambos windows e linux */
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

typedef struct {
    /* Objeto que faz parte do tabuleiro
    * Pode ser jogador, tiro, inimigo (fraco, medio ou forte), combustivel extra ou vazio */
    char sprite;
    int vida; /* Caso chegue a 0, objeto e destruido. -50 por colisao por padrao */
    int tipo; /* 1 - tiro, 2 - combustivel, 3 - jogador
                * 0 - vazio, -1 - inimigo X, -2 - inimigo T */
}Obj;

typedef struct {
    int pontuacao;
    int combustivel;
    int probX; /* Probabilidade feita com prob/1000 */
    int probF; /* Ex.: probX = 10; significa 1% de probabilidade */
    int probO;
    int probT;
    int velocidade;
}Jogoinf;

Jogoinf configs;
Obj tabuleiro[TAMANHO_Y][TAMANHO_X];

/* Forward declaration */
void atualizar_pos(int pos_x_atual, int pos_y_atual, int nova_pos_x, int nova_pos_y );


void limparTela() {
    system(CLEAR);
}

void ini_tile_vazio(int pos_x, int pos_y) {
    /* inicializa tile com vazio 
    * vazio.vida e vazio.tipo sao, por padrao, 0 */
    tabuleiro[pos_y][pos_x].sprite = ' ';
    tabuleiro[pos_y][pos_x].vida = 0;
    tabuleiro[pos_y][pos_x].tipo = TIPO_VAZIO;
}

void ini_tabuleiro() {
    /* Inicializa tabuleiro com vazios */
    int x, y;
    for(x = 0; x < TAMANHO_X; x++) {
        for(y = 0; y < TAMANHO_Y; y++) {
            ini_tile_vazio(x, y);
        }
    }
}

void print_cor(int tipo, int vida){
    /* Printa cor baseado no tipo e na vida */

    if(tipo == TIPO_COMBS_XTRA) {
        printf(VERDE);
    }
    
    else if(tipo == TIPO_INIMIGO_X) {
        
        if(vida == 50) {
            printf(VERMELHO); /* Inimigo fraco */
        }
        
        else if(vida == 100) {
            printf(AMARELO); /* Inimigo medio */
        }
        
        else if(vida == 150) {
            printf(MAGENTA); /* Inimigo forte */
        }
        
    }

    /* Printa BRANCO por padrao */
    else {
        printf(BRANCO);
    }
}

void print_tabul(char* limite) {
    /* Printa tabuleiro do jogo */
    int x, y;
    
    limparTela();
    
    /* Printa Informacoes para o jogador */
    printf(BRANCO"Combustivel: %3d          Pontos: %d\n", configs.combustivel, configs.pontuacao);
    
    /*Printa limite superior */
    printf(COR_LIMITE"%s", limite);
        
    /* Printa tabuleiro */
    printf("\n");
    for(x = 0; x < TAMANHO_X; x++) {
        for(y = 0; y < TAMANHO_Y; y++) {
            print_cor(tabuleiro[y][x].tipo, tabuleiro[y][x].vida);
            printf("%c", tabuleiro[y][x].sprite);
            }
        printf("\n");
    }
        
    /* Printa limite inferior */
    printf(COR_LIMITE"%s", limite);
}

void atirar(int pos_x_jgdr, int pos_y_jgdr) {
    /* Spawna tiro um tile na frente do jogador */
    if(pos_y_jgdr < TAMANHO_Y) {
        tabuleiro[pos_y_jgdr+1][pos_x_jgdr].sprite = '>';
        tabuleiro[pos_y_jgdr+1][pos_x_jgdr].vida = 5;
        tabuleiro[pos_y_jgdr+1][pos_x_jgdr].tipo = TIPO_TIRO;
    }
}

void wipe() {
    /*TODO: mudar nome. Limpa a tela de inimigos X */
    int i, j;
    for(i = 0; i < TAMANHO_X; i++) {
        for(j = 0; j < TAMANHO_Y; j++) {
            if(tabuleiro[j][i].tipo == TIPO_INIMIGO_X) {
                ini_tile_vazio(i, j);
            }
        }
    }
}

void destruir_obj(int pos_x, int pos_y, int colisao_jogador) {
    /* Destroi objeto e executa acoes adicionais */
    int pontuacao_xtra = 50;
    int qt_combs_xtra = 40;
    
    /* Adiciona pontuacao caso objeto morto seja inimigo */
    if(tabuleiro[pos_y][pos_x].tipo == -1) {
        configs.pontuacao+=pontuacao_xtra;
    }
    
    /* Jogador recebe combustivel caso colida com F */
    if(tabuleiro[pos_y][pos_x].tipo == TIPO_COMBS_XTRA && colisao_jogador) {
        configs.combustivel+=qt_combs_xtra;
    }
    
    if(tabuleiro[pos_y][pos_x].tipo == TIPO_BOMBA && colisao_jogador == 0) {
        wipe();
    }
    
    ini_tile_vazio(pos_x, pos_y);
}

void colisao(int pos_x_atual, int pos_y_atual, int nova_pos_x, int nova_pos_y ) {
    /* Detecta e resolve colisoes 
    * Adiciona pontuacao e combustivel extra */
    
    /* Padrao */
    int dano = 50;
    int colisao_jogador = 0;
    if(tabuleiro[pos_y_atual][pos_x_atual].tipo == TIPO_JOGADOR || tabuleiro[nova_pos_y][nova_pos_x].tipo == TIPO_JOGADOR) {
        colisao_jogador = 1;
    }
    
    /* Combustivel extra (F) ou bomba (O) entrando na posicao do jogador */
    if( (tabuleiro[pos_y_atual][pos_x_atual].tipo == TIPO_COMBS_XTRA || tabuleiro[pos_y_atual][pos_x_atual].tipo == TIPO_BOMBA) && colisao_jogador) {
        destruir_obj(pos_x_atual, pos_y_atual, 1);

    }
    
    /* Jogador entrando na posicao do combustivel extra (F) ou da bomba (O) */
    else if( (tabuleiro[nova_pos_y][nova_pos_x].tipo == TIPO_COMBS_XTRA || tabuleiro[nova_pos_y][nova_pos_x].tipo == TIPO_BOMBA) && colisao_jogador) {
        destruir_obj(nova_pos_x, nova_pos_y, 1);
        
        /* Objeto atual(jogador) toma lugar do objeto destino */
        atualizar_pos(pos_x_atual, pos_y_atual, nova_pos_x, nova_pos_y);
    }
    
    /* Caso padrao */
    else {

            tabuleiro[pos_y_atual][pos_x_atual].vida -= dano;
            tabuleiro[nova_pos_y][nova_pos_x].vida -= dano;
        
        /* Objeto se movimentando desaparece caso vida chegue em 0 */
        if(tabuleiro[pos_y_atual][pos_x_atual].vida <= 0) {
            destruir_obj(pos_x_atual, pos_y_atual, 0);
        }
        
        /* Caso objeto na pos destino morra, objeto atual entra em sua posicao */
        if(tabuleiro[nova_pos_y][nova_pos_x].vida <= 0) {
            destruir_obj(nova_pos_x, nova_pos_y, 0);
            atualizar_pos(pos_x_atual, pos_y_atual, nova_pos_x, nova_pos_y);
        }
    }
}

void atualizar_pos(int pos_x_atual, int pos_y_atual, int nova_pos_x, int nova_pos_y ) {
    /* Move objeto para nova posicao */
        
    /* Caso nova posicao seja vazia, apenas move objeto da posicao atual */
    if(tabuleiro[nova_pos_y][nova_pos_x].tipo == 0) {
        tabuleiro[nova_pos_y][nova_pos_x].sprite = tabuleiro[pos_y_atual][pos_x_atual].sprite;
        tabuleiro[nova_pos_y][nova_pos_x].vida = tabuleiro[pos_y_atual][pos_x_atual].vida;
        tabuleiro[nova_pos_y][nova_pos_x].tipo = tabuleiro[pos_y_atual][pos_x_atual].tipo;
        ini_tile_vazio(pos_x_atual, pos_y_atual);
    }
    /* Caso nova posicao nao seja vazia, chama funcao colisao */
    else {
        colisao(pos_x_atual, pos_y_atual, nova_pos_x, nova_pos_y);
    }
}

int input_jogo(int input, int *pos_x_jgdr, int *pos_y_jgdr) {
    /* Recebe o input do jogador e faz a jogada de acordo, retornando a quantidade de combustivel gastada
    * atualiza variavel que guarda a posicao do jogador caso haja movimento */
    int combs_movimento = 2;
    int combs_tiro = 3;

    /* Tiro */
    if(input == TIRO || input == TIRO_UPPER) {
        atirar(*pos_x_jgdr, *pos_y_jgdr);
        return combs_tiro;
    }
        
    /* Movimentacao vertical */	
    else if( (input == CIMA || input == CIMA_UPPER) && *pos_x_jgdr-1 >= 0 ) {
        atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr-1, *pos_y_jgdr);
        *pos_x_jgdr-=1;
        return combs_movimento;
    }
    
    else if( (input == BAIXO || input == BAIXO_UPPER) && *pos_x_jgdr+1 < TAMANHO_X ) {
        atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr+1, *pos_y_jgdr);
        *pos_x_jgdr+=1;
        return combs_movimento;
    }
                
                
    /* Movimentacao horizontal */	
    else if( (input == DIREITA || input == DIREITA_UPPER) && *pos_y_jgdr+1 < TAMANHO_Y ) {
        atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr, *pos_y_jgdr+1);
        *pos_y_jgdr+=1;
        return combs_movimento;
    }
                
    else if ((input == ESQUERDA || input == ESQUERDA_UPPER) && *pos_y_jgdr-1 >= 0 ) {
        atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr, *pos_y_jgdr-1);
        *pos_y_jgdr-=1;
        return combs_movimento;
    }
        
    /* Caso tecla nao esteja nos controles, jogador fica parado gastando 1 combustivel */
    return 1;
        
                        
}

int spawn(int x, int y) {
    /* Spawn aleatorio nas coordenadas dadas e retorna o tipo do obj spawnnado */
    char spawn_sprite;
    int spawn_tipo, spawn_vida;
    int odds = (rand() % 1000)+1;
    int spawn_X = configs.probX;
    int spawn_F = spawn_X + configs.probF;
    int spawn_O = spawn_F + configs.probO;
    
    /* 1% de chance de spawnar inimigo */
    if(odds >= 1 && odds <= spawn_X) {
        spawn_sprite = 'X';
        spawn_tipo = TIPO_INIMIGO_X;
        
        /* Caso seja inimigo, 70% de chance de ser fraco, 20% de ser medio e 10% de ser forte */
        if(odds == 1) {
            spawn_vida = 150;
        }
        
        else if(odds >= 2 && odds <= 3) {
            spawn_vida = 100;
        }
        
        else {
            spawn_vida = 50;
        }
    }
    
    /* 0.3% de chance de spawnar combustivel extra (F) */
    else if(odds >= 1+spawn_X && odds <= spawn_F) {
        spawn_sprite = 'F';
        spawn_tipo = TIPO_COMBS_XTRA;
        spawn_vida = 50;
    }
    
    else if(odds >= 1+spawn_F && odds <= spawn_O) {
        spawn_sprite = 'O';
        spawn_tipo = TIPO_BOMBA;
        spawn_vida = 150;
    }
    
    /* Caso nao spawne inimigo ou combustivel extra, retorna 0 */
    else {
        return 0;
    }
    
    tabuleiro[y][x].sprite = spawn_sprite;
    tabuleiro[y][x].tipo = spawn_tipo;
    tabuleiro[y][x].vida = spawn_vida;
    return spawn_tipo;
}

void mover_tabul(int *pos_x_jgdr, int *pos_y_jgdr, int input){
    /* Move todos objetos do tabuleiro na ordem tiros, jogador e inimigos */
    int x, y;
        
    /* Movimentacao tiros, iterado do fim da linha ate o inicio
    * Tiros se movem da esquerda para a direita */
    for(x = 0; x < TAMANHO_X; x++) {
        for(y = TAMANHO_Y-2; y >= 0; y--) {
            if(tabuleiro[y][x].tipo == TIPO_TIRO) {
                atualizar_pos(x, y, x, y+1);
            }
        }
        /* Despawna tiros na ultima coluna */
        if(tabuleiro[TAMANHO_Y-1][x].tipo == TIPO_TIRO) {
            ini_tile_vazio(x, TAMANHO_Y-1);
        }
    }
    
    /* Movimentacao jogador e diminuicao do combustivel baseado na acao do jogador */
    if(input != -1) {
        configs.combustivel -= input_jogo(input, pos_x_jgdr, pos_y_jgdr);
    }
    
    else {
        configs.combustivel -= 1;
    }
        
    /* Movimentacao inimigos e combustivel extra, iterado do inicio da linha ate o fim 
    * Se movem da direita para a esquerda */
    for(x = 0; x < TAMANHO_X; x++) {
        for(y = 1; y < TAMANHO_Y ; y++) {
            if(tabuleiro[y][x].tipo == TIPO_COMBS_XTRA || tabuleiro[y][x].tipo == TIPO_INIMIGO_X || tabuleiro[y][x].tipo == TIPO_BOMBA) {
                atualizar_pos(x, y, x, y-1);
            }
        }
        /* Despawna inimigos/combustivel extra na primeira coluna */
        if(tabuleiro[0][x].tipo == TIPO_COMBS_XTRA || tabuleiro[0][x].tipo == TIPO_INIMIGO_X || tabuleiro[0][x].tipo == TIPO_BOMBA) {
            ini_tile_vazio(x, 0);
        }
    }
    
    /* Spawn aleatorio na ultima coluna de cada linha */
    for(x = 0; x < TAMANHO_X; x++) {
        spawn(x, TAMANHO_Y-1);
    }
}

int jogar() {
    /* Inicia um novo jogo */
    int input, i;
    int combustivel_inicial = 400;
        
    /* Inicializacao configuracoes iniciais do jogo */
    configs.combustivel = combustivel_inicial;
    configs.pontuacao = 0;
    configs.probX = 10;
    configs.probF = 3;
    configs.probO = 1;
    configs.probT = 2;
    configs.velocidade = 50;
    char* game_over = "\nVoce foi atingido por um inimigo e perdeu!\nAperte alguma tecla para voltar ao menu.\n";
    ini_tabuleiro();  

    /* Inicializacao string de limite superior/inferior do tabuleiro */
    char limite[TAMANHO_Y+1];
    for(i = 0; i < TAMANHO_Y; i++) {
        limite[i] = '#';
    }
    limite[TAMANHO_Y] = '\0';
    
    /* Posicao inicial do jogador: [1][4] por padrao */
    tabuleiro[1][4].sprite = '+';
    tabuleiro[1][4].vida = 50;
    tabuleiro[1][4].tipo = TIPO_JOGADOR;
    
    /* Variaveis para identificar posicao e estado do jogador */
    int pos_x_jgdr = 4;
    int pos_y_jgdr = 1;
    Obj* jogador = &tabuleiro[1][4];
    
    print_tabul(limite);
    printf("\nPressione qualquer tecla para comecar!\n");
    getch();

    
    while(jogador->vida > 0) {
        
        print_tabul(limite);
        
        /* Verifica caso tecla seja pressionada e a guarda */
        if(kbhit()) {
            input = getch();
        }
        
        else {
            input = -1;
        }
        
        mover_tabul(&pos_x_jgdr, &pos_y_jgdr, input);
        
        /* Atualiza ponteiro da posicao do jogador */
        jogador = &tabuleiro[pos_y_jgdr][pos_x_jgdr];
        configs.pontuacao+=1;
        
        /* Verifica possivel falta de combustivel e prepara mensagem de Game Over de acordo */
        if(configs.combustivel<0) {
            game_over = "\nVoce ficou sem combustivel e perdeu!\nAperte alguma tecla para voltar ao menu.\n";
            jogador->vida = 0;
        }
        
        /* Para sleep em milisegundos multiplica-se por 1000 */
        usleep(configs.velocidade*1000);
        
    }
    /* Game Over */
    printf("%s", game_over);
    getch();
    return 1;
}

int menu_jogar() {
    int input, continuar = 1;

    while(continuar) {
        limparTela();
        printf(BRANCO"%s", MENU_JOGAR_MSG);
        input = getch();
        
        switch(input) {
            case MODO_CASUAL:
                return jogar();
            case MODO_RANKEADO:
                return jogar();
            case MENU_SAIR:
                continuar = 0;
        }
    }
    return 1;
}

int instruc() {
    /* Mostra instrucoes do jogo */    
    int input, continuar = 1;
    
    while(continuar) {
        limparTela();

        printf("%s", INSTRUC_MSG);
        input = getch();

        switch(input) {
                case MENU_SAIR:
                        continuar = 0;
        }
    }
    return 1;
        
}

int menu() {
    /* Mostra o menu principal do jogo */
    int input;
    limparTela();
    
    printf(BRANCO"%s", MENU_MSG);
    input = getch();
        
    switch(input) {
        case MENU_JOGAR:
            return menu_jogar();
                        
        case MENU_CONFIG:
            return 1;
                        
        case MENU_RANKING:
            return 1;
                        
        case MENU_INSTRUC:
            return instruc();
                        
        case MENU_SAIR:
            return 0;
                        
        default:
            return 1;
    }
}

int main() {
    int continuar = 1;
    int input = 0;
    
    srand(time(0));
    
    /* Mensagem de Boas vindas */
    while(input != ENTER) {
        limparTela();
        printf("Bem vindo ao jogo Boladeixon!\nAperte enter para entrar no menu");        
        input = getch();
    }
    
    /* Loop do menu principal, para quando menu() retornar 0 */
    continuar = 1;
    while(continuar) {
        continuar = menu();
    }
    
    getch();
    return 0;
}
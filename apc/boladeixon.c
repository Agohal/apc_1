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
#define TAMANHO_Y_MAX 185
#define TAMANHO_X_MAX 18

/* Cores */
#define VERMELHO "\033[22;31m"
#define VERDE "\033[22;32m"
#define AZUL "\033[22;34m"
#define AMARELO "\033[01;33m"
#define MAGENTA "\033[22;35m"
#define CYAN "\033[22;36m"
#define COR_LIMITE "\033[01;36m" /* Limites do mapa, cor: Light Cyan */
#define BRANCO "\033[01;37m"
#define NORMAL "\x1B[0m"

/* Tipos de objetos */
#define TIPO_COMBS_XTRA -5
#define TIPO_BOMBA -4
#define TIPO_TIRO_INIMIGO -3
#define TIPO_INIMIGO_T -2
#define TIPO_INIMIGO_X -1
#define TIPO_VAZIO 0
#define TIPO_TIRO 1
#define TIPO_JOGADOR 2

/* Segundo valor retornado de Enter na tabela ASCII */
#define ENTER 10

/* Mensagem do menu */
#define MENU_MSG "1 - Jogar\n2 - Configuracoes\n3 - Ranking\n4 - Instrucoes\n5 - Sair\n\nEscolha uma opcao: "

/* Mensagem do menu jogar */
#define MENU_JOGAR_MSG "1 - Modo casual\n2 - Modo rankeado\n5 - Sair\n\nEscolha uma opcao: "

/* Mensagem do menu de configuracoes */
#define MENU_CONFIG_MSG "1 - Tabuleiro\n2 - NPCs\n5 - Sair\n\nEscolha uma opcao: "

/* Mensagem do menu de configuracoes tabuleiro */
#define MENU_CONFIG_TAB_MSG "1 - Altura\n2 - Largura\n3 - Velocidade\n5 - Sair\n\nEscolha uma configuracao para alterar: "

/* Mensagem do menu de configuracoes npcs */
#define MENU_CONFIG_NPCS_MSG "1 - Probabilidade de spawn X\n2 - Probabilidade de spawn F\n3 - Probabilidade de spawn O\n4 - Probabilidade de spawn T\n6 - Vida Bomba\n7 - Municao T\n5 - Sair\n\nEscolha uma configuracao para alterar: "
/* Mensagem das instrucoes */
#define INSTRUC_MSG VERMELHO"X - inimigo fraco, morre com 1 tiro.\n"AMARELO"X - inimigo medio, morre com 2 tiros.\n"MAGENTA"X - inimigo forte, morre com 3 tiros.\n"VERDE"F - combustivel, da +40 de combustivel para o jogador. Obs.: e destruido com 1 tiro.\n"AZUL"O - bomba, necessita de multiplos tiros para morrer. Quando morta por tiros, destroi todos os "VERMELHO"X"AZUL" do mapa.\n\n"NORMAL"Controles:\nTodo movimento gasta 2 de combustivel, sendo que ficar parado gasta 1.\nW anda para cima.\nS anda para baixo.\nA anda para esquerda.\nD anda para direita.\nK atira > gastando 3 de combustivel.\n\nO objetivo do jogo e ganhar o maximo de pontos possiveis sobrevivendo e matando inimigos.\nCaso encoste em um inimigo ou fique sem combustivel, o jogo acaba.\n\n\n5 - Sair\n\nEscolha uma opcao:"

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

/* Menu Configs */
#define MENU_CONFIG_TAB '1'
#define MENU_CONFIG_NPCS '2'

/* Menu Configs Tabuleiro  */
#define MENU_CONFIG_TAB_ALTURA '1'
#define MENU_CONFIG_TAB_LARGURA '2'
#define MENU_CONFIG_TAB_VELOC '3'

/* Menu Configs Npcs */
#define MENU_CONFIG_NPCS_PROBX '1'
#define MENU_CONFIG_NPCS_PROBF '2'
#define MENU_CONFIG_NPCS_PROBO '3'
#define MENU_CONFIG_NPCS_PROBT '4'
#define MENU_CONFIG_NPCS_VIDAO '6'
#define MENU_CONFIG_NPCS_MUNT '7'


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

/* Nome do arquivo de configuracoes e de ranking */
#define CONFIGS_FILE "configs.txt"
#define RANKING_FILE "ranking.bin"
#define RANKING_TAMANHO 10

/* Configuracoes do modo rankeado */

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
    int tipo; /* 1 - tiro, 2 - jogador
                * 0 - vazio, -1 - inimigo X, -2 - inimigo T, -3 - tiro inimigo, -4 - combustivel */
    int tiros; /* Tiros apenas relevante para inimigo T, player tem tiros infinitos */
}Obj;

typedef struct {
    /* Configuracoes */
    int altura, largura; /* Probabilidade feita com prob/1000 */
    int probX, probF; /* Ex.: probX = 10; significa 1% de probabilidade */
    int probO, probT;
    int velocidade, vidaO, municaoT;
    /* Outros TODO:mudar nome */
    int pontuacao;
    int combustivel;
}Jogoinf;

typedef struct {
    /* Objeto para registro de pontuacao no ranking */
    char nome[11];
    int pontuacao;
} Player;

Jogoinf configs;
Obj tabuleiro[TAMANHO_Y_MAX+1][TAMANHO_X_MAX+1];

/* Forward declaration */
void atualizar_pos(int pos_x_atual, int pos_y_atual, int nova_pos_x, int nova_pos_y );


int verifTamanho(char* str) {
/* Verifica se tamanho da string excede 10 caracteres 
 * retorna 1 caso esteja do tamanho entre [1, 10], retorna 0 caso contrario */
    int i;

    for(i = 0; i < 11; i++) {
        if(str[i] == '\0')
            return 1;
    }
    
    return 0;
}

void printString(char* str) {
/* Printa string e completa com espaço, ate 10 caracteres */
    int i;
    
    for(i = 0; str[i] != '\0'; i++) {
        printf("%c", str[i]);
    }
    
    for(;i < 10; i++) {
        printf(" ");
    }
}

void copyString(char* orig, char* novo) {
    int i;
    
    for(i = 0; orig[i] != '\0'; i++) {
        novo[i] = orig[i];
    }
    novo[i] = '\0';
}

void limparTela() {
    
    system(CLEAR);
}

void ini_tile_vazio(int pos_x, int pos_y) {
    /* inicializa tile com vazio 
    * vazio.vida e vazio.tipo sao, por padrao, 0 */
    tabuleiro[pos_y][pos_x].sprite = ' ';
    tabuleiro[pos_y][pos_x].vida = 0;
    tabuleiro[pos_y][pos_x].tipo = TIPO_VAZIO;
    tabuleiro[pos_y][pos_x].tiros = 0;
}

void ini_tabuleiro() {
    /* Inicializa tabuleiro com vazios */
    int x, y;
    for(x = 0; x < configs.altura; x++) {
        for(y = 0; y < configs.largura; y++) {
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
    
    else if(tipo == TIPO_BOMBA) {
        
        if(vida == 50) {
            printf(VERMELHO); /* Bomba 1 vida */
        }
        
        else if(vida == 100) {
            printf(AMARELO); /* Bomba 2 vidas */
        }
        
        else if(vida == 150) {
            printf(MAGENTA); /* Bomba 3 vidas */
        }
        else if(vida == 200) {
            printf(AZUL); /* Bomba 4 vidas */
        }
        
    }

    /* Printa NORMAL por padrao */
    else {
        printf(NORMAL);
    }
}

void print_tabul(char* limite) {
    /* Printa tabuleiro do jogo */
    int x, y;
    
    limparTela();
    
    /* Printa Informacoes para o jogador */
    printf(NORMAL"Combustivel: %3d          Pontos: %d\n", configs.combustivel, configs.pontuacao);
    
    /*Printa limite superior */
    printf(COR_LIMITE"%s", limite);
        
    /* Printa tabuleiro */
    printf("\n");
    for(x = 0; x < configs.altura; x++) {
        for(y = 0; y < configs.largura; y++) {
            print_cor(tabuleiro[y][x].tipo, tabuleiro[y][x].vida);
            printf("%c", tabuleiro[y][x].sprite);
            }
        printf("\n");
    }
        
    /* Printa limite inferior */
    printf(COR_LIMITE"%s", limite);
}

int atirar(int pos_x, int pos_y, int tiro_inimigo) {
    /* Spawna tiro um tile na frente do jogador 
     * Retorna 1 caso consiga atirar */
    char sprite;
    int tipo, nova_pos_y, atirar;
    
    if(tiro_inimigo) {
        nova_pos_y = pos_y-2;
        atirar = 1;
        sprite = '<';
        tipo = TIPO_TIRO_INIMIGO;
    }
    
    else if(pos_y < configs.largura) {
        nova_pos_y = pos_y+1;
        atirar = 1;
        sprite = '>';
        tipo = TIPO_TIRO;
    }
    
    /* Caso atire, coloca tiro em posicao temporaria e chama atualizar_pos para resolver movimentacao */
    if(atirar) {
        tabuleiro[TAMANHO_Y_MAX][TAMANHO_X_MAX].sprite = sprite;
        tabuleiro[TAMANHO_Y_MAX][TAMANHO_X_MAX].vida = 50;
        tabuleiro[TAMANHO_Y_MAX][TAMANHO_X_MAX].tipo = tipo;
        tabuleiro[TAMANHO_Y_MAX][TAMANHO_X_MAX].tiros = 0;
        atualizar_pos(TAMANHO_X_MAX, TAMANHO_Y_MAX, pos_x, nova_pos_y);
        
        return 1;
    }
    
    return 0;
}

void wipe() {
    /*TODO: mudar nome. Limpa a tela de inimigos X */
    int i, j;
    for(i = 0; i < configs.altura; i++) {
        for(j = 0; j < configs.largura; j++) {
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
    if(tabuleiro[pos_y][pos_x].tipo == -1 && colisao_jogador == 0) {
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
    
    /* Caso tiro inimigo colida com objeto que nao seja jogador nem tiro, procurar proximo tile livre ou com jogador ou tiro */
    else if( (tabuleiro[pos_y_atual][pos_x_atual].tipo == TIPO_TIRO_INIMIGO) && tabuleiro[nova_pos_y][nova_pos_x].tipo < 0 ) {
        atualizar_pos(pos_x_atual, pos_y_atual, nova_pos_x, nova_pos_y-1);
    }
    
    /* Caso padrao */
    else {

        tabuleiro[pos_y_atual][pos_x_atual].vida -= dano;
        tabuleiro[nova_pos_y][nova_pos_x].vida -= dano;
        
        /* Objeto se movimentando desaparece caso vida chegue em 0 */
        if(tabuleiro[pos_y_atual][pos_x_atual].vida <= 0) {
            destruir_obj(pos_x_atual, pos_y_atual, colisao_jogador);
        }
        
        /* Caso objeto na pos destino morra, objeto atual entra em sua posicao */
        if(tabuleiro[nova_pos_y][nova_pos_x].vida <= 0) {
            destruir_obj(nova_pos_x, nova_pos_y, colisao_jogador);
            atualizar_pos(pos_x_atual, pos_y_atual, nova_pos_x, nova_pos_y);
        }
    }
}

void atualizar_pos(int pos_x_atual, int pos_y_atual, int nova_pos_x, int nova_pos_y ) {
    /* Move objeto para nova posicao */
        
    if(nova_pos_y < 0) {
        ini_tile_vazio(pos_x_atual, pos_y_atual);
    }
    
    else if(tabuleiro[nova_pos_y][nova_pos_x].tipo == 0) {
        tabuleiro[nova_pos_y][nova_pos_x].sprite = tabuleiro[pos_y_atual][pos_x_atual].sprite;
        tabuleiro[nova_pos_y][nova_pos_x].vida = tabuleiro[pos_y_atual][pos_x_atual].vida;
        tabuleiro[nova_pos_y][nova_pos_x].tipo = tabuleiro[pos_y_atual][pos_x_atual].tipo;
        tabuleiro[nova_pos_y][nova_pos_x].tiros = tabuleiro[pos_y_atual][pos_x_atual].tiros;
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
        atirar(*pos_x_jgdr, *pos_y_jgdr, 0);
        return combs_tiro;
    }
        
    /* Movimentacao vertical */	
    else if( (input == CIMA || input == CIMA_UPPER) && *pos_x_jgdr-1 >= 0 ) {
        atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr-1, *pos_y_jgdr);
        *pos_x_jgdr-=1;
        return combs_movimento;
    }
    
    else if( (input == BAIXO || input == BAIXO_UPPER) && *pos_x_jgdr+1 < configs.altura ) {
        atualizar_pos(*pos_x_jgdr, *pos_y_jgdr, *pos_x_jgdr+1, *pos_y_jgdr);
        *pos_x_jgdr+=1;
        return combs_movimento;
    }
                
                
    /* Movimentacao horizontal */	
    else if( (input == DIREITA || input == DIREITA_UPPER) && *pos_y_jgdr+1 < configs.largura ) {
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
    /* Spawn aleatorio nas coordenadas dadas e retorna o tipo do obj spawnnado 
     * TODO: Mudar modo de probabilidade de spawn */
    char spawn_sprite;
    int spawn_tipo, spawn_vida, spawn_tiros = 0;
    int odds = (rand() % 1000)+1; /* as chances sao feitas com base em mil. Portanto, todos os configs.prob deverao ser multiplicados por 10 */
    int spawn_X = (configs.probX*10) / configs.altura;
    int spawn_F = spawn_X + (configs.probF*10) / configs.altura;
    int spawn_O = spawn_F + (configs.probO*10) / configs.altura;
    int spawn_T = spawn_O + (configs.probT*10) / configs.altura;
    
    /* Spawn X */
    if(odds >= 1 && odds <= spawn_X) {
        spawn_sprite = 'X';
        spawn_tipo = TIPO_INIMIGO_X;
        
        /* Caso seja inimigo X, 50% de chance de ser fraco, 30% de ser medio e 20% de ser forte */
        if(odds >= 1 && odds <= spawn_X*0.60) {
            spawn_vida = 50;
        }
        
        else if(odds > spawn_X*0.60 && odds <= spawn_X*0.85) {
            spawn_vida = 100;
        }
        
        else {
            spawn_vida = 150;
        }
    }
    
    /* Spawn F */
    else if(odds >= 1+spawn_X && odds <= spawn_F) {
        spawn_sprite = 'F';
        spawn_tipo = TIPO_COMBS_XTRA;
        spawn_vida = 50;
    }
    
    /* Spawn O */
    else if(odds >= 1+spawn_F && odds <= spawn_O) {
        spawn_sprite = 'O';
        spawn_tipo = TIPO_BOMBA;
        spawn_vida = configs.vidaO*50;
    }
    
    /* Spawn T */
    else if(odds >= 1+spawn_O && odds <= spawn_T) {
        spawn_sprite = 'T';
        spawn_tipo = TIPO_INIMIGO_T;
        spawn_vida = 50;
        spawn_tiros = configs.municaoT;
    }
    
    /* Caso nao spawne inimigo ou combustivel extra, retorna 0 */
    else {
        return 0;
    }
    
    tabuleiro[y][x].sprite = spawn_sprite;
    tabuleiro[y][x].tipo = spawn_tipo;
    tabuleiro[y][x].vida = spawn_vida;
    tabuleiro[y][x].tiros = spawn_tiros;
    return spawn_tipo;
}

void mover_tabul(int *pos_x_jgdr, int *pos_y_jgdr, int input){
    /* Move todos objetos do tabuleiro na ordem tiros, jogador e inimigos */
    int x, y, odds;
        
    /* Movimentacao tiros, iterado do fim da linha ate o inicio
    * Tiros se movem da esquerda para a direita */
    for(x = 0; x < configs.altura; x++) {
        for(y = configs.largura-2; y >= 0; y--) {
            if(tabuleiro[y][x].tipo == TIPO_TIRO) {
                atualizar_pos(x, y, x, y+1);
            }
        }
        /* Despawna tiros na ultima coluna */
        if(tabuleiro[configs.largura-1][x].tipo == TIPO_TIRO) {
            ini_tile_vazio(x, configs.largura-1);
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
    for(x = 0; x < configs.altura; x++) {
        for(y = 1; y < configs.largura ; y++) {
            if(tabuleiro[y][x].tipo == TIPO_COMBS_XTRA || tabuleiro[y][x].tipo == TIPO_INIMIGO_X || tabuleiro[y][x].tipo == TIPO_BOMBA) {
                atualizar_pos(x, y, x, y-1);
            }
            else if(tabuleiro[y][x].tipo == TIPO_INIMIGO_T) {
                /* 10% de chance de atirar, caso tenha tiros > 0 */
                odds = rand() % 10;
                if(odds == 0 && tabuleiro[y][x].tiros > 0) {
                    tabuleiro[y][x].tiros -= atirar(x, y, 1);
                }
                atualizar_pos(x, y, x, y-1);

            }
            else if(tabuleiro[y][x].tipo == TIPO_TIRO_INIMIGO) {
                atualizar_pos(x, y, x, y-2);
            }
            
        }
        /* Despawna inimigos/combustivel extra na primeira coluna */
        if(tabuleiro[0][x].tipo == TIPO_COMBS_XTRA || tabuleiro[0][x].tipo == TIPO_INIMIGO_X || tabuleiro[0][x].tipo == TIPO_BOMBA || tabuleiro[0][x].tipo == TIPO_INIMIGO_T || tabuleiro[0][x].tipo == TIPO_TIRO_INIMIGO) {
            ini_tile_vazio(x, 0);
        }
    }
    
    /* Spawn aleatorio na ultima coluna de cada linha */
    for(x = 0; x < configs.altura; x++) {
        spawn(x, configs.largura-1);
    }
}

void criarConfigsTXT() {
/* Cria arquivo de configuracoes, com configs padroes */
    FILE* fd;
    fd = fopen(CONFIGS_FILE, "w");
    fprintf(fd, "%d %d %d %d %d %d %d %d %d", 10, 135, 10, 3, 1, 2, 50000, 3, 3);
    /* altura, largura, probX, probF, probO, probT, velocidade, vidaO, municaoT */
    fclose(fd);
}

void carregarConfigs(int modo) {
/* Carrega configuracoes 
 * caso modo == casual, carrega configs de arquivo */
    if(modo == MODO_CASUAL) {
        FILE* fd;
        fd = fopen(CONFIGS_FILE, "r");
    
        /* Caso arquivo de configuracoes nao seja encontrado, criar um novo com configuracoes padroes */
        if(fd == NULL) {
            criarConfigsTXT();
            fd = fopen(CONFIGS_FILE, "r");
        }
        
        /* Carrega configs */
        fscanf(fd, "%d %d", &configs.altura, &configs.largura);
        configs.altura -= 2; /* Tira 2 linhas referentes aos limites superior e inferior */
        fscanf(fd, "%d %d %d %d", &configs.probX, &configs.probF, &configs.probO, &configs.probT);
        fscanf(fd, "%d %d %d", &configs.velocidade, &configs.vidaO, &configs.municaoT);
        
        fclose(fd);
    }
    
    else if(modo == MODO_RANKEADO) {
        /* Configs modo rankeado */
        configs.altura = 10-2;
        configs.largura = 135;
        configs.probX = 25;
        configs.probF = 10;
        configs.probO = 1;
        configs.probT = 4;
        configs.vidaO = 10;
        configs.municaoT = 5;
        configs.velocidade = 60000;
    }
}

void ordRanking(Player* ranking, int tamanho) {
    /* Ordena ranking com a maior pontuacao em primeiro */
    int i, j;
    char tmpnome[11];
    int tmppont;
    /* Bubblesort, orderna do maior pro menor */
    for(i = 0; i < tamanho; i++) {
        for(j = 0; j < tamanho-1-i; j++) {
            if(ranking[j].pontuacao < ranking[j+1].pontuacao) {
                copyString(ranking[j+1].nome, tmpnome);
                tmppont = ranking[j+1].pontuacao;
                
                copyString(ranking[j].nome, ranking[j+1].nome);
                ranking[j+1].pontuacao = ranking[j].pontuacao;
                
                copyString(tmpnome, ranking[j].nome);
                ranking[j].pontuacao = tmppont;
            }
        }
    }
}

void registrarPont(char* nome, int pontuacao) {
    /* Registra nome e pontuacao do jogador em arquivo binario */
    int i;
    FILE* fd;
    fd = fopen(RANKING_FILE, "r+b");
    
    if(fd == NULL) {
        fd = fopen(RANKING_FILE, "wb");
        Player tmp;
        copyString("---------", tmp.nome);
        tmp.pontuacao = 0;
        
        for(i = 0; i < RANKING_TAMANHO; i++) {
            fwrite(&tmp, sizeof(Player), 1, fd);
        }
        fclose(fd);
        fd = fopen(RANKING_FILE, "r+b");
    }
    
    Player ranking[RANKING_TAMANHO+1];
    fread(ranking, sizeof(Player), RANKING_TAMANHO, fd);

    
    /* Coloca novo rank no ranking, contendo, assim, RANKING_TAMANHO +1 */
    copyString(nome, ranking[RANKING_TAMANHO].nome);
    ranking[RANKING_TAMANHO].pontuacao = pontuacao;
    
    /* Ordena ranking, colocando o menor na ultima posicao */
    ordRanking(ranking, RANKING_TAMANHO+1);
    
    /* Por gravar apenas (RANKING_TAMANHO) pontuacoes, a pontuacao na ultima posicao, que e a menor, e perdida, mantendo, assim, o ranking em ordem */
    rewind(fd);
    fwrite(ranking, sizeof(Player), RANKING_TAMANHO, fd);
    fclose(fd);
}

int jogar(int modo) {
    /* Inicia um novo jogo
     * Quando modo == 1, ativa modo casual (com configs personalizadas)
     * Quando modo == 2, ativa modo rankeado com configs especiais */
    int input, i, terminar = 0;
    int combustivel_inicial = 400;
    char nome[10000];
        
    /* Inicializacao configuracoes iniciais do jogo */
    configs.combustivel = combustivel_inicial;
    configs.pontuacao = 0;
    carregarConfigs(modo);
    char* game_over = "\nVoce foi atingido por um inimigo e perdeu!\nAperte alguma tecla para voltar ao menu.\n";
    ini_tabuleiro();  

    /* Inicializacao string de limite superior/inferior do tabuleiro */
    char limite[configs.largura+1];
    for(i = 0; i < configs.largura; i++) {
        limite[i] = '#';
    }
    limite[configs.largura] = '\0';
    
    /* Variaveis para identificar posicao e estado do jogador */
    int pos_x_jgdr = 1;
    int pos_y_jgdr = 0;
    Obj* jogador = &tabuleiro[pos_y_jgdr][pos_x_jgdr];
    
    /* Posicao inicial do jogador: [1][4] por padrao */
    jogador->sprite = '+';
    jogador->vida = 50;
    jogador->tipo = TIPO_JOGADOR;

    /* Nome jogador caso seja rankeado */
    if(modo == MODO_RANKEADO) {
        while(terminar == 0) {
            limparTela();
            printf("Digite seu nome, de 1 a 10 caracteres: ");
            scanf("%s", nome);
            terminar = verifTamanho(nome);
        }
        
    }
    
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
        usleep(configs.velocidade);
        
    }
    /* Game Over */
    printf("%s", game_over);
    if(modo == MODO_RANKEADO) {
        registrarPont(nome, configs.pontuacao-1);
    }
    getch();
    return 1;
}

int menu_jogar() {
    /* Menu de escolha entre casual e rankeada */
    int input, continuar = 1;

    while(continuar) {
        limparTela();
        printf(NORMAL"%s", MENU_JOGAR_MSG);
        input = getch();
        
        switch(input) {
            case MODO_CASUAL:
                return jogar(MODO_CASUAL);
            case MODO_RANKEADO:
                return jogar(MODO_RANKEADO);
            case MENU_SAIR:
                continuar = 0;
        }
    }
    return 1;
}

int alterarValor(int lim_inf, int lim_sup) {
    /* Pede valor pro usuario para mudancas de configuracoes */
    int continuar = 1, tmp;
    
    while(continuar) {
        limparTela();
        printf("Digite um valor entre [%d, %d]:\n", lim_inf, lim_sup);
        scanf("%d", &tmp);
        getch();
        if(tmp >= lim_inf && tmp <= lim_sup) {
            continuar = 0;
        }
    }
    
    return tmp;
}

int menu_configs_npcs() {
    /* Menu de alteracao de configuracoes do tabuleiro */
    int input, continuar = 1, altura, largura, probX, probF, probO, probT, velocidade, vidaO, municaoT;
    int lim_inf_prob = 1, lim_sup_prob = 25;
    int lim_inf_vidaO = 1 , lim_sup_vidaO = 15;
    int lim_inf_munT = 1 , lim_sup_munT = 20;
    FILE* fd;
    fd = fopen(CONFIGS_FILE, "r");
    
    /* Verifica a existencia do arquivo de configuracoes e, caso nao exista, cria-o */
    if(fd == NULL) {
        criarConfigsTXT();
        fd = fopen(CONFIGS_FILE, "r");
        /* Verifica se arquivo foi criado com sucesso */
        if(fd == NULL) {
            return 2;
        }   
    }
    
    /* Le arquivo de configuracoes */
    fscanf(fd, "%d %d", &altura, &largura);
    fscanf(fd, "%d %d %d %d", &probX, &probF, &probO, &probT);
    fscanf(fd, "%d %d %d", &velocidade, &vidaO, &municaoT);
    fclose(fd);

    /* Pede novos valores de configuracoes pro usuario */
    while(continuar) {
        limparTela();
        printf(NORMAL"%s", MENU_CONFIG_NPCS_MSG);
        input = getch();
        
        switch(input) {
            /* Probabilidades */
            case MENU_CONFIG_NPCS_PROBX:
                probX = alterarValor(lim_inf_prob, lim_sup_prob);
                break;
            case MENU_CONFIG_NPCS_PROBF:
                probF = alterarValor(lim_inf_prob, lim_sup_prob);
                break;
            case MENU_CONFIG_NPCS_PROBO:
                probO = alterarValor(lim_inf_prob, lim_sup_prob);
                break;
            case MENU_CONFIG_NPCS_PROBT:
                probT = alterarValor(lim_inf_prob, lim_sup_prob);
                break;
             
            /* Outros */
            case MENU_CONFIG_NPCS_VIDAO:
                vidaO = alterarValor(lim_inf_vidaO, lim_sup_vidaO);
                break;
            case MENU_CONFIG_NPCS_MUNT:
                municaoT = alterarValor(lim_inf_munT, lim_sup_munT);
                break;
                
            case MENU_SAIR:
                continuar = 0;
        }
    }
    
    /* Escreve novas configuracoes em arquivo */
    fd = fopen(CONFIGS_FILE, "w");
    fprintf(fd, "%d %d %d %d %d %d %d %d %d",altura, largura, probX, probF, probO, probT, velocidade, vidaO, municaoT);
    fclose(fd);
    
    return 1;
}

int menu_configs_tab() {
    /* Menu de alteracao de configuracoes do tabuleiro */
    int input, continuar = 1, altura, largura, probX, probF, probO, probT, velocidade, vidaO, municaoT;
    int lim_inf_alt = 5, lim_sup_alt = 18;
    int lim_inf_lar = 85 , lim_sup_lar = 185;
    int lim_inf_veloc = 40000 , lim_sup_veloc = 80000;
    FILE* fd;
    fd = fopen(CONFIGS_FILE, "r");
    
    /* Verifica a existencia do arquivo de configuracoes e, caso nao exista, cria-o */
    if(fd == NULL) {
        criarConfigsTXT();
        fd = fopen(CONFIGS_FILE, "r");
        /* Verifica se arquivo foi criado com sucesso */
        if(fd == NULL) {
            return 2;
        }   
    }
    
    /* Le arquivo de configuracoes */
        fscanf(fd, "%d %d", &altura, &largura);
        fscanf(fd, "%d %d %d %d", &probX, &probF, &probO, &probT);
        fscanf(fd, "%d %d %d", &velocidade, &vidaO, &municaoT);

    /* Pede novos valores de configuracoes pro usuario */
    while(continuar) {
        limparTela();
        printf(NORMAL"%s", MENU_CONFIG_TAB_MSG);
        input = getch();
        
        switch(input) {
            /* Probabilidades */
            case MENU_CONFIG_TAB_ALTURA:
                altura = alterarValor(lim_inf_alt, lim_sup_alt);
                break;
            case MENU_CONFIG_TAB_LARGURA:
                largura = alterarValor(lim_inf_lar, lim_sup_lar);
                break;
            case MENU_CONFIG_TAB_VELOC:
                velocidade = alterarValor(lim_inf_veloc, lim_sup_veloc);
                break;
            case MENU_SAIR:
                continuar = 0;
        }
    }
    /* Escreve novas configuracoes em arquivo */
    fd = fopen(CONFIGS_FILE, "w");
    fprintf(fd, "%d %d %d %d %d %d %d %d %d",altura, largura, probX, probF, probO, probT, velocidade, vidaO, municaoT);
    fclose(fd);
    
    return 1;
}

int menu_configs() {
    /* Menu de configuracoes, que leva ao configs tab e configs npcs */
    int input, continuar = 1;

    while(continuar) {
        limparTela();
        printf(NORMAL"%s", MENU_CONFIG_MSG);
        input = getch();
        
        switch(input) {
            case MENU_CONFIG_TAB:
                menu_configs_tab();
                break;
            case MENU_CONFIG_NPCS:
                menu_configs_npcs();
                break;
            case MENU_SAIR:
                continuar = 0;
        }
    }
    return 1;
}

int menu_ranking() {
/* Mostra ranking de pontuacao de partidas rankeadas */
    int i;
    FILE* fd;
    Player tmp;
    fd = fopen(RANKING_FILE, "rb");
    
    limparTela();
    printf("Ranking!!!\n\nJogador:   Pontuacao:\n");
    
    if(fd == NULL) {
        printf("Nao há nenhuma pontuacao registrada!\n");
    }
    
    else {
        
       for(i = 0; i < RANKING_TAMANHO; i++) {
            if(fread(&tmp, sizeof(Player), 1, fd) != 0 ) {
                printString(tmp.nome);
                printf(" %d\n", tmp.pontuacao);
            }
            
       }
    }
    printf("\nAperte alguma tecla para voltar\n");
    getch();
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
    
    printf(NORMAL"%s", MENU_MSG);
    input = getch();
        
    switch(input) {
        case MENU_JOGAR:
            return menu_jogar();
                        
        case MENU_CONFIG:
            return menu_configs();
                        
        case MENU_RANKING:
            return menu_ranking();
                        
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
        if(continuar == 2) {
            printf("Erro na criacao do arquivo de configuracoes!!!\n");
            getch();
            continuar = 1;
        }
    }
    
    getch();
    return 0;
}
Programa compilado e testado em:
    1. gcc (Ubuntu 16.04.9) 5.4.0
    Funciona, porem há ocasionalmente demora nos prints, o que quebra a ilusão de movimento, principalmente quando há input muito rapido.
    2. gcc (Windows 10) 7.3.0
    Funciona, mas quase injogável devido à problemas no print.

Flags utilizada no GCC:
    -lm -ansi -O3

Extras:
    1. Nos Menus, no lugar do scanf para a escolha, é usado getch, tornando a utilização dos menus mais dinâmica.
    2. Há cores para diferentes objetos do tabuleiro.
    3. Inimigos possuem vida maior, levando 1, 2 ou ate 3 tiros para morrerem, sendo a vida identificada por cores (vermelho, amarelo e magenta, respectivamente).

A seguir são algumas mudanças especificamente na implementação do jogo.

    4. O tabuleiro foi implementado como uma matriz de um struct chamado Obj, que possui sprite, tipo e vida. Apesar da variável tipo parecer inútil porque a identificação dos objetos poderia ser feita utilizando sprite, ela existe para que possa haver diferentes objetos com mesmo sprite em uma futura atualização.
    5. Para facilitar a implementação, algumas variáveis globais foram trocadas por um struct que as contêm (note que a variável do struct continua sendo global).

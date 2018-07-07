/*
	Universidade de Brasilia
	Instituto de Ciencias Exatas
	Departamento de Ciencia da Computacao
	Algoritmos e Programação de Computadores – 1/2018
	Aluno(a): Joao Victor de Souza Calassio
	Matricula: 180033808
	Turma: A
	Versão do compilador: gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.9) padrao ANSI
	Descricao: Jogo de nave, cujo objetivo é atirar nos inimigos e conseguir a maior quantidade de pontos possivel
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#ifdef _WIN32
	#include <conio.h>
#else
	#include <termios.h>
	#include <fcntl.h>
	int kbhit(){
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

		if(ch != EOF){
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
#endif
#ifdef _WIN32
	#define CLEAR "cls"
	#define B_ENTER 13
#else
	#define CLEAR "clear"
	#define B_ENTER 10
#endif
#define ROWS 10
#define COLUMNS 135
#define ROWS_M 21
#define COLUMNS_M 60
/* expressão que gera um número aleatório no intervalo [1, 100]*/
#define RAND() (rand()%100 + 1)

int usleep();
/* posicao da nave */
int refY = 5;
int refX = 1;
/*  VARIAVEIS DO JOGO
	loopa: estado do jogo (rodando ou nao)
	atirando: se há projetil em movimento
	gas: combustivel
	vidas: vidas do player
	velocidade: tempo de atualizacao do sleep, em microssegundos
	probX: probabilidade de surgir um obstaculo
	probY: probabilidade de surgir um combustivel
	piscou: verifica se a nave piscou (tecla invalida)
*/
int loopa = 0;
int atirando = 1;
int piscou = 0;
int vidas = 1;
int ranked;
int altura;
int largura;
int gas;
int pontos;
int velocidade;
int probX;
int probF;
int probO;
int probT;
int vidaO;
int municaoT;

/* structs dos inimigos T e O, e do ranking */
typedef struct {
	int x;
	int y;
	int tiros;
} tipo_T; 

typedef struct {
	int x;
	int y;
	int hp;
} tipo_O;

typedef struct {
	char nick[11];
	int score;
} player;


tipo_O inimigo_O[150]; /* array dos inimigos O */
int o_atual = 0;
tipo_T inimigo_T[150]; /* array dos inimigos T */
int t_atual = 0;

/* gera o campo */
void gerar_campo(char campo[ROWS][COLUMNS]){
	int i, j;
	for(i=0;i<altura;i++){
		for(j=0;j<largura;j++){
			if(i==0 || i==altura-1){
				campo[i][j] = '#';
			} else if (i==(altura/2) && j==1){
				campo[i][j] = '+';
				refX = j;
				refY = i;
			} else {
				campo[i][j] = ' ';
			}
		}
	}
}
/* move os projeteis e faz colisao deles com os obstaculos/combustiveis X pt1
  funciona em conjunto com a ger_ev (essa com os tiros se movendo) */
void mov_proj(char vet[ROWS][COLUMNS]){
	int i, j, stop_proj=1;
	/* stop_proj : caso nao haja nenhum tiro se movendo, seta atirando=0
		para nao executar essa funcao e economizar processamento */
	for(i=1;i<altura-1;i++){
		for(j=largura-2;j>1;j--){
			if(vet[i][j]=='X'||vet[i][j]=='F'||vet[i][j]=='O'||vet[i][j]=='T'){
				if(vet[i][j-1]=='>'){ /* se um tiro encontrar um X, add pontos */
					if(vet[i][j]=='X'){
						pontos+=50;
						vet[i][j-1]=' ';
						vet[i][j]=' ';
					} else { /* senao, apenas some os dois */
						vet[i][j-1]=' ';
						vet[i][j]=' ';
					}
				}
			} else {
				if(vet[i][j]=='>'){ /* faz o tiro andar. nao vai colidir por causa do if anterior, nao pode ser O */
					if(vet[i][j+1]!='O'){
						vet[i][j+1]='>';
						vet[i][j]=' ';
					}
					if(j==largura-2){ /* faz o tiro sumir no final do grid */
						vet[i][j+1]=' ';
					}
					stop_proj=0;
				}
			}
		}
	}
	if(stop_proj==1){
		atirando = 0;
	}
}
/* gera os inimigos X e combustiveis, move, faz a colisao deles com a nave, e com os projeteis pt2
	funciona em conjunto com a mov_proj */
void ger_ev(char vet[ROWS][COLUMNS]){
	int i, j;
	for(i=1;i<altura-1;i++){
		for(j=0;j<largura;j++){
			if(vet[i][j]=='F' || vet[i][j]=='X' || vet[i][j]=='O' || vet[i][j]=='T'){
				if(vet[i][j-1]=='>'){ /* se um dos personagens encontrar um tiro */
					if(vet[i][j]=='X'){ /* se for X, add pontos */
						pontos+=50;
						vet[i][j-1]=' ';
						vet[i][j]=' ';
					} else {
						vet[i][j-1]=' ';
						vet[i][j]=' ';
					}
				} else if(j!=0 && vet[i][j-1]!='+' && vet[i][j-1]!='>'){ /* move os personagens */
					vet[i][j-1] = vet[i][j];
					vet[i][j] = ' ';
				} else if(vet[i][j-1]=='+' && (vet[i][j]=='X' || vet[i][j]=='T')){ /* mata o jogador caso ele bata */
					vidas--;
					vet[i][j] = ' ';
				} else if(vet[i][j-1]=='+' && vet[i][j]=='F'){ /* add fuel */
					gas+=40;
					vet[i][j] = ' ';
				} else { /* some o personagem no final do grid */
					vet[i][j] = ' ';
				}
			}
		}
	}
	/* gera os inimigos/fuel */
	int rG = RAND();
	int trG; /* random generator temporario, apenas para escolher a posicao */  
	if(rG>=0 && rG <=probF){ /* gera F */
		trG = 1+rand()%9;
		if((vet[trG][largura-2]!='F' && vet[trG][largura-2]!='X' && vet[trG][largura-2]!='O' && vet[trG][largura-2]!='T') && trG<altura-1){
			if((vet[trG][largura-3]!='F' && vet[trG][largura-3]!='X' && vet[trG][largura-3]!='O' && vet[trG][largura-3]!='T')){
				vet[trG][largura-2] = 'F';
			}
		}
	} else if (rG>probF && rG<=(probF+probX)){ /* gera X */
		trG = 1+rand()%9;
		if((vet[trG][largura-2]!='F' && vet[trG][largura-2]!='X' && vet[trG][largura-2]!='O' && vet[trG][largura-2]!='T') && trG<altura-1){
			if((vet[trG][largura-3]!='F' && vet[trG][largura-3]!='X' && vet[trG][largura-3]!='O' && vet[trG][largura-3]!='T')){
				vet[trG][largura-2] = 'X';
			}
		}
	} else if (rG>(probF+probX) && rG<=(probF+probX+probO)){ /* gera O */
		trG = 1+rand()%9;
		if((vet[trG][largura-2]!='F' && vet[trG][largura-2]!='X' && vet[trG][largura-2]!='O' && vet[trG][largura-2]!='T') && trG<altura-1){
			if((vet[trG][largura-3]!='F' && vet[trG][largura-3]!='X' && vet[trG][largura-3]!='O' && vet[trG][largura-3]!='T')){
				if(o_atual<150){
					/* seta os dados do novo O */
					vet[trG][largura-2] = 'O';
					inimigo_O[o_atual].x = largura-3;
					inimigo_O[o_atual].y = trG;
					inimigo_O[o_atual].hp = vidaO;
					o_atual++;
				} else {
					o_atual = 0;
				}
			}
		}
	} else if (rG>(probF+probX+probO) && rG<=(probF+probX+probO+probT)){ /* gera T */
		trG = 1+rand()%9;
		if((vet[trG][largura-2]!='F' && vet[trG][largura-2]!='X' && vet[trG][largura-2]!='O' && vet[trG][largura-2]!='T') && trG<altura-1){
			if((vet[trG][largura-3]!='F' && vet[trG][largura-3]!='X' && vet[trG][largura-3]!='O' && vet[trG][largura-3]!='T')){
				if(t_atual<150){
					/* seta os dados do novo T */
					vet[trG][largura-2] = 'T';
					inimigo_T[t_atual].x = largura-3;
					inimigo_T[t_atual].y = trG;
					inimigo_T[t_atual].tiros = municaoT;
					t_atual++;
				} else {
					t_atual = 0;
				}
			}
		}
	}
}

/* pisca nave (caso o jogador aperte uma tecla invalida) */
void pisca_nave(char vet[ROWS][COLUMNS]){ 
	if(piscou==0){
		piscou = 1;
	} else if(piscou==2){
		piscou = 0;
	}
}

/* caso o jogador aperte uma tecla invalida */
void penalidade(char vet[ROWS][COLUMNS]){
	int c = RAND();
	if(c<50 && refY>1){
		vet[refY-1][refX] = vet[refY][refX];
		vet[refY][refX] = ' ';
		refY--;
		gas-=2;
		pisca_nave(vet);
	} else if(c>=50 && refY<altura-2){
		vet[refY+1][refX] = vet[refY][refX];
		vet[refY][refX] = ' ';
		refY++;
		gas-=2;
		pisca_nave(vet);
	}
}
/* move o aviao e controla o tiro dos projeteis
	*/
void m_aviao(char vet[ROWS][COLUMNS]){
	int c = kbhit();
	if(c==1){
		int b = getch();
		if((b==119 || b==87) && (refY>1)){ /* w - mov pra cima */
			/* caso a nave colida com um objeto */
			if(vet[refY-1][refX]=='X'){
				vidas--;
			} else if(vet[refY-1][refX]=='F'){
				gas+=40;
			}
			/* realiza o movimento da nave */
			vet[refY-1][refX] = vet[refY][refX];
			vet[refY][refX] = ' ';
			
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}
			refY--;
			gas-=2;
		} else if ((b==115 || b==83) && (refY<altura-2)){ /* s - mov pra baixo */
			/* caso a nave colida com um objeto */
			if(vet[refY+1][refX]=='X'){
				vidas--;
			} else if(vet[refY+1][refX]=='F'){
				gas+=40;
			}
			/* realiza o movimento da nave */
			vet[refY+1][refX] = vet[refY][refX];
			vet[refY][refX] = ' ';
			
			refY++;
			gas-=2;
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}
		} else if((b==32)) {/* espaco - tiro */
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}		
			vet[refY][refX+1] = '>';
			atirando = 1;
			gas-=3;
		} else if(b!=32 && b!=115 && b!=83 && b!=119 && b!=87){ /* penalidade - botao invalido */
			penalidade(vet);
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}
		}
	} else {
		if(atirando==1){ /* continua movimento dos tiros */
			mov_proj(vet);
		}
	}
}
/* mostra o jogo */
void show(char vet[ROWS][COLUMNS]){
	int i, j;
	for(i=0;i<altura;i++){
		for(j=0;j<largura;j++){
			if(vet[i][j]=='X'){ /* coloracao do X */
				printf("\x1b[31m" "%c" "\x1b[0m", vet[i][j]);
			} else if(vet[i][j]=='F'){ /* coloracao do F */
				printf("\x1b[32m" "%c" "\x1b[0m", vet[i][j]);
			} else if(vet[i][j]=='O'){ 
				printf("\x1b[34m" "%c" "\x1b[0m", vet[i][j]);
			} else if(vet[i][j]=='T'||vet[i][j]=='<'){
				printf("\e[0;93m" "%c" "\e[0m", vet[i][j]);
			} else if(vet[i][j]=='+' && piscou!=0 && piscou<2){ /* mostrar nave piscando */
				printf("\e[0;31m" "%c" "\e[0;0m", vet[i][j]);
				piscou++;
				pisca_nave(vet);
			} else {
				printf("%c",vet[i][j]);
			}
		}
		printf("\n");
	}
	printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("  COMBUSTIVEL: %d | PONTOS: %d \n", gas, pontos);
	printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
}

/* tela de instrucoes */
void infos(){
	system(CLEAR);
	printf("\n\n\t\e[1;33mInstrucoes:\e[0m\n\n");
	printf("\t W : move a nave para cima\n");
	printf("\t S : move a nave para baixo\n");
	printf("\t Espaco : atira\n");
	printf("\t Atire nos obstaculos \x1b[31mX\x1b[0m, para ganhar pontos e sobreviver.\n");
	printf("\t A nave gasta combustivel, mas ha recargas \x1b[32mF\x1b[0m durante o percurso.\n");
	printf("\t Tome cuidado com os inimigos \e[0;93mT\e[0m, eles atiram em voce, mas tem tiros limitados.\n");
	printf("\t Os inimigos \x1b[34mO\x1b[0m precisam de mais tiros para morrer, mas explodem todos os X quando morrem\n");
	printf("\t Se voce ficar sem combustivel ou bater, game over.\n");
	printf("\t Seus tiros gastam combustivel, entao fique atento!\n");
	printf("\t Apertar um botao invalido acarretara em um movimento aleatorio.\n\n");
	printf("\t Pressione qualquer botao para voltar ao menu.\n");
	int x = 1;
	while(x==1){
		int c = getch();
		if(c){
			x = 0;
		}
	}
}
/* game over */
void game_over(char motivo[23]){
	system(CLEAR);
	printf("\n\n\tGAME OVER\n\t%s\n\tPONTOS: %d\n\t",motivo,pontos);
	printf("PRESSIONE <ENTER> PARA VOLTAR AO MENU\n");
	int x = 1;
	while(x==1){
		int c = getch();
		if(c==B_ENTER){
			x = 0;
		}
	}
}

/* especial do O, explode todos os X */
void spec_O(char vet[ROWS][COLUMNS]){
	int i, j;
	for(i=0;i<altura;i++){
		for(j=0;j<largura;j++){
			if(vet[i][j]=='X'){
				vet[i][j]=' ';
				pontos+=10;
			}
		}
	}
}

/* atualiza as posicoes dos O, faz a colisao deles com os tiros */
void att_posicoes_o(char vet[ROWS][COLUMNS]){
	int i, j, k;
	for(i=1;i<altura;i++){
		for(j=0;j<largura-2;j++){
			if(vet[i][j]=='O'){
				for(k=0;k<o_atual;k++){
					if((inimigo_O[k].y == i) && (inimigo_O[k].x == j)){
						/*printf(":: cod: %d x:%d y:%d hp:%d ::\n",k,inimigo_O[k].x,inimigo_O[k].y,inimigo_O[k].hp);*/
						if(inimigo_O[k].hp>0){
							if(vet[i][j-1]=='>'){
								vet[i][j-1]=' ';
								inimigo_O[k].hp--;
							}
							if(vet[i][j-2]=='>'){
								vet[i][j-2]=' ';
								inimigo_O[k].hp--;
							}
							inimigo_O[k].x--;
						} else {
							vet[i][j]=' ';
							inimigo_O[k].x = 0;
							inimigo_O[k].y = 0;
							spec_O(vet);
						}
					}
				}
			}
		}
	}
}

/* move os tiros do T */
void mov_tiros_t(char vet[ROWS][COLUMNS]){
	int i, j;
	for(i=1;i<altura;i++){
		for(j=0;j<largura-2;j++){
			if(vet[i][j]=='<' && j>=3){
				if(vet[i][j-1]=='+'||vet[i][j-2]=='+'){
					vidas--;
					vet[i][j-2]='<';
				}
				if(vet[i][j-2]=='>' || vet[i][j-1]=='>'){
					vet[i][j]=' ';
					if(vet[i][j-2]=='>'){
						vet[i][j-2]=' ';
					} else {
						vet[i][j-1]=' ';
					}
				} else {
					if(vet[i][j-2]==' '){
						vet[i][j-2]='<';
						vet[i][j]=' ';
					} else {
						if(vet[i][j-2]=='X'||vet[i][j-2]=='O'||vet[i][j-2]=='F'||vet[i][j-2]=='T'){
							vet[i][j-4]='<';
							vet[i][j]=' ';
						} else if(vet[i][j-1]=='X'||vet[i][j-1]=='O'||vet[i][j-1]=='F'||vet[i][j-1]=='T'){
							vet[i][j-3]='<';
							vet[i][j]=' ';
						}
					}
				}
			} else if(vet[i][j]=='<' && j<=2){
				if(vet[i][j-1]=='+'||vet[i][j-2]=='+'){
					vidas--;
					vet[i][j-2]='<';
				}
				vet[i][j]=' ';
			}
		}
	}
}

/* atualiza as posicoes do T e gera o tiro */
void att_posicoes_t(char vet[ROWS][COLUMNS]){
	int i, j, k, r;
	for(i=1;i<altura;i++){
		for(j=0;j<largura-2;j++){
			if(vet[i][j]=='T'){
				for(k=0;k<t_atual;k++){
					if((inimigo_T[k].y == i) && (inimigo_T[k].x == j)){
						if(inimigo_T[k].tiros>0 && vet[i][j-2]==' '){
							r = RAND();
							if(r<15 && j>2){
								vet[i][j-2]='<';
								inimigo_T[k].tiros--;
							}
						}
						inimigo_T[k].x--;
					}
				}
			}
		}
	}
}

/* calc tamanho da string */
int size(char a[]){
	int i=0;
	while(a[i]!='\0'){
		i++;
	}
	return i;
}

/* comparador para o qsort */
int compare (const void * a, const void * b){
	player *playerA = (player *)a;
	player *playerB = (player *)b;
	return (playerB->score - playerA->score);
}

/* grava o ranking no arquivo */
/* obs sobre o ranking: 
	de 0 a 9 = 10 primeiros do ranking
	10 = ultimo colocado 
		 (jogador da partida atual, nao aparece no ranking)
	
*/
void grava_ranking(player jogador){
	FILE* fd;
	player old_players[11];
	int i,j;

	fd = fopen("ranking.bin","rb");
	if(fd==NULL){ /* se o ranking ainda nao existir, cria um com o novo player em 1o e todos os outros 0 */
		fd = fopen("ranking.bin","wb");

		old_players[0] = jogador;
		fwrite(&old_players[0],sizeof(player),1,fd);
		for(i=1;i<10;i++){
			for(j=0;j<10;j++)
				old_players[i].nick[j] = ' ';
			old_players[i].nick[10] = '\0';
			old_players[i].score = 0;
			fwrite(&old_players[i],sizeof(player),1,fd);
		}
	} else { /* senao, apenas adiciona o jogador na posicao 10, e ordena o ranking */
		for(i=0;i<10;i++){
			fread(&old_players[i],sizeof(player),1,fd);
		}
		old_players[10] = jogador;
		qsort(old_players,11,sizeof(player),compare);
		fclose(fd);
		fd = fopen("ranking.bin","wb");
		for(i=0;i<10;i++){
			fwrite(&old_players[i],sizeof(player),1,fd);
		}
	}
	fclose(fd);
}

/* tela do usuario digitar o nickname para ranked */
void read_nickname(char nickname[]){
	int i;
	system(CLEAR);
	printf("\n\n");
	printf("\t\e[1;33mModo Rankeado\e[0m\n");
	printf("\tDigite seu nickname (min 1, max 10 caracteres):\n\t");
	scanf("%s",nickname);
	i = size(nickname);
	while(i<1 || i>10){ /* se invalido */
		printf("\tNickname invalido. Por favor, digite novamente:\n\t");
		scanf("%s",nickname);
		i = size(nickname);
	}
}

/* execucao do jogo */
void jogar(){
	int taltura,tlargura,tprobX,tprobF,tprobO,tprobT,tvidaO,tmunicaoT,tvelocidade;
	player jogador;
	/* le o arquivo de cfg */
	FILE* fd;
	fd = fopen("config.txt","r");
	fscanf(fd, "%d %d %d %d %d %d %d %d %d %d",&altura,&largura,&probX,&probF,&probO,&probT,&vidaO,&municaoT,&velocidade,&ranked);
	fclose(fd);

	/* muda as variaveis de jogo */
	atirando = 0;
	pontos = 0;
	vidas = 1;
	gas = 400;
	vidas = 1;
	pontos = 0;

	/* se for ranked, volta para esses valores e salva os do arquivo nos temporarios*/
	if(ranked==1){
		read_nickname(jogador.nick);
		setbuf(stdin,NULL); /* limpa o buffer para nao bugar o caractere do nome no input do jogo*/
		taltura = altura;
		tlargura = largura;
		tprobX = probX;
		tprobF = probF;
		tprobO = probO;
		tprobT = probT;
		tvidaO = vidaO;
		tmunicaoT = municaoT;
		tvelocidade = velocidade;

		altura = 10;
		largura = 135,
		probX = 25;
		probF = 10;
		probO = 1;
		probT = 4;
		vidaO = 10;
		municaoT = 5;
		velocidade = 60000;
	}
	char campo[ROWS][COLUMNS];
	system(CLEAR);
	gerar_campo(campo);
	loopa = 1;

	/* main loop */
	while(loopa==1){
		usleep(velocidade);
		system(CLEAR);
		/* chama as funcoes do jogo */
		att_posicoes_o(campo);
		mov_tiros_t(campo);
		m_aviao(campo);
		ger_ev(campo);
		att_posicoes_t(campo);
		show(campo);
		gas--;
		pontos++;
		if(vidas<=0){
			loopa = 0;
			game_over("VOCE MORREU");
		}
		if(gas<=0){
			loopa = 0;
			game_over("SEU COMBUSTIVEL ACABOU");
		}
	}
	/* se for ranked, devolve os valores personalizados para as variaveis e salva o ranking */
	if(ranked==1){
		jogador.score = pontos;

		altura = taltura;
		largura = tlargura;
		probX = tprobX;
		probF = tprobF;
		probO = tprobO;
		probT = tprobT;
		vidaO = tvidaO;
		municaoT = tmunicaoT;
		velocidade = tvelocidade;

		grava_ranking(jogador);
	}
}

/* salva as cfgs */
void savesettings(){
	FILE* fd;
	fd = fopen("config.txt","w");
	fprintf(fd, "%d %d %d %d %d %d %d %d %d %d\n",altura,largura,probX,probF,probO,probT,vidaO,municaoT,velocidade,ranked);
	fprintf(fd, "#altura - largura - probX - probF - probO - probT - vidaO - municaoT - velocidade - ranked");
	fclose(fd);
}

/* menu de configuracoes do tabuleiro */
void cfg_tabuleiro(){
	int x = 1;
	char seletor[] = {'>',' ',' ',' '};
	int c;

	system(CLEAR);
	printf("\n\n");
	printf("\t\e[1;33mConfiguracoes do tabuleiro\e[0m\n\n");
	printf("\t%c Altura: %d\n",seletor[0],altura);
	printf("\t%c Largura: %d\n",seletor[1],largura);
	printf("\t%c Velocidade: %d us\n",seletor[2],velocidade);
	printf("\t%c Voltar\n",seletor[3]);
	printf("\n\tAperte enter para selecionar.\n");

	while(x==1){
		c = getch();
		if(c==115 || c==83){
			if(seletor[0]=='>'){
				seletor[0] = ' ';
				seletor[1] = '>';
			} else if(seletor[1]=='>'){
				seletor[1] = ' ';
				seletor[2] = '>';
			} else if(seletor[2]=='>'){
				seletor[2] = ' ';
				seletor[3] = '>';
			} else if(seletor[3]=='>'){
				seletor[3] = ' ';
				seletor[0] = '>';
			}
			system(CLEAR);
			printf("\n\n");
			printf("\t\e[1;33mConfiguracoes do tabuleiro\e[0m\n\n");
			printf("\t%c Altura: %d\n",seletor[0],altura);
			printf("\t%c Largura: %d\n",seletor[1],largura);
			printf("\t%c Velocidade: %d us\n",seletor[2],velocidade);
			printf("\t%c Voltar\n",seletor[3]);
			printf("\n\tAperte enter para selecionar.\n");

		} else if(c==B_ENTER){
			if(seletor[3]=='>'){
				x = 0;
			} else {
				printf("\n\tNovo valor: ");
				if(seletor[0]=='>'){
					printf("(min: 3; max: 10)\n\t");
					scanf("%d",&altura);
					while(altura<3 || altura>10){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&altura);
					}
				}
				if(seletor[1]=='>'){
					printf("(min: 3; max: 135\n\t");
					scanf("%d",&largura);
					while(largura<3 || largura>135){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&largura);
					}
				}
				if(seletor[2]=='>'){
					printf("(min: 1; max: 1000000000)\n\t");
					scanf("%d",&velocidade);
					while(velocidade<1 || velocidade>1000000000){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&velocidade);
					}
				}
				setbuf(stdin,NULL);
				system(CLEAR);
				printf("\n\n");
				printf("\t\e[1;33mConfiguracoes do tabuleiro\e[0m\n\n");
				printf("\t%c Altura: %d\n",seletor[0],altura);
				printf("\t%c Largura: %d\n",seletor[1],largura);
				printf("\t%c Velocidade: %d us\n",seletor[2],velocidade);
				printf("\t%c Voltar\n",seletor[3]);
				printf("\n\tAperte enter para selecionar.\n");
				savesettings();
			}
		}
	}

}

/* menu de configuracoes dos npcs */
void cfg_npcs(){
	int x = 1;
	char seletor[] = {'>',' ',' ',' ',' ',' ',' '};
	int c;

	system(CLEAR);
	printf("\n\n");
	printf("\t\e[1;33mConfiguracoes dos inimigos/combustivel\e[0m\n\n");
	printf("\t%c Probabilidade do X: %d%%\n",seletor[0],probX);
	printf("\t%c Probabilidade do combustivel (F): %d%%\n",seletor[1],probF);
	printf("\t%c Probabilidade do O: %d%%\n",seletor[2],probO);
	printf("\t%c Probabilidade do T: %d%%\n",seletor[3],probT);
	printf("\t%c Vidas do O: %d\n",seletor[4],vidaO);
	printf("\t%c Municao do T: %d\n",seletor[5],municaoT);
	printf("\t%c Voltar\n",seletor[6]);
	printf("\n\tAperte enter para selecionar.\n");
	while(x==1){
		c = getch();
		if(c==115 || c==83){
			if(seletor[0]=='>'){
				seletor[0]=' ';
				seletor[1]='>';
			} else if(seletor[1]=='>'){
				seletor[1]=' ';
				seletor[2]='>';
			} else if(seletor[2]=='>'){
				seletor[2]=' ';
				seletor[3]='>';
			} else if(seletor[3]=='>'){
				seletor[3]=' ';
				seletor[4]='>';
			} else if(seletor[4]=='>'){
				seletor[4]=' ';
				seletor[5]='>';
			} else if(seletor[5]=='>'){
				seletor[5]=' ';
				seletor[6]='>';
			} else if(seletor[6]=='>'){
				seletor[6]=' ';
				seletor[0]='>';
			}

			system(CLEAR);
			printf("\n\n");
			printf("\t\e[1;33mConfiguracoes dos inimigos/combustivel\e[0m\n\n");
			printf("\t%c Probabilidade do X: %d%%\n",seletor[0],probX);
			printf("\t%c Probabilidade do combustivel (F): %d%%\n",seletor[1],probF);
			printf("\t%c Probabilidade do O: %d%%\n",seletor[2],probO);
			printf("\t%c Probabilidade do T: %d%%\n",seletor[3],probT);
			printf("\t%c Vidas do O: %d\n",seletor[4],vidaO);
			printf("\t%c Municao do T: %d\n",seletor[5],municaoT);
			printf("\t%c Voltar\n",seletor[6]);
			printf("\n\tAperte enter para selecionar.\n");	
		} else if(c==B_ENTER){
			if(seletor[6]=='>'){
				x = 0;
			} else {
				printf("\tNovo valor: ");
				if(seletor[0]=='>'){
					printf("(min: 0; max: 100\n\t");
					scanf("%d",&probX);
					while(probX<0 || probX>100){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&probX);
					}
				} else if(seletor[1]=='>'){
					printf("(min: 0; max: 100\n\t");
					scanf("%d",&probF);
					while(probF<0 || probF>100){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&probF);
					}
				} else if(seletor[2]=='>'){
					printf("(min: 0; max: 100\n\t");
					scanf("%d",&probO);
					while(probO<0 || probO>100){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&probO);
					}
				} else if(seletor[3]=='>'){
					printf("(min: 0; max: 100\n\t");
					scanf("%d",&probT);
					while(probT<0 || probT>100){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&probT);
					}
				} else if(seletor[4]=='>'){
					printf("(min: 0; max: 30\n\t");
					scanf("%d",&vidaO);
					while(vidaO<0 || vidaO>100){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&vidaO);
					}
				} else if(seletor[5]=='>'){
					printf("(min: 0; max: 100\n\t");
					scanf("%d",&municaoT);
					while(municaoT<0 || municaoT>100){
						printf("\tValor invalido! Insira novamente:\n\t");
						scanf("%d",&municaoT);
					}
				}
				setbuf(stdin,NULL);
				system(CLEAR);
				printf("\n\n");
				printf("\t\e[1;33mConfiguracoes dos inimigos/combustivel\e[0m\n\n");
				printf("\t%c Probabilidade do X: %d%%\n",seletor[0],probX);
				printf("\t%c Probabilidade do combustivel (F): %d%%\n",seletor[1],probF);
				printf("\t%c Probabilidade do O: %d%%\n",seletor[2],probO);
				printf("\t%c Probabilidade do T: %d%%\n",seletor[3],probT);
				printf("\t%c Vidas do O: %d\n",seletor[4],vidaO);
				printf("\t%c Municao do T: %d\n",seletor[5],municaoT);
				printf("\t%c Voltar\n",seletor[6]);
				printf("\n\tAperte enter para selecionar.\n");
				savesettings();
			}
		}
	}
}

/* loop do menu de configuracoes */
void config(){
	int x = 1;
	char seletor[] = {'>',' ',' ',' '};
	int c;

	system(CLEAR);
	printf("\n\n");
	printf("\t\e[1;33mConfiguracoes\e[0m\n\n");
	printf("\t%c Tabuleiro\n",seletor[0]);
	printf("\t%c NPCs\n",seletor[1]);
	if(ranked==0)
		printf("\t%c Ativar modo rankeado\n",seletor[2]);
	else if(ranked==1)
		printf("\t%c Desativar modo rankeado\n",seletor[2]);
	printf("\t%c Voltar\n",seletor[3]);

	while(x==1){
		c = getch();

		if(c==115 || c==83){
			if(seletor[0]=='>'){
				seletor[0]=' ';
				seletor[1]='>';
			} else if(seletor[1]=='>'){
				seletor[1]=' ';
				seletor[2]='>';
			} else if(seletor[2]=='>'){
				seletor[2]=' ';
				seletor[3]='>';
			} else if(seletor[3]=='>'){
				seletor[3]=' ';
				seletor[0]='>';
			}
			system(CLEAR);
			printf("\n\n");
			printf("\t\e[1;33mConfiguracoes\e[0m\n\n");
			printf("\t%c Tabuleiro\n",seletor[0]);
			printf("\t%c NPCs\n",seletor[1]);
			if(ranked==0)
				printf("\t%c Ativar modo rankeado\n",seletor[2]);
			else if(ranked==1)
				printf("\t%c Desativar modo rankeado\n",seletor[2]);
			printf("\t%c Voltar\n",seletor[3]);
		} else if(c==B_ENTER){
			if(seletor[3]=='>'){
				x = 0;
			} else {
				if(seletor[0]=='>'){
					cfg_tabuleiro();
				} else if(seletor[1]=='>'){
					cfg_npcs();
				} else if(seletor[2]=='>'){
					if(ranked==0)
						ranked = 1;
					else if(ranked==1)
						ranked = 0;
				}
				system(CLEAR);
				printf("\n\n");
				printf("\t\e[1;33mConfiguracoes\e[0m\n\n");
				printf("\t%c Tabuleiro\n",seletor[0]);
				printf("\t%c NPCs\n",seletor[1]);
				if(ranked==0)
					printf("\t%c Ativar modo rankeado\n",seletor[2]);
				else if(ranked==1)
					printf("\t%c Desativar modo rankeado\n",seletor[2]);
				printf("\t%c Voltar\n",seletor[3]);
				savesettings();
			}
		}
	}
}

/* tela que mostra o ranking */
void rank_scr(){
	int x = 1;
	int c, i;

	FILE* fd;
	fd = fopen("ranking.bin","rb");
	player jogador[11];

	system(CLEAR);
	printf("\n\n");
	printf("\t\e[1;33mRanking\e[0m\n\n");
	if(fd==NULL){
		printf("\t Sem registros.\n");
	} else {
		printf("\t\e[1;37mScore\tNome\e[0m\n");
		for(i=0;i<10;i++){
			fread(&jogador[i], sizeof(player), 1, fd);
			printf(" \t%d\t%s\n",jogador[i].score,jogador[i].nick);
		}
	}
	printf("\n\t > Voltar\n");

	while(x==1){
		c = getch();
		if(c==B_ENTER){
			x = 0;
		}
	}
}

/* atualiza o menu principal */
void att(char vet[ROWS_M][COLUMNS_M], int tipo){
	int i,j;
	if(tipo==2){
		vet[13][27] = ' ';
		vet[14][24] = '>';
	} else if(tipo==3){
		vet[14][24] = ' ';
		vet[15][26] = '>';
	} else if(tipo==4){
		vet[15][26] = ' ';
		vet[16][23] = '>';
	} else if(tipo==5){
		vet[16][23] = ' ';
		vet[17][27] = '>';
	} else if(tipo==1){
		vet[17][27] = ' ';
		vet[13][27] = '>';
	}
	system(CLEAR);
	for(i=0;i<ROWS_M;i++){
		for(j=0;j<COLUMNS_M;j++){
			printf("%c",vet[i][j]);
		}
		if(i!=ROWS_M-1){
			printf("\n");
		}
	}
	printf("- S : muda de alternativa | Enter : seleciona              -\n");
	printf("------------------------------------------------------------\n");
}
/* menu principal */
void menu_f(){
	system(CLEAR);
	char menu[ROWS_M][COLUMNS_M] = {{'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','_','_',' ','_',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','_',' ',' ',' ',' ','_','_',' ','_',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','/',' ','_','(','_',')',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','(',' ',')',' ',' ','/',' ','_','|',' ','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|',' ','|','_',' ','_',' ','_',' ','_','_',' ','_','_','_',' ',' ',' ','_',' ','_','_',' ','|','/',' ',' ','|',' ','|','_','|',' ','|','_',' ',' ',' ','_',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|',' ',' ','_','|',' ','|',' ','\'','_','_','/',' ','_',' ','\\',' ','|',' ','\'','_',' ','\\',' ',' ',' ',' ','|',' ',' ','_','|',' ','|',' ','|',' ','|',' ','|',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ',' ','_','_','/',' ','|',' ','|',' ','|',' ','|',' ',' ',' ','|',' ','|',' ','|',' ','|',' ','|','_','|',' ','|',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','_','|',' ','|','_','|','_','|',' ',' ','\\','_','_','_','|',' ','|','_','|',' ','|','_','|',' ',' ',' ','|','_','|',' ','|','_','|','\\','_','_',',',' ','|',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','_','/',' ','|',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','_','_','/',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','>','J','o','g','a','r',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','I','n','s','t','r','u','c','o','e','s',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','R','a','n','k','i','n','g',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','C','o','n','f','i','g','u','r','a','c','o','e','s',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','S','a','i','r',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',},
                                   {'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',}};
    int x = 1;
    /* sel : verifica qual opcao esta selecionada;
    	1 = jogar
    	2 = instrucoes
    	3 = ranking
    	4 = config
    	5 = sair
    	*/
    int sel = 1;

    /* faz a leitura das configuracoes para as variaveis */
    FILE* fd;
	fd = fopen("config.txt","r+");
	if(fd==NULL){ /* se o arquivo nao existir, cria um */
		fd = fopen("config.txt","w");
		/* altura, largura, probx, probf, probO, probT, vidaO, municaoT, velocidade */
		altura = 10;
		largura = 135,
		probX = 25;
		probF = 10;
		probO = 1;
		probT = 4;
		vidaO = 10;
		municaoT = 5;
		velocidade = 60000;
		ranked = 0;
		fprintf(fd, "%d %d %d %d %d %d %d %d %d %d\n",altura,largura,probX,probF,probO,probT,vidaO,municaoT,velocidade,ranked);
		fprintf(fd, "#altura - largura - probX - probF - probO - probT - vidaO - municaoT - velocidade - ranked");

	} else { 
		fscanf(fd, "%d %d %d %d %d %d %d %d %d %d",&altura,&largura,&probX,&probF,&probO,&probT,&vidaO,&municaoT,&velocidade,&ranked);
	}
	fclose(fd);

	/* loop do menu */
    while(x==1){
    	att(menu,sel);
    	int c = getch();
    	if(c==B_ENTER){
    		switch(sel){
    			case 1 : jogar(); break;
    			case 2 : infos(); break;
    			case 3 : rank_scr(); break;
    			case 4 : config(); break;
    			case 5 : x=0; break;
    		}
    	} else {
    		if(c==115 || c==83){
    			if(sel<5){
    				sel++;
    			} else {
    				sel = 1;
    			}
    			att(menu,sel);
    		}
    	}
    }
}
int main(){
	srand(time(0)); /* muda a seed de geracao das posicoes aleatorias*/
	menu_f();
	return 0;
}
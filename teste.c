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
	#include <windows.h>
	#include <conio.h>
	/*void usleep(__int64 usec) 
	{ 
   		HANDLE timer; 
    	LARGE_INTEGER ft; 

    	ft.QuadPart = -(10*usec); 

    	timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    	WaitForSingleObject(timer, INFINITE); 
    	CloseHandle(timer); 
	} */
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
	int usleep();
#endif
#ifdef _WIN32
	#define CLEAR "cls"
	#define B_ENTER 13
#else
	#define CLEAR "clear"
	#define B_ENTER 10
#endif
#define ROWS 21
#define COLUMNS 60
/* expressão que gera um número aleatório no intervalo [1, 100]*/
#define RAND() (rand()%100 + 1)
/* posição do bloco superior esquerdo do aviao
   tamanho do aviao : 3x2 (y,x) */
int refY = 9;
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
int atirando = 0;
int gas = 400;
int vidas = 1;
int pontos = 0;
int velocidade = 50000;
int probX;
int probF;
int piscou = 0;
/* move a parte de cima do campo 
	da primeira linha ate a linha 4 da matriz
*/
void m_cima(char vet[ROWS][COLUMNS]){
	int i, j;
	char guarda_primeiro;
	for(i=0;i<4;i++){
		for(j=0;j<COLUMNS;j++){
			if(j==0){
				guarda_primeiro = vet[i][j];
				vet[i][j] = vet[i][j+1];
			} else {
				if(j==COLUMNS-1){
					vet[i][j] = guarda_primeiro;
				} else {
					vet[i][j] = vet[i][j+1];
				}
			}
		}
	}
}
/* move a parte de baixo do campo 
	da linha 18 até o final da matriz
*/
void m_baixo(char vet[ROWS][COLUMNS]){
	int i, j;
	char guarda_primeiro;
	for(i=17;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			if(j==0){
				guarda_primeiro = vet[i][j];
				vet[i][j] = vet[i][j+1];
			} else {
				if(j==COLUMNS-1){
					vet[i][j] = guarda_primeiro;
				} else {
					vet[i][j] = vet[i][j+1];
				}
			}
		}
	}
}
/* move os projeteis e faz colisao deles com os obstaculos/combustiveis pt1*/
void mov_proj(char vet[ROWS][COLUMNS]){
	int i, j, stop_proj=1;
	/* stop_proj : caso nao haja nenhum tiro se movendo, seta atirando=0
		para nao executar essa funcao e economizar processamento */
	for(i=5;i<16;i++){
		for(j=COLUMNS-2;j>2;j--){
			if(vet[i][j]=='X'||vet[i][j]=='+'){
				if(vet[i][j-1]=='>'){
					if(vet[i][j]=='X'){
						pontos+=50;
					}
					vet[i][j-1]=' ';
					vet[i][j]=' ';
				}
			} else {
				if(vet[i][j]=='>'){
					vet[i][j+1]='>';
					vet[i][j]=' ';
					if(j==COLUMNS-2){
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
/* gera os inimigos e combustiveis, move, faz a colisao deles com a nave, e com os projeteis pt2*/
void ger_ev(char vet[ROWS][COLUMNS]){
	int i, j;
	for(i=5;i<16;i++){
		for(j=0;j<COLUMNS;j++){
			if(vet[i][j]=='+' || vet[i][j]=='X'){
				if(vet[i][j-1]=='>'){
					if(vet[i][j]=='X'){
						pontos+=50;
					}
					vet[i][j-1]=' ';
					vet[i][j]=' ';
				} else if(j!=0 && vet[i][j-1]!='D' && vet[i][j-1]!='>'){
					vet[i][j-1] = vet[i][j];
				} else if(vet[i][j-1]=='D' && vet[i][j]=='X'){
					vidas--;
				} else if(vet[i][j-1]=='D' && vet[i][j]=='+'){
					gas+=40;
				}
				vet[i][j] = ' ';
			}
		}
	}
	int rG = RAND();
	if(rG>=75 && rG <=100){
		probF = 1+rand()%15;
		if((vet[probF+4][59]!='+' || vet[probF+4][59]!='X') && probF+4<16){
			vet[probF+4][59] = '+';
		}
	} else if (rG>=50 && rG<75){
		probX = 1+rand()%15;
		if((vet[probX+4][59]!='+' || vet[probX+4][59]!='X') && probX+4<16){
			vet[probX+4][59] = 'X';
		}
	}
}
void pisca_nave(char vet[ROWS][COLUMNS]){ /* pisca nave (caso o jogador aperte uma tecla invalida) */
	if(piscou==0){
		piscou = 1;
	} else if(piscou==5){
		piscou = 0;
	}
}
/* caso o jogador aperte uma invalido */
void penalidade(char vet[ROWS][COLUMNS]){
	int i, j;
	int c = RAND();
	if(c<50 && refY>4){
		refY--;
		gas-=2;
		for(i=0;i<=3;i++){
			for(j=0;j<=2;j++){
				if(vet[refY+1+i][refX+j]!='@' && vet[refY+1+i][refX+j]!='+' && vet[refY+1+i][refX+j]!='X'){
					vet[refY+i][refX+j] = vet[refY+1+i][refX+j];
				} else {
					vet[refY+i][refX+j] = ' ';
				}
			}
		}
		pisca_nave(vet);
	} else if(c>=50 && refY<14){
		for(i=3;i>=0;i--){
			for(j=0;j<=2;j++){
				if(vet[refY+i-1][refX+j]!='@' && vet[refY+i-1][refX+j]!='+' && vet[refY+i-1][refX+j]!='X'){
					vet[refY+i][refX+j] = vet[refY+i-1][refX+j];
				} else {
					vet[refY+i][refX+j] = ' ';
				}
			}
		}
		refY++;
		gas-=2;
		pisca_nave(vet);
	}
}
/* move o aviao e controla o tiro dos projeteis
   apenas entre as linhas 4 e 17
	*/
void m_aviao(char vet[ROWS][COLUMNS]){
	int i, j;
	int c = kbhit();
	if(c==1){
		int b = getch();
		if((b==119 || b==87) && (refY>4)){ /* w - mov pra cima */
			refY--;
			gas-=2;
			if(vet[refY-1][refX]=='X' || vet[refY][refX+1]=='X'){
				vidas--;
			} else if(vet[refY-1][refX]=='+' || vet[refY][refX+1]=='+'){
				gas+=40;
			}
			for(i=0;i<=3;i++){
				for(j=0;j<=2;j++){
					if(vet[refY+1+i][refX+j]!='@' && vet[refY+1+i][refX+j]!='+' && vet[refY+1+i][refX+j]!='X'){
						vet[refY+i][refX+j] = vet[refY+1+i][refX+j];
					} else {
						vet[refY+i][refX+j] = ' ';
					}
				}
			}
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}
		} else if ((b==115 || b==83) && (refY<14)){ /* s - mov pra baixo */
			if(vet[refY+3][refX]=='X' || vet[refY+2][refX+1]=='X'){
				vidas--;
			} else if(vet[refY+3][refX]=='+' || vet[refY+2][refX+1]=='+'){
				gas+=40;
			}
			for(i=3;i>=0;i--){
				for(j=0;j<=2;j++){
					if(vet[refY+i-1][refX+j]!='@' && vet[refY+i-1][refX+j]!='+' && vet[refY+i-1][refX+j]!='X'){
						vet[refY+i][refX+j] = vet[refY+i-1][refX+j];
					} else {
						vet[refY+i][refX+j] = ' ';
					}
				}
			}
			refY++;
			gas-=2;
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}
		} else if((b==32)) {/* espaco - tiro */
			if(atirando==1){ /* continua movimento dos tiros */
				mov_proj(vet);
			}		
			vet[refY+1][refX+2] = '>';
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
	for(i=0;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			if(vet[i][j]=='X'){ /* coloracao do X */
				printf("\x1b[31m" "%c" "\x1b[0m", vet[i][j]);
			} else if(vet[i][j]=='+'){ /* coloracao do + */
				printf("\x1b[32m" "%c" "\x1b[0m", vet[i][j]);
			} else if(vet[i][j]=='D' && piscou!=0 && piscou<5){ /* mostrar nave piscando */
				printf("\e[0;31m" "%c" "\e[0;0m", vet[i][j]);
				piscou++;
				pisca_nave(vet);
			} else {
				printf("%c",vet[i][j]);
			}
		}
		printf("\n");
	}
	printf("-------------------------------------------------------------\n");
	printf("  COMBUSTIVEL: %d | PONTOS: %d \n", gas, pontos);
	printf("-------------------------------------------------------------\n");
}
/* apenas para corrigir o erro da chamada do menu */
void menu_f();

/* tela de instrucoes */
void infos(){
	system(CLEAR);
	printf("\n\n\tInstrucoes:\n");
	printf(" W : move a nave para cima\n");
	printf(" S : move a nave para baixo\n");
	printf(" Espaco : atira\n");
	printf(" Atire nos obstaculos \x1b[31mX\x1b[0m, para ganhar pontos e sobreviver.\n");
	printf(" A nave gasta combustivel, mas ha recargas \x1b[32m+\x1b[0m durante o percurso.\n");
	printf(" Se voce ficar sem combustivel ou bater, game over.\n");
	printf(" Seus tiros gastam combustivel, entao fique atento!\n");
	printf(" Pressione qualquer botao para voltar ao menu.\n");
	int x = 1;
	while(x==1){
		int c = getch();
		if(c){
			x = 0;
			menu_f();
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
			menu_f();
		}
	}
}
/* execucao do jogo */
void jogar(){
	system(CLEAR);
	/* zera as variaveis de jogo */
	refY = 9;
	refX = 1;
	atirando = 0;
	gas = 400;
	vidas = 1;
	pontos = 0;
	char campo[ROWS][COLUMNS]= {{'.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','.','.','.','.','.',},
                            	{'@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ','@','.','.','.','.','@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ','@','.','.','.','.','@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ','@','.','.','.','.',},
                            	{' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ','@','@','@','@',' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ','@','@','@','@',' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ','@','@','@','@',},
                            	{' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ','D','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',},
                            	{' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ',' ','@','@','@',' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ',' ','@','@','@',' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ',' ','@','@','@',},
                            	{'@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ',' ','@','.','.','.','@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ',' ','@','.','.','.','@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ',' ','@','.','.','.',},
                            	{'.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','@','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','@','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','@','.','.','.','.',}};
    loopa = 1;
	while(loopa==1){
		usleep(velocidade);
		/*COORD newpos = {0,0};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),newpos);*/
		system(CLEAR);
		m_aviao(campo);
		m_cima(campo);
		m_baixo(campo);
		ger_ev(campo);
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
}
/* atualiza o menu principal */
void att(char vet[ROWS][COLUMNS], int tipo){
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
	for(i=0;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			printf("%c",vet[i][j]);
		}
		if(i!=ROWS-1){
			printf("\n");
		}
	}
	printf("- S : muda de alternativa | Enter = seleciona              -\n");
	printf("------------------------------------------------------------\n");
}
/* menu principal */
void menu_f(){
	system(CLEAR);
	char menu[ROWS][COLUMNS] = {{'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',},
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
    att(menu,sel);
    while(x==1){
    	int c = getch();
    	if(c==B_ENTER){
    		switch(sel){
    			case 1 : x=0;jogar(); break;
    			case 2 : x=0;infos(); break;
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

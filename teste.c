#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
	void usleep(__int64 usec) 
	{ 
   		HANDLE timer; 
    	LARGE_INTEGER ft; 

    	ft.QuadPart = -(10*usec); 

    	timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    	WaitForSingleObject(timer, INFINITE); 
    	CloseHandle(timer); 
	}
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
#define ROWS 21
#define COLUMNS 40

/*  TO DO LIST:
	BALANCEAMENTO DOS ENEMYS/FUEL
	PISCAR NAVE AO BATER E PERDER VIDA
	CORES NOS ENEMYS/FUEL/HP
	FAZER A NAVE BATER NOS OBSTACULOS E PAREDES NO MOVIMENTO VERTICAL
	MENU BONITO
*/
/* posição do bloco superior esquerdo do aviao
   tamanho do aviao : 3x2 (y,x) */
int refY = 9;
int refX = 1;
/*  VARIAVEIS DO JOGO
	loopa: estado do jogo (rodando ou nao)
	atirando: se há projetil em movimento
	gas: combustivel
	vidas: vidas do player
	km: distancia percorrida
*/
int loopa = 0;
int atirando = 0;
int gas = 400;
int vidas = 1;
float km = 0;
int pontos = 0;
/* move a parte de cima do campo 
	da primeira linha até a linha 4 da matriz
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
/* move os projeteis e faz colisao deles com os obstaculos/buffs*/
void mov_proj(char vet[ROWS][COLUMNS]){
	int i, j, stop_proj=1;
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
/* gera os inimigos e combustiveis, move, faz a colisao com a nave, e com os projeteis */
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
	if((int)km%15==0){
		int y = 1+(rand()%15);
		if((vet[y+4][39]!='+' || vet[y+4][39]!='X') && y+4<16){
			vet[y+4][39] = '+';
		}
	}
	if((int)km%3==0){
		int y = 1+(rand()%15);
		if((vet[y+4][39]!='+' || vet[y+4][39]!='X') && y+4<16){
			vet[y+4][39] = 'X';
		}
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
		if((b==72||b==119) && (refY>4)){ /* w/seta pra cima */
			refY--;
			gas-=2;
			for(i=0;i<=3;i++){
				for(j=0;j<=2;j++){
					if(vet[refY+1+i][refX+j]!='@'){
						vet[refY+i][refX+j] = vet[refY+1+i][refX+j];
					} else {
						vet[refY+i][refX+j] = ' ';
					}
				}
			}
			if(atirando==1){
				mov_proj(vet);
			}
		} else if ((b==80||b==115) && (refY<14)){ /* s/seta pra baixo */
			for(i=3;i>=0;i--){
				for(j=0;j<=2;j++){
					if(vet[refY+i-1][refX+j]!='@'){
						vet[refY+i][refX+j] = vet[refY+i-1][refX+j];
					} else {
						vet[refY+i][refX+j] = ' ';
					}
				}
			}
			refY++;
			gas-=2;
			if(atirando==1){
				mov_proj(vet);
			}
		} else if((b==32)) {/* espaco - tiro */
			if(atirando==1){
				mov_proj(vet);
			}		
			vet[refY+1][refX+2] = '>';
			atirando = 1;
			gas-=3;
		}
	} else {
		if(atirando==1){
			mov_proj(vet);
		}
	}
}
/* mostra o jogo */
void show(char vet[ROWS][COLUMNS]){
	int i,j;
	for(i=0;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			printf("%c",vet[i][j]);
		}
		printf("\n");
	}
	printf(" -----------------------------------------------\n");
	printf("|  GAS: %d | PONTOS: %d | KM: %.1f \n", gas, pontos, km);
	printf(" -----------------------------------------------\n");
}
/* menu */
void infos(){
	system(CLEAR);
	printf("\n\n\tInstrucoes:\n");
	printf(" W / seta para cima : movem a nave para cima\n");
	printf(" S / seta para baixo : movem a nave para baixo\n");
	printf(" Espaco : atira\n");
	printf(" Atire nos obstaculos, para ganhar pontos e sobreviver\n");
	printf(" Se voce bater, game over\n");
	printf(" A nave gasta combustivel, mas ha suprimentos de combustivel durante o percurso\n");
	printf(" Seus tiros gastam combustivel, entao fique atento!\n");
	printf(" Pressione <ENTER> para jogar\n");
	printf(" Pressione <ESC> para sair\n");
}
/* game over */
void game_over(char motivo[23]){
	system(CLEAR);
	printf("\n\n\tGAME OVER\n\t%s\n\tDISTANCIA PERCORRIDA: %.1f km\n\t",motivo,km);
	printf("PONTOS: %d\n\tPRESSIONE <ENTER> PARA VOLTAR AO MENU\n",pontos);
	int x = 1;
	while(x==1){
		int c = getch();
		if(c==B_ENTER){
			x = 0;
			infos();
		}
	}
}
/* execucao do jogo */
void jogar(){
	system(CLEAR);
	refY = 9;
	refX = 1;
	atirando = 0;
	gas = 300;
	vidas = 1;
	km = 0;
	pontos = 0;
	char campo[ROWS][COLUMNS]={{'.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','.','.','.','.','.',},
                               {'@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ','@','.','.','.','.','@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ','@','.','.','.','.',},
                               {' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ','@','@','@','@',' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ','@','@','@','@',},
                               {' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ','D','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@',' ',' ',' ',' ','@','@','@',' ',' ',' ',' ',' ',' ',' ',' ',},
                               {' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ',' ','@','@','@',' ',' ',' ','@','.','@',' ',' ','@','.','.','.','@',' ',' ',' ',' ','@','@','@',},
                               {'@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ',' ','@','.','.','.','@','@','@','.','.','.','@','@','.','.','.','.','.','@',' ',' ','@','.','.','.',},
                               {'.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','@','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','@','@','.','.','.','.',}};
	loopa = 1;
	while(loopa==1){
		usleep(50000);
		/*COORD newpos = {0,0};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),newpos);*/
		system(CLEAR);
		m_aviao(campo);
		m_cima(campo);
		m_baixo(campo);
		ger_ev(campo);
		show(campo);
		gas--;
		if(vidas<=0){
			loopa = 0;
			game_over("VOCE MORREU");
		}
		if(gas<=0){
			loopa = 0;
			game_over("SEU COMBUSTIVEL ACABOU");
		}
		km+=0.1;
	}
}
int main(){
    infos();
	int x = 1;
	while(x==1){
		int c = getch();
		if(c==B_ENTER){
			jogar();
		} else if(c==27){
			x = 0;
		}
	}
	return 0;
}

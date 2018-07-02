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
int atirando = 0;
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
/* move os projeteis e faz colisao deles com os obstaculos/combustiveis pt1*/
void mov_proj(char vet[ROWS][COLUMNS]){
	int i, j, stop_proj=1;
	/* stop_proj : caso nao haja nenhum tiro se movendo, seta atirando=0
		para nao executar essa funcao e economizar processamento */
	for(i=1;i<altura-1;i++){
		for(j=largura-2;j>1;j--){
			if(vet[i][j]=='X'||vet[i][j]=='F'){
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
					if(j==largura-2){
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
	for(i=1;i<altura-1;i++){
		for(j=0;j<largura;j++){
			if(vet[i][j]=='F' || vet[i][j]=='X'){
				if(vet[i][j-1]=='>'){
					if(vet[i][j]=='X'){
						pontos+=50;
					}
					vet[i][j-1]=' ';
					vet[i][j]=' ';
				} else if(j!=0 && vet[i][j-1]!='+' && vet[i][j-1]!='>'){
					vet[i][j-1] = vet[i][j];
				} else if(vet[i][j-1]=='+' && vet[i][j]=='X'){
					vidas--;
				} else if(vet[i][j-1]=='+' && vet[i][j]=='F'){
					gas+=40;
				}
				vet[i][j] = ' ';
			}
		}
	}
	int rG = RAND();
	int trG; /* random generator temporario, apenas para escolher a posicao */  
	if(rG>=0 && rG <=probF){
		trG = 1+rand()%9;
		if((vet[trG][largura-2]!='F' || vet[trG][largura-2]!='X') && trG<altura-1){
			vet[trG][largura-2] = 'F';
		}
	} else if (rG>probF && rG<=probX){
		trG = 1+rand()%9;
		if((vet[trG][largura-2]!='F' || vet[trG][largura-2]!='X') && trG<altura-1){
			vet[trG][largura-2] = 'X';
		}
	}
}
void pisca_nave(char vet[ROWS][COLUMNS]){ /* pisca nave (caso o jogador aperte uma tecla invalida) */
	if(piscou==0){
		piscou = 1;
	} else if(piscou==2){
		piscou = 0;
	}
}
/* caso o jogador aperte uma invalido */
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
   apenas entre as linhas 4 e 17
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
	printf(" A nave gasta combustivel, mas ha recargas \x1b[32mF\x1b[0m durante o percurso.\n");
	printf(" Se voce ficar sem combustivel ou bater, game over.\n");
	printf(" Seus tiros gastam combustivel, entao fique atento!\n");
	printf(" Apertar um botao invalido acarretara em um movimento aleatorio.\n");
	printf(" Pressione qualquer botao para voltar ao menu.\n");
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

/* execucao do jogo */
void jogar(){
	system(CLEAR);
	int taltura,tlargura,tprobX,tprobF,tprobO,tprobT,tvidaO,tmunicaoT,tvelocidade;

	/* le o arquivo de cfg */
	FILE* fd;
	fd = fopen("config.txt","r");
	fscanf(fd, "%d %d %d %d %d %d %d %d %d %d",&altura,&largura,&probX,&probF,&probO,&probT,&vidaO,&municaoT,&velocidade,&ranked);

	/* muda as variaveis de jogo */
	atirando = 0;
	pontos = 0;
	vidas = 1;
	gas = 400;
	vidas = 1;
	pontos = 0;

	/* se for ranked, volta para esses valores e salva os do arquivo nos temporarios*/
	if(ranked==1){
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
	gerar_campo(campo);
	loopa = 1;
	while(loopa==1){
		usleep(velocidade);
		system(CLEAR);
		m_aviao(campo);
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
	if(ranked==1){
		altura = taltura;
		largura = tlargura;
		probX = tprobX;
		probF = tprobF;
		probO = tprobO;
		probT = tprobT;
		vidaO = tvidaO;
		municaoT = tmunicaoT;
		velocidade = tvelocidade;
	}
}

void savesettings(){
	FILE* fd;
	fd = fopen("config.txt","w");
	fprintf(fd, "%d %d %d %d %d %d %d %d %d %d\n",altura,largura,probX,probF,probO,probT,vidaO,municaoT,velocidade,ranked);
	fprintf(fd, "#altura - largura - probX - probF - probO - probT - vidaO - municaoT - velocidade - ranked");
	fclose(fd);
}

void cfg_tabuleiro(){
	system(CLEAR);
	int x = 1;
	char seletor[] = {'>',' ',' ',' '};
	int c;

	printf("\n\n");
	printf("\tConfiguracoes do tabuleiro\n\n");
	printf("\t%c Altura atual: %d\n",seletor[0],altura);
	printf("\t%c Largura atual: %d\n",seletor[1],largura);
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
			printf("\tConfiguracoes do tabuleiro\n\n");
			printf("\t%c Altura atual: %d\n",seletor[0],altura);
			printf("\t%c Largura atual: %d\n",seletor[1],largura);
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
				printf("\tConfiguracoes do tabuleiro\n\n");
				printf("\t%c Altura atual: %d\n",seletor[0],altura);
				printf("\t%c Largura atual: %d\n",seletor[1],largura);
				printf("\t%c Velocidade: %d us\n",seletor[2],velocidade);
				printf("\t%c Voltar\n",seletor[3]);
				printf("\n\tAperte enter para selecionar.\n");
				savesettings();
			}
		}
	}

}

void cfg_npcs(){
	system(CLEAR);
	int x = 1;
	char seletor[] = {'>',' ',' ',' ',' ',' ',' '};
	int c;

	printf("\n\n");
	printf("\tConfiguracoes dos inimigos/combustivel\n\n");
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
			printf("\tConfiguracoes dos inimigos/combustivel\n\n");
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
				printf("\tConfiguracoes dos inimigos/combustivel\n\n");
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
	printf("\tConfiguracoes\n\n");
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
			printf("\tConfiguracoes\n\n");
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
				printf("\tConfiguracoes\n\n");
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
	printf("- S : muda de alternativa | Enter = seleciona              -\n");
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
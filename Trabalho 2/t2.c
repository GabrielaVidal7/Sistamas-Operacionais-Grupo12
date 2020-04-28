#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

#define num_max 10

sem_t  mutex;	//semaforo utilizado para garantir que haja exclusao mutua na regiao critica (a regiao do buffer)
sem_t vazio;	//semaforo utilizado para controlar as posicoes do buffer que estao vazias
sem_t cheio;	//semaforo utilizado para controlar as posicoes do buffer que estao preenchidas

char *buffer[num_max];	//buffer com no maximo 10 espacos
int pos_cheia;	//valor que indica a proxima posicao preenchida no buffer
int pos_vazia;	//valor que indica a proxima posicao vazia no buffer
int total;		//variavel utilizada para saber quantos arquivos estao presentes no buffer

void *usuario(void *);
void *impressora(void *);

int main(void){
	srand(time(NULL));

	total = 0;
	pos_cheia = 0;
	pos_vazia = 0;

	//Comandos para inicializar os semaforos utilizados no codigo
	sem_init (&mutex, 0 , 1);
	sem_init(&vazio, 0, num_max);
	sem_init(&cheio, 0, 0);
	
	pthread_t thread1;
	pthread_t thread2;	

	//Cria as threads para serem utilizadas no trabalho
	pthread_create(&thread1, 0, usuario, NULL);
	pthread_create(&thread2, 0, impressora, NULL);

	//Comandos utilizados para nao finalizar as threads em um momento indesejado
	pthread_join(thread1,0);
	pthread_join(thread2,0);
	
	exit(0);
}


//FUncao que pega do usuario os arquivos para serem impressos
char *arquivo_novo()
{
	char *arquivo = (char*) malloc(sizeof(char)*64);
	printf("Insira o nome do arquivo para impressao\n");
	scanf("%s", arquivo);
	return arquivo;
}

//Funcao que indica se o arquivo na fila foi impresso
void consume_item(char *item)
{
	printf("\nImprimindo arquivo: %s\n", item);
	free(item);
}

//Função que insere o arquivo no buffer
void insere_arquivo(char *val)
{
	if(total < num_max)	{	//Enquanto o buffer nao estiver cheio, e possivel inserir mais um arquivo para ser impresso
		buffer[pos_vazia] = val;
		pos_vazia = (pos_vazia + 1) % num_max; 
		total++;
		if(total == num_max)
			printf("\nBuffer cheio, aguarde para adicionar mais um arquivo\n");
	}
}

//Função que remove o arquivo do buffer, pois ja foi impresso
char *remove_item() {
	char *valor;
	if(total > 0) {	//Enquanto existem arquivos para serem impressos
		valor = buffer[pos_cheia];
		pos_cheia = (pos_cheia + 1) % num_max;	//Funcao utilizada para nao ultrapassar o limite do buffer
		total--;
		return valor;
	}
}

//Função que serve como produtor do exercicio
void *usuario(void *p_arg) {
	char *item;	//Nome do arquivo a ser impresso
	
	while(TRUE) {
		item = arquivo_novo();	//Retorna o nome do arquivo que será impresso
		
		sem_wait(&vazio);	//Faz o semaforo dormir
		sem_wait(&mutex);
		insere_arquivo(item);
		sem_post(&mutex);	//Faz o semaforo acordar
		sem_post(&cheio);
	}
}

//Funcao que serve como consumidor, imprimindo os arquivos presentes na lista
void *impressora(void *p_arg) {
	char *item;

	while(TRUE) {
		sem_wait(&cheio);
		sem_wait(&mutex);
		item = remove_item();
		sem_post(&mutex);
		sem_post(&vazio);
		consume_item(item);
		sleep(5);
	}
}
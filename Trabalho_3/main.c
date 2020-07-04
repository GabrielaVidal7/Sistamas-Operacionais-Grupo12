#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>

typedef struct{
	int numPags;
	int *pagRam;//Lista dos links com paginas fisicas
	bool *R;//Lista dos bits R
}tabela;

typedef struct p{
  int tam;//tamanho do processo
	tabela tabela;//tabela de enderecos
  int num; //numero do processo
  int mode; // 0 - > CPU; 1 -> I/O;
  struct p *prox;  //ponteiro para o próximo processo
}processo;

typedef struct{
  processo *processo;//ponteiro para o processo da lista
  int pag;//pagina virtual que representa
	bool R;// Bit de segunda chance
}moldura; // o tamanho da moldura é definido pelo usuário

typedef struct m{
  processo *processo;//ponteiro para o processo dono
  int pag;//Pagina virtual que representa
	struct m *nxt;//Proximo pag armazenada em disco
}molduraDisco; // o tamanho da moldura é definido pelo usuário

int end_to_pag(int end, int sizeEnd, int tamPag){
	end*=sizeEnd;
	end/=8;
	end/=1024;
	end/=tamPag;

	printf("\n** pagina %d **\n\n",end);

	return end;
}

//Função relógio
int relogio(moldura lista_mold[], molduraDisco **disco, int atual, int tam_ram, processo* proc, int pag){
	//lista_mold - a memRAM
	//disco - o memDISC
	//atual - posicao atual da lista circular da memoria ram
	//tam_ram - numero de molduras da memRAM
	//proc - ponteiro para o processo que esta pedindo uma pagina
	//pag - numero da pagina virtual que foi pedida

	molduraDisco *prev;
	molduraDisco *tmp;
	
	printf("Procurando pagina para alocar\n");
	while(lista_mold[atual].R!=0){
		printf("Pagina %d ocupada\n",atual);
		lista_mold[atual].R=0;
		atual++;
		atual%=tam_ram;
	}
	printf("Pagina %d selecionada\n",atual);
	
	if(lista_mold[atual].processo!=NULL){
		printf("Pagina %d estava sendo usada por P%d\n",atual,lista_mold[atual].processo->num);		
		tabela tabela = lista_mold[atual].processo->tabela;
		int size = lista_mold[atual].processo->tam;
		
		printf("Copiando para disco\n");
		
		if(*disco!=NULL){
			tmp = *disco;
			while(tmp->nxt!=NULL){
				tmp=tmp->nxt;
			}
			tmp->nxt=calloc(1, sizeof(molduraDisco));
			tmp=tmp->nxt;
		}else{
			*disco=calloc(1, sizeof(molduraDisco));
			tmp=*disco;
		}

		tmp->pag=lista_mold[atual].pag;
		tmp->processo=lista_mold[atual].processo;

		for(int i=0; i<size;i++){
			if(tabela.pagRam[i]==atual){
				tabela.R[i]=0;
				break;
			}
		}
	}

	if(proc->tabela.pagRam[pag]!=-1){
		printf("Pagina esta guardada na memoria secundaria\n");
		printf("Procurando em disco\n");
		prev=NULL;
		tmp=*disco;
		while(tmp->processo!=proc && tmp->pag!=pag){
			prev=tmp;
			tmp=tmp->nxt;
		}
		printf("Recuperando dados do disco\n");
		if(prev!=NULL){
			prev->nxt=tmp->nxt;
		}else{
			*disco=tmp->nxt;			
		}
		
		free(tmp);
	}



	lista_mold[atual].processo=proc;
	lista_mold[atual].pag=pag;

	proc->tabela.pagRam[pag]=atual;
	proc->tabela.R[pag]=1;

	printf("\n");

	return atual;
}

moldura *cria_moldura(int pagP){
  moldura *mold;
  
	mold=(moldura*)calloc(pagP, sizeof(moldura));
  return mold;
}

//Função que inicializa um processo
processo *cria_processo(int size, int tamPag, int num, processo *first){
	processo *proc;
	int num_pags;

	proc=(processo*)calloc(1,sizeof(processo));

	proc->tam=size;
	proc->num=num;
  proc->prox=first;	//Fica na primeira posição da lista encadeada
  proc->mode = 0; // inicia como CPU
  num_pags=ceil(((float)size)/((float)tamPag)); // numero de paginas ocupadas pelo processo

	proc->tabela.numPags=num_pags;
	proc->tabela.R=(bool*)calloc(num_pags, sizeof(bool));
	proc->tabela.pagRam=(int*)calloc(num_pags, sizeof(int));
  for(int i=0;i<num_pags;i++){
		proc->tabela.pagRam[i]=-1;
	}
  return proc;
}

//Libera memória alocada para o processo proc
void del_processo(processo *proc, processo **first, moldura listaMold[], int tam_ram, molduraDisco **DiscFirst){
  processo *atual = *first;
  molduraDisco *DiscAnterior=NULL; //ponteiro anterior ao ponteiro do Disco que contem proc
  molduraDisco *DiscAt=*DiscFirst; //ponteiro atual do Disco que contem proc

  if(proc==*first){    //Encontra na lista encadeada o ponteiro anterior ao ponteiro do processo que será deletado
    *first=(*first)->prox;
  }else{
    while(atual->prox!=proc && atual->prox!=NULL) {
      atual=atual->prox;
    }
    if(atual->prox==NULL){
      printf("Erro! Processo que será deletado não existe!\n");
      return;
    }
    atual->prox=proc->prox;
  }

  for(int i=0; i<tam_ram; i++){   //Libera a memória RAM que o processo ocupava
    if(listaMold[i].processo!=NULL && listaMold[i].processo->num==proc->num){
      listaMold[i].pag=0;
      listaMold[i].R=0;
    }
  }

  //Função para liberar a memória em disco do processo
  while (DiscAt!=NULL){
    if(DiscAt->processo==proc){
      if(DiscAt==*DiscFirst){
        DiscAt=DiscAt->nxt;
        free (*DiscFirst);
        *DiscFirst=DiscAt;
      }else{
        DiscAnterior->nxt=DiscAt->nxt;
        free(DiscAt);
        DiscAt=DiscAt->nxt;
      }
    }else{
			DiscAnterior=DiscAt;
			DiscAt=DiscAt->nxt;
		}
	}
  
	free(proc->tabela.R);
	free(proc->tabela.pagRam);
	free(proc);
}

//suspende processo
int suspproc(int nSusp, processo *proc, processo **first, moldura listaMold[], int tam_ram, molduraDisco **DiscFirst){ 
  del_processo(proc,first,listaMold,tam_ram,DiscFirst);
  
  nSusp++;

  return nSusp;
}

//Função que printa informações da memória RAM e secundária, além da tabela de páginas associadas
void print_dados(processo proc, int tamP, int nSusp, int totProc, int pagP, molduraDisco *memDISC, moldura memRAM[]){
  int qtdPdisc=0;	//valor que guarda a quantidade de páginas de disco usadas
	int qtdPram=0;	//valor que guarda a quantidade de páginas de RAM usadas

	//Descobre a quantidade de páginas que estão sendo usadas na memória secundária atualmente
  while (memDISC!=NULL){
    qtdPdisc++;
    memDISC=memDISC->nxt;
  }

	//Descobre a quantidade de páginas que estão sendo usadas na memória primária atualmente
	for(int i=0; i<pagP; i++){
		if(memRAM[i].processo!=NULL){
			qtdPram++;
		}
	}
	
  printf("Número de processos suspensos: %d\n", nSusp);
  printf("Número de processos executando: %d\n", totProc-nSusp);
  printf("Páginas de memória secundária alocadas: %d\n", qtdPdisc);
  printf("Páginas de memória primária alocadas: %d\n", qtdPram);

  printf("Página virtual      Página Física					R\n");  //print da tabela de páginas do processo
  for(int i=0; i<proc.tabela.numPags; i++){
    printf("     %d                    %d				  		%d\n", i, proc.tabela.pagRam[i], proc.tabela.R[i]);
  }
	printf("\n");
}

//Função chamada quando será lido ou escrito no processo definido
int comando_RW(processo *proc, int endereco, int tamP, int nSusp, int totProc, int sizeEnd, int pagP, molduraDisco **memDISC, moldura *memRAM, int *atualLista, char cmd, processo **listaProcs){
  int pagEnd = end_to_pag(endereco, sizeEnd, tamP); // Pagina relativa ao endereço

  if(proc->tabela.numPags < pagEnd || pagEnd <0){
    printf("Não é possível realizar o comando na memória de P%d\n", proc->num);
		printf("Processo %d suspenso\n", proc->num);
    nSusp=suspproc(nSusp, proc,listaProcs,memRAM,pagP,memDISC); // suspende o processo
  }else{
    if(proc->tabela.R[pagEnd]==0){
      (*atualLista) = relogio(memRAM, memDISC, *atualLista, pagP, proc, pagEnd);
    }else{
			memRAM[proc->tabela.pagRam[pagEnd]].R=1;
		}

		print_dados(*proc, tamP, nSusp, totProc, pagP, *memDISC, memRAM);

    switch(cmd){
      case 'R':
        printf("\nP%d executando em leitura\n", proc->num);
        break;
      case 'W':
        printf("\nP%d executando em escrita\n", proc->num);
        break;
    }
  }
  return nSusp;
}

void functionP(processo *proc,int tamP, int nSusp, int totProc, int pagP, molduraDisco *memDISC, moldura *memRAM, int num){ //Intrução CPU
  print_dados(*proc, tamP, nSusp, totProc, pagP, memDISC, memRAM);
	printf("P%d executando instrução %d em CPU\n", proc->num, num);
	proc->mode = 0;		//Flag setada para processo executando em CPU
}

void functionI(processo *proc, int tamP, int nSusp, int totProc, int pagP, molduraDisco *memDISC, moldura *memRAM, int num){ // Instrução I/O
  print_dados(*proc, tamP, nSusp, totProc, pagP, memDISC, memRAM);
	printf("P%d executando instrução %d de I/O\n", proc->num, num);
	proc->mode = 1;
}

int main(void) {
  FILE *arq;
	moldura *memRAM; // Lista para mem primaria
	molduraDisco *memDISC=NULL; // Lista para mem secundaria
	int id;
	char comando;
	int num;
  processo *proc = NULL;
  processo *atual;
  int procSuspenso = 0; //numero de processos suspensos
	int totProc=0;
  int atualLista = 0; // Guarda
	
  //Configuração do usuário
  int tam, elog, pagP, pagS;
  printf("Tamanho das páginas e quadros de página: ");
  scanf("%d", &tam);// tamanho padrao de paginas e molduras em kBs
  printf("\nTamanho em bits do endereco logico: ");
  scanf("%d", &elog);// tamanho do endereço logico em bits
  printf("\nQuantas paginas de memoria fisica serao alocadas: ");
  scanf("%d", &pagP);// Tamanho da Memoria RAM em paginas
  printf("\nTamanho maximo da memoria secundaria: ");
  scanf("%d", &pagS);// Tamanho do Disco em paginas

	memRAM=cria_moldura(pagP);
	printf("moldura criada\n");

	arq = fopen("entrada.txt", "r");
	while(fgetc(arq)!=EOF){
		fscanf(arq,"%d %c %d ",&id,&comando,&num);
		printf("%d %c %d\n",id,comando,num);

		switch(comando){
			case 'C':
        proc = cria_processo(num, tam, id, proc);
        totProc++;
				printf("processo criado\n");
				break;
        
			case 'R':
        atual=proc;
        while(atual!=NULL && atual->num!=id){
          atual=atual->prox;
        }
        if(atual==NULL){
          printf("Processo não inicializado\n");
          break;
        }
        procSuspenso=comando_RW(atual, num, tam, procSuspenso, totProc, elog, pagP, &memDISC, memRAM, &atualLista, comando,&proc);
				break;
        
			case 'W':
        atual=proc;
        while(atual!=NULL && atual->num!=id){
          atual=atual->prox;
        }
        if(atual==NULL){
          printf("Processo não inicializado\n");
          break;
        }
        procSuspenso=comando_RW(atual, num, tam, procSuspenso, totProc, elog, pagP, &memDISC, memRAM, &atualLista, comando,&proc);
				break;

			case 'P':
        atual=proc;
        while(atual!=NULL && atual->num!=id){
          atual=atual->prox;
        }
        if(atual==NULL){
          printf("Processo não inicializado\n");
          break;
        }
        functionP(atual, tam, procSuspenso, totProc, pagP, memDISC, memRAM, num);
				break;

			case 'I':
        atual=proc;
        while(atual!=NULL && atual->num!=id){
          atual=atual->prox;
        }
        if(atual==NULL){
          printf("Processo não inicializado\n");
          break;
        }
        functionI(atual, tam, procSuspenso, totProc, pagP, memDISC, memRAM, num);
				break;
		}
	}

	free(memRAM);
	fclose(arq);
  return 0;
}
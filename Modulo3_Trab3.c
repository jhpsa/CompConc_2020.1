#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

#ifndef _CLOCK_TIMER_H
#define _CLOCK_TIMER_H

#include <sys/time.h>
#define BILLION 1000000000L

#define GET_TIME(now) { \
   struct timespec time; \
   clock_gettime(CLOCK_MONOTONIC, &time); \
   now = time.tv_sec + time.tv_nsec/1000000000.0; \
}
#endif

#define NTHREADS 4 //numero de threads, a primeira para leitura do arquivo e escrita no buffer e as outras 3 para implementar as buscas

int M; //numero de blocos
int N; //tamanho de cada bloco

char buffer[1000]; //buffer compartilhado pelas threads

sem_t cond12, cond13, cond14, cond21, cond31, cond41, cond23, cond34; //semaforos para sincronizar a ordem de execucao das threads

//funcao executada pela thread 1, que vai ler o arquivo e escrever no buffer em blocos
void *t1 (void *threadid){
    int *tid = (int*) threadid;
    
    FILE *arq;
    arq = fopen("arquivo.txt", "r"); //abre o arquivo para ser lido
    
    //descobre quantas casas decimais tem o primeiro numero inteiro (de valor M*N) do arquivo
    double tam = M*N;
    int cont = 0;
    while(tam>=10){
      tam /= 10;
      cont++;
    }
  
    fread(buffer, cont + 1, 1, arq); //le esse primeiro numero para que ele nao seja contabilizado no resto das leituras
  
    for(int m = 0; m < M; m++){ //for para repetir M vezes (numero de blocos a serem lidos)
      fread(buffer, 1, N * 2, arq); //le um bloco de tamanho 2*N (para contabilizar cada espaço em branco antes de um numero)
      				     //e escreve ele no buffer compartilhado
      				     
      //apos ler o bloco libera as threads de busca
      sem_post(&cond12);
      sem_post(&cond13);
      sem_post(&cond14);
      
      //espera as threads de busca terminarem para terminar o for e seguir para a proxima repeticao
      sem_wait(&cond21);
      sem_wait(&cond31);
      sem_wait(&cond41);
    }
  
    fclose(arq); //fecha o arquivo
    
    pthread_exit(NULL);
}

//funcao executada pela thread 2, que vai buscar a maior sequencia de numeros repetidos seguidos, sua posicao inicial e o numero repetido
void *t2 (void *threadid){
    int *tid = (int*) threadid;
    
    char buffer2[M*N*2]; //cria um array para receber e ir juntando os blocos do buffer compartilhado
    
    //variaveis para controlar o valor atual de tudo que ira ser buscado
    int pos = M*N*2;
    int pos_max = 0;
    int tam_seq = 0;
    int tam_max = 0;
    char valor = '\0';
    char valor_max = '\0';
    
    for(int m = 0; m < M; m++){ //for para repetir M vezes (numero de blocos a serem lidos)
    
        sem_wait(&cond12); //espera a thread 1 terminar de ler o bloco e escrever no buffer compartilhado
        
        //copia o bloco do buffer e adiciona ao proprio array
        for(int i = 0; i < N * 2; i++){
          buffer2[i + m * N * 2] = buffer[i];
        }

        //faz as buscas no bloco atual
        for(int i = 1; i < N * 2; i+=2){
          if(buffer2[i + m * N * 2]==buffer2[i + m * N * 2 - 2]){
            if(buffer2[i + m * N * 2]!=valor){
              if(tam_seq > tam_max){
                pos_max = pos;
                pos = M*N*2;
                tam_max = tam_seq;
                tam_seq = 1;
                valor_max = valor;
                valor = '\0';
              }
              pos = i + m * N * 2 - 2;
              valor = buffer2[i + m * N * 2];
              tam_seq = 1;
            }
            if(i + m * N * 2 < pos){
              pos = i + m * N * 2 - 2;
            }
            tam_seq++;
          }
        }
        if(tam_seq > tam_max){
          pos_max = pos;
          tam_max = tam_seq;
          valor_max = valor;
        }
        sem_post(&cond21); //libera a thread 1 para ler o proximo bloco
    }
    
    //imprime o resultado das buscas
    if(valor_max=='\0'){
      printf("Não há uma sequência de valores idênticos.\n");
    }
    else{
      printf("Maior sequência de valores idênticos: %d %d %d\n", (pos_max+1)/2, tam_max, valor_max-48);
    }
    
    sem_post(&cond23); //libera a thread 3 para imprimir o resultado dela
    
    pthread_exit(NULL);
}

//funcao executada pela thread 3, que vai buscar o numero de sequencias de 3 numeros repetidos seguidamente
void *t3 (void *threadid){
    int *tid = (int*) threadid;
    
    char buffer3[M*N*2]; //cria um array para receber e ir juntando os blocos do buffer compartilhado
    
    //variaveis para controlar o valor atual de tudo que ira ser buscado
    int iguais = 1;
    int qtdd = 0;
    
    for(int m = 0; m < M; m++){ //for para repetir M vezes (numero de blocos a serem lidos)
    
        sem_wait(&cond13); //espera a thread 1 terminar de ler o bloco e escrever no buffer compartilhado
        
        //copia o bloco do buffer e adiciona ao proprio array
        for(int i = 0; i < N * 2; i++){
          buffer3[i + m * N * 2] = buffer[i];
        }
        
        //faz as buscas no bloco atual
        for(int i = 1; i < N * 2; i+=2){
          if(buffer3[i + m * N * 2]==buffer3[i + m * N * 2 - 2]){
            iguais++;
          }
          else{
            int count = 0;
            while(iguais>=3){
              iguais -= 3;
              count++;
            }
            qtdd += count;
            iguais = 1;
          }
        }
        sem_post(&cond31); //libera a thread 1 para ler o proximo bloco
    }
    
    sem_wait(&cond23); //espera a thread 2 imprimir o resultado dela
    
    printf("Quantidade de triplas: %d\n", qtdd); //imprime o resultado das buscas
    
    sem_post(&cond34); //libera a thread 4 para imprimir o resultado dela
    
    pthread_exit(NULL);
}

//funcao executada pela thread 4, que vai buscar o numero de sequencias do tipo 0 1 2 3 4 5
void *t4 (void *threadid){
    int *tid = (int*) threadid;
    
    char buffer4[M*N*2]; //cria um array para receber e ir juntando os blocos do buffer compartilhado
    
    //variaveis para controlar o valor atual de tudo que ira ser buscado
    int sequencias = 0;
    int qtdd = 0;
    
    for(int m = 0; m < M; m++){ //for para repetir M vezes (numero de blocos a serem lidos)
    
        sem_wait(&cond14); //espera a thread 1 terminar de ler o bloco e escrever no buffer compartilhado
        
        //copia o bloco do buffer e adiciona ao proprio array
        for(int i = 0; i < N * 2; i++){
          buffer4[i + m * N * 2] = buffer[i];
        }
        
        //faz as buscas no bloco atual
        for(int i = 1; i < N * 2; i+=2){
          if(buffer4[i + m * N * 2]-1==buffer4[i + m * N * 2 - 2]){
            sequencias++;
            if(sequencias==5){
              qtdd++;
            }
          }
          else{
            sequencias = 0;
          }
        }
        sem_post(&cond41); //libera a thread 1 para ler o proximo bloco
    }
    
    sem_wait(&cond34); //espera a thread 3 imprimir o resultado dela
    
    printf("Quantidade de ocorrências da sequência <012345>: %d\n", qtdd); //imprime o resultado das buscas
    
    pthread_exit(NULL);
}

//funcao principal
int main(int argc, char *argv[]) {

  //variaveis para medicao de tempo
  double ini, fim;
  double t_seq, t_conc;

  //pegando os inputs do usuario
  int tam_entrada;
  char tipo_entrada;
  printf("Escolha a quantidade de inteiros da sequência de entrada: ");
  scanf("%d", &tam_entrada);
  int count = 0;
  int size = tam_entrada;
  while(size>=10){
    size /= 10;
    count++;
  }
  char size_entrada[count+1];
  sprintf(size_entrada, "%d", tam_entrada);
  char entrada[tam_entrada*2+strlen(size_entrada)]; //sequencia de numeros inteiros do arquivo
  for(int i = 0; i < strlen(size_entrada); i++){
    entrada[i] = size_entrada[i];
  }
  printf("Deseja digitar a sequência de inteiros da entrada? (S/N): ");
  scanf("%s", &tipo_entrada);
  srand(time(NULL));
  char entrada0[tam_entrada];
  if(tipo_entrada=='S' || tipo_entrada=='s'){
    for(int i = strlen(size_entrada); i < tam_entrada*2+strlen(size_entrada); i+=2){
      entrada[i] = ' ';
      printf("Digite um inteiro (entre 0 e 5) para a sequência: ");
      scanf("%s", &entrada[i+1]);
    }
  }
  else{
    for(int i = strlen(size_entrada); i < tam_entrada*2+strlen(size_entrada); i+=2){
      entrada[i] = ' ';
      int a = rand();
      while(a>=6){
        a-=6;
      }
      entrada[i+1] = a + '0';
    }
  }
  
  printf("Digite o número de blocos em que a entrada do arquivo deve ser dividida: ");
  scanf("%d", &M);
  N = tam_entrada / M;
  
  FILE *arq;
  arq = fopen("arquivo.txt", "w"); //cria um arquivo e abre ele para ser escrito
  fwrite(entrada, 1, sizeof(entrada), arq); //escreve a entrada no arquivo
  fclose(arq); //fecha o arquivo
  
  GET_TIME(ini);
  //programa sequencial
  FILE *arq0;
  arq0 = fopen("arquivo.txt", "r");
  
  double tam = M*N;
  int cont = 0;
  while(tam>=10){
    tam /= 10;
    cont++;
  }
  
  char buffer0[M*N*2];
  char buffer00[M*N*2];
  
  int pos = M*N*2;
  int pos_max = 0;
  int tam_seq = 0;
  int tam_max = 0;
  char valor = '\0';
  char valor_max = '\0';
  
  int iguais = 1;
  int qtdd = 0;
  
  int sequencias = 0;
  int qtdd0 = 0;
  
  fread(buffer0, cont + 1, 1, arq0);
  
  for(int m = 0; m < M; m++){
    fread(buffer0, 1, N * 2, arq0);
    
    for(int i = 0; i < N * 2; i++){
      buffer00[i + m * N * 2] = buffer0[i];
    }
    for(int i = 1; i < N * 2; i+=2){
      if(buffer00[i + m * N * 2]==buffer00[i + m * N * 2 - 2]){
        if(buffer00[i + m * N * 2]!=valor){
          if(tam_seq > tam_max){
            pos_max = pos;
            pos = M*N*2;
            tam_max = tam_seq;
            tam_seq = 1;
            valor_max = valor;
            valor = '\0';
          }
          pos = i + m * N * 2 - 2;
          valor = buffer00[i + m * N * 2];
          tam_seq = 1;
        }
        if(i + m * N * 2 < pos){
          pos = i + m * N * 2 - 2;
        }
        tam_seq++;
      }
      
      if(buffer00[i + m * N * 2]==buffer00[i + m * N * 2 - 2]){
        iguais++;
      }
      else{
        int count = 0;
        while(iguais>=3){
          iguais -= 3;
          count++;
        }
        qtdd += count;
        iguais = 1;
      }
      
      if(buffer00[i + m * N * 2]-1==buffer00[i + m * N * 2 - 2]){
        sequencias++;
        if(sequencias==5){
          qtdd0++;
        }
      }
      else{
        sequencias = 0;
      }
    }
    if(tam_seq > tam_max){
      pos_max = pos;
      tam_max = tam_seq;
      valor_max = valor;
    } 
  }
  
  fclose(arq0);
  
  GET_TIME(fim);
  t_seq = fim-ini;
  //printf("Tempo sequencial: %lf\n", t_seq);


  GET_TIME(ini);
  //programa concorrente
  pthread_t tid[NTHREADS];
  int *id[4], t;

  for (t=0; t<NTHREADS; t++) {
    if ((id[t] = malloc(sizeof(int))) == NULL) {
       pthread_exit(NULL); return 1;
    }
    *id[t] = t+1;
  }

  //inicia os semaforos
  sem_init(&cond12, 0, 0);
  sem_init(&cond13, 0, 0);
  sem_init(&cond14, 0, 0);
  sem_init(&cond21, 0, 0);
  sem_init(&cond31, 0, 0);
  sem_init(&cond41, 0, 0);
  sem_init(&cond23, 0, 0);
  sem_init(&cond34, 0, 0);

  //cria as quatro threads
  if (pthread_create(&tid[0], NULL, t1, (void *)id[0])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  if (pthread_create(&tid[1], NULL, t2, (void *)id[1])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  if (pthread_create(&tid[2], NULL, t3, (void *)id[2])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  if (pthread_create(&tid[3], NULL, t4, (void *)id[3])) { printf("--ERRO: pthread_create()\n"); exit(-1); }

  //espera todas as threads terminarem
  for (t=0; t<NTHREADS; t++) {
    if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
    free(id[t]);
  }
  GET_TIME(fim);
  t_conc = fim-ini;
  //printf("Tempo concorrente: %lf\n", t_conc);
  
  //printf("Ganho de desempenho: %lf\n", t_seq/t_conc);
   
  pthread_exit(NULL);
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "timer.h"

long long int N; //dimensao do vetor de entrada (numero de termos utilizados no calculo de pi)
int T; //numero de threads
double *vetor; //vetor de entrada com dimensao N

//fluxo das threads
void * tarefa(void * arg){
    int id = (long int) arg; //identificador da thread
    double *soma_local; //variavel local do calculo concorrente de pi
    long long int tam_bloco = N / T; //tamanho do bloco de cada thread
    long long int ini = id * tam_bloco; //elemento inicial do bloco da thread
    long long int fim; //elemento final (nao processado) do bloco da thread
    
    soma_local = (double*) malloc(sizeof(double));
    if(soma_local == NULL){
        fprintf(stderr, "ERRO--malloc\n"); exit(1);
    }
    *soma_local = 0;
    if(id == T-1) fim = N;
    else fim = ini + tam_bloco; //trata o resto se houver
    //soma os elementos do bloco da thread
    for(long long int i = ini; i<fim; i++){
        *soma_local += vetor[i];
    }
    //retorna o resultado da soma local
    pthread_exit((void *) soma_local);
}

//fluxo principal
int main(int argc, char *argv[]){
    double pi_seq = 0; //calculo sequencial do valor de pi
    double pi_conc = 0; //calculo concorrente do valor de pi
    double ini, fim; //tomada de tempo
    pthread_t *tid; //identificadores das threads no sistema
    double *retorno; //valor de retorno das threads
    
    //recebe e valida os parametros de entrada (numero de termos para o calculo de pi, numero de threads)
    if(argc<3){
        fprintf(stderr, "Digite: %s <numero de termos pi> <numero de threads>\n", argv[0]);
        return 1;
    }
    N = atoll(argv[1]);
    T = atoi(argv[2]);
    //aloca o vetor de entrada
    vetor = (double *) malloc(sizeof(double) * N);
    if(vetor == NULL){
        fprintf(stderr, "ERRO--malloc\n");
        return 2;
    }
    //preenche o vetor de entrada com os primeiros N valores dos termos do calculo de pi
    int flag = 1;
    for(long long int i = 0; i<N; i++){
        if(flag){
            vetor[i] = 4.0/(2*i+1);
        }
        else{
            vetor[i] = -4.0/(2*i+1);
        }
        flag = !flag;
    }
    
    //calculo sequencial do valor de pi
    GET_TIME(ini);
    for(long long int i = 0; i<N; i++){
        pi_seq += vetor[i];
    }
    GET_TIME(fim);
    printf("Tempo sequencial: %lf\n", fim-ini);
    
    //calculo concorrente do valor de pi
    GET_TIME(ini);
    tid = (pthread_t *) malloc(sizeof(pthread_t) * T);
    if(tid == NULL){
        fprintf(stderr, "ERRO--malloc\n");
        return 2;
    }
    //criar as threads
    for(int i = 0; i<T; i++){
        if(pthread_create(tid+i, NULL, tarefa, (void*) i)){
            fprintf(stderr, "ERRO--pthread_create\n");
            return 3;
        }
    }
    //aguardar o termino das threads
    for(int i = 0; i<T; i++){
        if(pthread_join(*(tid+i), (void**) &retorno)){
            fprintf(stderr, "ERRO--pthread_create\n");
            return 3;
        }
        //soma global
        pi_conc += *retorno;
        free(retorno);
    }
    GET_TIME(fim);
    printf("Tempo concorrente: %lf\n", fim-ini);
    
    //exibir os resultados
    printf("Valor aproximado de pi calculado sequencialmente: %.15lf\n", pi_seq);
    printf("Diferença em módulo entre o valor calculado sequencialmente e a constante M_PI: %.15lf\n", fabs(pi_seq-M_PI));
    printf("Valor aproximado de pi calculado concorrentemente: %.15lf\n", pi_conc);
    printf("Diferença em módulo entre o valor calculado concorrentemente e a constante M_PI: %.15lf\n", fabs(pi_conc-M_PI));
    
    //libera as areas de memoria alocadas
    free(vetor);
    free(tid);
    
    return 0;
}

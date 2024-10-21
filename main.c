#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>


#define MAX_SALAS 10 // número máximo de salas definidos na especificação do problema
#define MAX_THREADS 30 // número máximo de threads definidos na especificação do problema (3 x MAX_SALAS)


/******************************************************
Tipo ThreadData: guarda dados relevantes sobre as threads
********************************************************/
typedef struct {
    int id; // identificador da thread
    int init_time;  // quanto tempo deve passar até que thread possa entrar em sala
    int num_salas; // número de salas em que a thread passa
    int salas[MAX_SALAS]; // ids ordenados das salas em que a thread passa
    int tempos[MAX_SALAS]; // tempos ordenados dos tempos que a thread deve passar em cada sala
} ThreadData;


/******************************************************
Tipo SalaData: guarda dados relevantes sobre as salas
********************************************************/
typedef struct {
    int ocupantes; // número de ocupantes da sala
    int fila; // fila de threads que querem entrar na sala
    pthread_mutex_t lock; // tranca de exclusão mútua
    pthread_cond_t cond; // condição para exclusão mútua
} Sala;


/******************************************************
Vetor de salas como variável global para acesso por todas
as funções. Identificador da sala é seu índice no vetor - 1
porque são numeradas de 1 a MAX_SALAS
********************************************************/
Sala salas[MAX_SALAS];


/******************************************************
Mutex para controle de concorrência
********************************************************/
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock; 


/*********************************************************
 Inclua o código a seguir no seu programa, sem alterações.
 Dessa forma a saída automaticamente estará no formato esperado 
 pelo sistema de correção automática.
 *********************************************************/

void passa_tempo(int tid, int sala, int decimos)
{
    struct timespec zzz, agora;
    static struct timespec inicio = {0,0};
    int tstamp;

    if ((inicio.tv_sec == 0)&&(inicio.tv_nsec == 0)) {
        clock_gettime(CLOCK_REALTIME,&inicio);
    }

    zzz.tv_sec  = decimos/10;
    zzz.tv_nsec = (decimos%10) * 100L * 1000000L;

    if (sala == 0) {
        nanosleep(&zzz,NULL);
        return;
    }

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d [ %2d @%2d z%4d\n",tstamp, tid, sala, decimos);

    nanosleep(&zzz, NULL);

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d ) %2d @%2d\n",tstamp, tid, sala);
}
/*********************** FIM DA FUNÇÃO *************************/


/******************************************************
entra_sala: dada uma thread com id t_id, e uma sala com 
id sala_id, quando a sala estiver vazia e houverem 3
threads querendo entrar, entre
********************************************************/
void entra_sala(int t_id, int sala_id) {
    Sala* sala = &salas[sala_id - 1]; // escolhe a sala no vetor
    pthread_mutex_lock(&sala->lock); // tranca a sala

    sala->fila++; // entra na fila para entrar na sala
    
    // espera até a sala estar vazia e ter pelo menos 3 threads querendo entrar
    while (sala->ocupantes >= 0 || sala->fila < 3) pthread_cond_wait(&sala->cond, &sala->lock);

    sala->ocupantes++; // entra na sala
    sala->fila--; // sai da fila

    pthread_mutex_unlock(&sala->lock); // destranca a sala
}


/******************************************************
sai_sala: dada uma thread com id t_id, e uma sala com 
id sala_id, sai da sala e, se for o último a sair, avisa
aos demais
********************************************************/
void sai_sala(int tid, int sala_id) {
    Sala* sala = &salas[sala_id - 1]; // escolhe a sala no vetor
    pthread_mutex_lock(&sala->lock); // tranca a sala

    sala->ocupantes--; // sai da sala

    // se for o último a sair, conta aos demais que a sala está vazia
    if (sala->ocupantes == 0) pthread_cond_broadcast(&sala->cond);

    pthread_mutex_unlock(&sala->lock); // destranca a sala
}


/******************************************************
thread_function: relacionada à biblioteca phtreads.
Passa o tempo inicial antes de começar a movimentar thread
entre salas, e depois faz o caminho entre as salas
********************************************************/
void* thread_function(void* arg) {
    ThreadData* data = (ThreadData*)arg; // pega thread
    passa_tempo(data->id, 0, data->init_time); // passa o tempo inicial

    // para toda sala no caminho da thread, entra, passa tempo e sai, em ordem
    for (int i = 0; i < data->num_salas; i++) {
        int sala_id = data->salas[i];
        int tempo = data->tempos[i];

        entra_sala(data->id, sala_id);
        passa_tempo(data->id, sala_id, tempo);
        sai_sala(data->id, sala_id);
    }

    return NULL;
}

int main() {
    int num_salas, num_threads;

    // lê o número de salas e threads
    scanf("%d %d", &num_salas, &num_threads);

    // inicializa as salas
    for (int i = 0; i < num_salas; i++) {
        salas[i].ocupantes = 0;
        salas[i].fila = 0;
        pthread_mutex_init(&salas[i].lock, NULL);
        pthread_cond_init(&salas[i].cond, NULL);
    }

    // gera vetor de threads
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];

    // para cada thread
    for (int t = 0; t < num_threads; t++) {
        // salva o id, tempo transcorrido até começar a andar 
        // e número de salas do caminho
        int id, init_time, num_salas_thread;
        scanf("%d %d %d", &id, &init_time, &num_salas_thread); 

        thread_data[t].id = id;
        thread_data[t].init_time = init_time;
        thread_data[t].num_salas = num_salas_thread;

        // para cada sala no caminho da thread
        for (int s = 0; s < num_salas_thread; s++) {
            // salva o id da sala e o tempo passado nela
            int sala_id, tempo;
            scanf("%d %d", &sala_id, &tempo);
            thread_data[t].salas[s] = sala_id;
            thread_data[t].tempos[s] = tempo;
        }

        // inicializa pthreads para thread com dados salvos anteriormente
        pthread_create(&threads[t], NULL, thread_function, &thread_data[t]);
    }

    // espera todas as threads terminarem
    for (int t = 0; t < num_threads; t++) pthread_join(threads[t], NULL);

    // destrói mutex e variáveis de condição
    for (int i = 0; i < num_salas; i++) {
        pthread_mutex_destroy(&salas[i].lock);
        pthread_cond_destroy(&salas[i].cond);
    }

    return 0;
}

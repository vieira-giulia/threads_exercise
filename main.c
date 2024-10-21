#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#define MAX_SALAS 10                 // Número máximo de salas definidos na especificação do problema
#define MAX_THREADS 30               // Número máximo de threads (3 x MAX_SALAS)


/******************************************************
Tipo ThreadInfo: guarda dados relevantes sobre as threads
********************************************************/
typedef struct {
    int id;                         // identificador da thread
    int t_espera;                   // quanto tempo deve passar até que thread possa entrar em sala
    int n_salas;                    // número de salas em que a thread passa
    int *salas;                     // ids ordenados das salas em que a thread passa
    int *tempos;                    // tempos ordenados dos tempos que a thread deve passar em cada sala
} ThreadInfo;


/******************************************************
Tipo SalaInfo: guarda dados relevantes sobre as salas
********************************************************/
typedef struct {
    pthread_mutex_t mutex;         // Mutex para acesso à sala
    pthread_cond_t cond_espera;    // Variável de condição para fila de espera
    pthread_cond_t cond_trio;      // Variável de condição para formar trio
    int ocupacao;                  // Número de threads atualmente na sala
    int esperando;                 // Número de threads esperando para entrar
} SalaInfo;

SalaInfo salas[MAX_SALAS];         // Array de estruturas para as salas


/*********************************************************
 Inclua o código a seguir no seu programa, sem alterações.
 Dessa forma a saída automaticamente estará no formato esperado 
 pelo sistema de correção automática.
 *********************************************************/
void passa_tempo(int tid, int sala, int decimos) {
    struct timespec zzz, agora;
    static struct timespec inicio = {0, 0};
    int tstamp;

    if ((inicio.tv_sec == 0) && (inicio.tv_nsec == 0)) {
        clock_gettime(CLOCK_REALTIME, &inicio);
    }

    zzz.tv_sec = decimos / 10;
    zzz.tv_nsec = (decimos % 10) * 100L * 1000000L;

    if (sala == 0) {
        nanosleep(&zzz, NULL);
        return;
    }

    clock_gettime(CLOCK_REALTIME, &agora);
    tstamp = (10 * agora.tv_sec + agora.tv_nsec / 100000000L)
             - (10 * inicio.tv_sec + inicio.tv_nsec / 100000000L);

    printf("%3d [ %2d @%2d z%4d\n", tstamp, tid, sala, decimos);

    nanosleep(&zzz, NULL);

    clock_gettime(CLOCK_REALTIME, &agora);
    tstamp = (10 * agora.tv_sec + agora.tv_nsec / 100000000L)
             - (10 * inicio.tv_sec + inicio.tv_nsec / 100000000L);

    printf("%3d ) %2d @%2d\n", tstamp, tid, sala);
}
/*********************** FIM DA FUNÇÃO *************************/


/******************************************************
entra: entra na sala com sincronização de trio
********************************************************/
void entra(int tid, int sala) {
    pthread_mutex_lock(&salas[sala - 1].mutex);

    // Entra na fila para entrar na sala
    salas[sala - 1].esperando++;

    // A thread só entra quando houver pelo menos 3 threads esperando e a sala estiver vazia
    while (salas[sala - 1].esperando < 3 || salas[sala - 1].ocupacao > 0) 
        pthread_cond_wait(&salas[sala - 1].cond_espera, &salas[sala - 1].mutex);

    // Entra na sala
    salas[sala - 1].ocupacao++;
    // Sai da fila
    salas[sala - 1].esperando--;

    // Notifica entrada
    if (salas[sala - 1].ocupacao == 3) pthread_cond_broadcast(&salas[sala - 1].cond_trio);

    pthread_mutex_unlock(&salas[sala - 1].mutex);
}



/******************************************************
sai: sai da sala
********************************************************/
void sai(int tid, int sala) {
    pthread_mutex_lock(&salas[sala - 1].mutex);

    // Sai da sala
    salas[sala - 1].ocupacao--;

    // Se o trio inteiro saiu, a sala é liberada para o próximo trio
    if (salas[sala - 1].ocupacao == 0) pthread_cond_broadcast(&salas[sala - 1].cond_espera);

    pthread_mutex_unlock(&salas[sala - 1].mutex);
}



/******************************************************
thread_func: relacionada a biblioteca pthreads.
Generencia o movimento das threads
********************************************************/
void *thread_func(void *arg) {
    // Para uma thread específica
    ThreadInfo *info = (ThreadInfo *)arg;

    // Espera o tempo inicial antes de começar o trajeto
    passa_tempo(info->id, 0, info->t_espera);

    // Move-se pelas salas no trajeto da thread
    for (int i = 0; i < info->n_salas; i++) {
        int sala = info->salas[i];
        int tempo = info->tempos[i];

        // Entra na sala
        entra(info->id, sala);   
        // Passa tempo na sala
        passa_tempo(info->id, sala, tempo);   
        // Sai da sala  
        sai(info->id, sala);                    
    }

    pthread_exit(NULL);
}


int main() {
    int S, T;
    scanf("%d %d", &S, &T);

    pthread_t threads[MAX_THREADS];
    ThreadInfo info[MAX_THREADS];

    // Cria salas
    for (int i = 0; i < S; i++) {
        pthread_mutex_init(&salas[i].mutex, NULL);
        pthread_cond_init(&salas[i].cond_espera, NULL);
        pthread_cond_init(&salas[i].cond_trio, NULL);
        salas[i].ocupacao = 0;
        salas[i].esperando = 0;
    }

    // Cria threads
    for (int i = 0; i < T; i++) {
        scanf("%d %d %d", &info[i].id, &info[i].t_espera, &info[i].n_salas);

        // Alocação dinâmica dos vetores para salas e tempos
        info[i].salas = (int *)malloc(info[i].n_salas * sizeof(int));
        info[i].tempos = (int *)malloc(info[i].n_salas * sizeof(int));

        // Salva lista de salas e tempos para a thread
        for (int j = 0; j < info[i].n_salas; j++) scanf("%d %d", &info[i].salas[j], &info[i].tempos[j]);

        // Chama pthreads para essa thread
        pthread_create(&threads[i], NULL, thread_func, (void *)&info[i]);
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < T; i++) pthread_join(threads[i], NULL);

    // Libera recursos e destrói mutexes e variáveis de condição
    for (int i = 0; i < T; i++) {
        free(info[i].salas);
        free(info[i].tempos);
    }

    for (int i = 0; i < S; i++) {
        pthread_mutex_destroy(&salas[i].mutex);
        pthread_cond_destroy(&salas[i].cond_espera);
        pthread_cond_destroy(&salas[i].cond_trio);
    }

    return 0;
}

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


#define MAX_SALAS 10                 // Número máximo de salas definidos na especificação do problema
#define MAX_THREADS 30               // Número máximo de threads (3 x MAX_SALAS)
#define GROUP_SIZE 3                 // Número de threads necessárias para entrar na sala


/***************************************************************
Tipo ThreadInfo: guarda dados relevantes sobre as threads
****************************************************************/
typedef struct {
    int id;                         // identificador da thread
    int t_espera;                   // quanto tempo deve passar até que thread possa entrar em sala
    int n_salas;                    // número de salas em que a thread passa
    int *salas;                     // ids ordenados das salas em que a thread passa
    int *tempos;                    // tempos ordenados dos tempos que a thread deve passar em cada sala
} ThreadInfo;


/***************************************************************
Tipo SalaInfo: guarda dados relevantes sobre as salas
****************************************************************/
typedef struct {
    pthread_mutex_t mutex;          // Mutex para acesso à sala
    pthread_cond_t cond_espera;     // Variável de condição para fila de espera
    pthread_cond_t cond_group;      // Variável de condição para trio entrar junto
    pthread_barrier_t porta;        // Barreira para entrar na sala
    int ocupacao;                   // Número de threads atualmente na sala
    int esperando;                  // Número de threads esperando para entrar
   
} SalaInfo;

SalaInfo salas[MAX_SALAS];          // Array de estruturas para as salas


/****************************************************************
 Inclua o código a seguir no seu programa, sem alterações.
 Dessa forma a saída automaticamente estará no formato esperado 
 pelo sistema de correção automática.
 ****************************************************************/
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
/*********************** FIM DA FUNÇÃO ***************************/


/*****************************************************************
entra: entra na sala vazia com sincronização de trio
-- Uso de barrier para permitir que threads entrem apenas em trio
******************************************************************/
void entra(int tid, int sala, int tempo) {
    pthread_mutex_lock(&salas[sala - 1].mutex);

    // Entra na fila para entrar na sala
    salas[sala - 1].esperando++;

    // Wait until the room is empty and there are at least 3 threads waiting
    //while (salas[sala - 1].ocupacao > 0 || salas[sala - 1].esperando < GROUP_SIZE)
    while (salas[sala - 1].ocupacao > 0) pthread_cond_wait(&salas[sala - 1].cond_espera, &salas[sala - 1].mutex);

    // Espera na barreira até 3 estarem juntos para entrar
    pthread_barrier_wait(&salas[sala - 1].porta);

    // Sala ocupada por 3 threads
    salas[sala - 1].ocupacao += GROUP_SIZE;
    
    // 3 threads sairam da fila para entrar na sala
    salas[sala - 1].esperando -= GROUP_SIZE;

    pthread_mutex_unlock(&salas[sala - 1].mutex);
    
    // // Thread espera sala ficar vazia
    // while (salas[sala - 1].ocupacao > 0) pthread_cond_wait(&salas[sala - 1].cond_espera, &salas[sala - 1].mutex);

    // // Se houverem pelo menos 3 threads querendo entrar na sala, avisa as outras para entrar e entra
    // if (salas[sala - 1].esperando >= GROUP_SIZE){
    //     pthread_cond_broadcast(&salas[sala - 1].cond_group);
    //     salas[sala - 1].ocupacao++;
    //     salas[sala - 1].esperando--;   
    // } 
    // // Se não, espera pelo aviso
    // else pthread_cond_wait(&salas[sala - 1].cond_group, &salas[sala - 1].mutex);

    // pthread_mutex_unlock(&salas[sala - 1].mutex);
}


/**************************************************************
sai: sai da sala e sinaliza sala vazia
***************************************************************/
void sai(int tid, int sala) {
    pthread_mutex_lock(&salas[sala - 1].mutex);

    // Sai da sala
    salas[sala - 1].ocupacao--;

    // Se o trio inteiro saiu, a sala é liberada para o próximo trio
    if (salas[sala - 1].ocupacao == 0) pthread_cond_broadcast(&salas[sala - 1].cond_espera);

    pthread_mutex_unlock(&salas[sala - 1].mutex);
}


/**************************************************************
thread_func: relacionada a biblioteca pthreads.
Generencia o movimento das threads: passa o tempo inicial,
para cada sala no trajeto, entra assim que possível, passa
o tempo estipulado e sai. 
-- Uso de scheduler para permitir que outras threads se movimentem
se a atual estiver travada.
***************************************************************/
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
        entra(info->id, sala, tempo);
        // Passa tempo na sala
        passa_tempo(info->id, sala, tempo);   
        // Sai da sala  
        sai(info->id, sala);
        // Permitir que outras threads executem enquanto esta está parada
        sched_yield();
    }

    pthread_exit(NULL);
}


int main() {
    int S, T;
    scanf("%d %d", &S, &T);

    pthread_t threads[MAX_THREADS];
    ThreadInfo info[MAX_THREADS];

    pthread_barrierattr_t attr;
    pthread_barrierattr_init(&attr);

    // Inicializa sala
    for (int i = 0; i < S; i++) {
        pthread_mutex_init(&salas[i].mutex, NULL);
        pthread_cond_init(&salas[i].cond_espera, NULL);
        pthread_barrier_init(&salas[i].porta, &attr, GROUP_SIZE);
        salas[i].ocupacao = 0;
        salas[i].esperando = 0;
    }

    // Inicializa threads
    for (int i = 0; i < T; i++) {
        // Salva tempo de espera inicial e número de salas para essa thread percorrer
        scanf("%d %d %d", &info[i].id, &info[i].t_espera, &info[i].n_salas);
        // Salva lista de salas e tempos para a thread
        info[i].salas = (int *)malloc(info[i].n_salas * sizeof(int));
        info[i].tempos = (int *)malloc(info[i].n_salas * sizeof(int));
        for (int j = 0; j < info[i].n_salas; j++) scanf("%d %d", &info[i].salas[j], &info[i].tempos[j]);

        // Inicializa thread em pthreads com as informações salvas em formato ThreadInfd
        pthread_create(&threads[i], NULL, thread_func, (void *)&info[i]);
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < T; i++) pthread_join(threads[i], NULL);

    // Libera recursos alocados dinamicamente
    for (int i = 0; i < T; i++) {
        free(info[i].salas);
        free(info[i].tempos);
    }

    // Destrói mutexes e variáveis de condição
    pthread_barrierattr_destroy(&attr);
    for (int i = 0; i < S; i++) {
        pthread_mutex_destroy(&salas[i].mutex);
        pthread_cond_destroy(&salas[i].cond_espera);
        pthread_barrier_destroy(&salas[i].porta);
    }

    return 0;
}

#include<stdio.h>
#include <time.h>

#include "threads.h"
#include "rooms.h"


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

    if (sala==0) {
        nanosleep(&zzz,NULL);
        return;
    }

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d [ %2d @%2d z%4d\n",tstamp,tid,sala,decimos);

    nanosleep(&zzz,NULL);

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d ) %2d @%2d\n",tstamp,tid,sala);
}
/*********************** FIM DA FUNÇÃO *************************/


void get_in(Thread* thread, Room* room) {
    int t_id = thread->id;
    int step = thread->curr_room_index;
    int r_id = thread->room_ids[step];
    int tr_time = thread->room_times[step];

    // Enter the queue to get in room
    room->size_queue++;

    // Wait until: there is both space for three threads in the room
    // and three threads want to get in the room
    while (room->n_occupants != 0 && room->size_queue < 3);
    
    // Enter the room
    room->n_occupants++;
    room->size_queue--;

    // Now the thread can spend time in the room
    passa_tempo(t_id, r_id, tr_time);
}


void get_out(Thread* thread, Room *room) {
    // Point towards next room
    thread->curr_room_index++;

    // Open space in room
    room->n_occupants--;
}


void main() {
    // Read the total number of rooms and threads from stdin
    int n_rooms, n_threads;
    scanf("%d %d", &n_rooms, &n_threads);

    // Generate rooms vector
    Room* rooms = (Room*)calloc(n_rooms, sizeof(Room));
    for (int i = 0; i < n_rooms; i++) rooms[i] = new_room(i);

    // Generate threads vector
    Thread* threads = (Thread*)malloc(n_threads * sizeof(Thread));
    for (int t = 0; t < n_threads; t++) {
        int t_id, t_i_time, n_rooms;
        scanf("%d %d %d", &t_id, &t_i_time, &n_rooms);

        // Initialize thread 
        threads[t] = new_thread(t_id, t_i_time, n_rooms);

        // Add rooms to thread
        for (int r = 0; r < n_rooms; r++) {
            int r_id, r_time;
            scanf("%d %d", &r_id, &r_time); // Read room IDs and times
            threads[t].room_ids[r] = r_id;
            threads[t].room_times[r] = r_time;
        }
    }

    for (int t = 0; t < n_threads; t++) {
        Thread* thread = &threads[t];
        int* step = &thread->curr_room_index;
        if (*step == -1) {
            passa_tempo(thread->id, 0, thread->init_time);
            (*step)++;
        }
        for (int r = 0; r < thread->n_rooms; r++) {
            get_in(thread, &rooms[*step]);
            get_out(thread, &rooms[*step]);
        }
    }

    // Free threads
    for (int i = 0; i < n_threads; i++) free_thread(&threads[i]);
    free(threads);

    // Free rooms
    for (int i = 0; i < n_rooms; i++) free(&rooms[i]);
    free(rooms);
}
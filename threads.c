#include<stdio.h>

#include "threads.h"


Thread new_thread(int t_id, int t_i_time, int n_rooms){
    Thread thread;
    thread.id = t_id;
    thread.init_time = t_i_time;
    thread.curr_room_index = -1;
    thread.room_ids = (int*)calloc(n_rooms, sizeof(int));
    thread.room_times = (int*)calloc(n_rooms, sizeof(int));
    thread.busy = 1;
    return thread;
}

void free_thread(Thread* thread){
    free(thread->room_ids);
    free(thread->room_times);
    free(thread);
}

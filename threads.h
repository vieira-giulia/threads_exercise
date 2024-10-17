#ifndef THREADS_H
#define THREADS_H


typedef struct Thread{
    int id;
    int init_time;
    int n_rooms;
    int* room_ids;
    int* room_times;
    int curr_room_index;
    int busy;
}Thread;

Thread new_thread(int t_ID, int t_i_time, int n_rooms);

void free_thread(Thread* thread);

#endif
#ifndef ROOMS_H
#define ROOM_H


#define ROOM_CAPACITY 3


typedef struct Room{
    int id;
    int n_occupants;
    int occupied_by[ROOM_CAPACITY];
    int size_queue;
} Room;

Room new_room(int r_id);

#endif
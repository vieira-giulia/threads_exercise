#include"rooms.h"

Room new_room(int r_id){
    Room room;
    room.id = r_id;
    room.n_occupants = 0;
    return room;
}
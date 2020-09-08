#ifndef LINKEDLISTS_H
#define LINKEDLISTS_H

#include "extern.h"

ListOrder add_order(ListOrder list, ListOrder new_order);
ListOrder remove_order(ListOrder list, int id);
ListOrder find_order(ListOrder list, int id);
void print_pending_orders(ListOrder list);
ListOrder destroy_order_list(ListOrder list);


ListDrone add_drone(ListDrone list, ListDrone new_drone);
ListDrone remove_drone(ListDrone list, int id);
void print_drones(ListDrone list);
ListDrone find_drone(ListDrone list, int id);
ListDrone destroy_drone_list(ListDrone list);
#endif /* LINKEDLISTS_H */
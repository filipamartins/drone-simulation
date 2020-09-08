#ifndef CENTRAL_H
#define CENTRAL_H

#include "extern.h"

int closest_base(int id_drone, ListDrone list_drone);
void send_to_warehouse(int id_drone, int w_no, Order order, ListDrone list_drone);
void send_to_destination(int id_drone, ListDrone list_drone);
void send_to_base(int id_drone, ListDrone list_drone);
void round_robin_distribution(int start,int num, ListDrone list_drone);
void update_num_drones(int new_num_drones, ListDrone list_drone);
int ind_product_warehouse(char *prod, int w_no);
int product_type_exists(char *prod);
void calculate_shortest_distance(char *prod, int quant, int dest_x, int dest_y, int *id_drone, int *w_no, int *product_index, ListDrone list_drone);
void update_warehouse_stock(int w_no, char *prod, int quantity);
void update_num_total_orders();
void udate_num_order_delivered();
void update_num_products_loaded(int quantity);
void update_num_products_delivered(int quantity);
void update_mean_time_order(int time);

int read_order(char order_message[], Order* order, int order_no);
void central_process();
void *drone_function(void* id_drone);

#endif
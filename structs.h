#ifndef STRUCTS_H
#define STRUCTS_H

/*Structs*/

typedef struct{
	double x;
	double y;
}Point;

typedef struct{
	char type[30];
	int quant;
}Product;

typedef struct order{
	int id;
	int w_no;
	Product product;
	Point destination;
}Order;

typedef struct{
	int id;
	int free;
	int behaviour; //1-Standby 2- Move to warehouse 3- Loading 4- Move to delivery destination 5- Return to base
	Point local;
	Point target;
	Order order;
}Drone;

typedef struct no_order *ListOrder;
typedef struct no_drone *ListDrone;

typedef struct no_order{
	Order info;
	ListOrder next;
}No_order;

typedef struct no_drone{
	Drone info;
	pthread_t my_thread;
	int thread_id;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	ListDrone next;
}No_drone;


typedef struct{
	int w_no;
	char name[30];
	Point local;
	Product products[4];
}Warehouse;


typedef struct{
	int num_total_order;
	int num_products_loaded;
	int num_order_delivered;
	int num_products_delivered;
	int mean_time_order;
}Stats;


typedef struct{
  /* Type of message */
  long type;
  /* Content of message */
  int id;
  char name[30];
  Product product;        
} Queue_Msg;


#endif /* STRUCTS_H */
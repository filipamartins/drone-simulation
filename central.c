#include "central.h"


int free_drone(ListDrone list_drone){
    ListDrone list_cursor = list_drone;
    while(list_cursor != NULL){ //for each drone 
        if(list_cursor->info.free)
            return 1;
        list_cursor = list_cursor->next;
    }
    return 0;
}

int closest_base(int id_drone, ListDrone list_drone){
    ListDrone drone;
    drone = find_drone(list_drone, id_drone);
    float dist_total_final=99999, dist_total = -1;
    int ind = -1;
    for(int i = 0;i<4;i++){
        dist_total = distance(drone_bases[i].x, drone_bases[i].y, drone->info.local.x, drone->info.local.y);
        if(dist_total < dist_total_final){
            dist_total_final = dist_total;
            ind = i;
        }
    }
    return ind;
}

void send_to_warehouse(int id_drone, int w_no, Order order, ListDrone list_drone){
    ListDrone drone;
    drone = find_drone(list_drone, id_drone);
    drone->info.target.x = (shm_war+w_no-1)->local.x;
    drone->info.target.y = (shm_war+w_no-1)->local.y;
    drone->info.free = 0;
    drone->info.behaviour = 2;
    order.w_no = w_no;
    drone->info.order = order;
}

void send_to_destination(int id_drone, ListDrone list_drone){
    ListDrone drone;
    drone = find_drone(list_drone, id_drone);
    drone->info.target.x = drone->info.order.destination.x;
    drone->info.target.y = drone->info.order.destination.y;
    drone->info.behaviour = 4;
}

void send_to_base(int id_drone, ListDrone list_drone){
    ListDrone drone;
    drone = find_drone(list_drone, id_drone);
    int ind = closest_base(id_drone, list_drone);
    if(ind != -1){
        drone->info.target.x = drone_bases[ind].x;
        drone->info.target.y = drone_bases[ind].x;
        drone->info.behaviour = 5;
        drone->info.free = 1;
    }
    else{
        printf("Error in finding base.");
    }
}

void round_robin_distribution(int start,int num, ListDrone list_drone){ 
    ListDrone drone;
    #ifdef DEBUG
    printf("\nDEBUGGING Round Robin...\n\n");
    #endif
    int id_base, bases = 4;
    for(int i=start; i<num; i++){
        drone = find_drone(list_drone, i);
        id_base = i % bases; //number of bases = 4  id_base: 0 1 2 3 0 1 2 3..
        drone->info.local.x = drone_bases[id_base].x;
        drone->info.local.y = drone_bases[id_base].y;
        #ifdef DEBUG
        printf("Drone %d local x %f local y %f\n", i, drone->info.local.x, drone->info.local.y);
        #endif
    }
    #ifdef DEBUG
    print_drones(list_drone);
    #endif
}


void update_num_drones(int new_num_drones, ListDrone list_drone){
    int i; ListDrone new_drone;
    
    for(i=num_drones; i<new_num_drones; i++){
        new_drone = (ListDrone)malloc(sizeof(No_drone));
        if(new_drone == NULL) {
            printf("Error! Memory not available.");
            exit(2); /*error code memory not available*/
        }
        new_drone->next = NULL;
        new_drone->thread_id = i;
        new_drone->info.id = i;
        new_drone->info.free = 1;
        new_drone->info.behaviour = 1;
        
        pthread_create(&new_drone->my_thread, NULL, drone_function, &new_drone->thread_id); //creates the thread and sends the drone id for which it is responsible
        fprintf(stderr, "[Central %d]: Drone Thread %d created.\n", getpid(), i);
        pthread_mutex_init(&new_drone->mutex, NULL);
        pthread_cond_init(&new_drone->cond, NULL);
        list_drone = add_drone(list_drone, new_drone);
    }

    round_robin_distribution(num_drones, new_num_drones, list_drone);
    //Update number of drones
    num_drones = new_num_drones;
    printf("Drone set to %d\n", num_drones);
}


//returns index if product is available in the warehouse  
int ind_product_warehouse(char *prod, int w_no){
    int ind = -1;
    for(int i=0; i<3; i++){     //for each product type in the warehouse                       
        if(strcmp((shm_war+w_no-1)->products[i].type, prod)==0){
            ind = i;
        }
    }
    return ind;
}

int product_type_exists(char *prod){
    for(int i=0; i<num_prod; i++){                                              
        if(strcmp(product_types[i].type, prod)==0){
            return 1;
        }
    }
    return -1;
}

void calculate_shortest_distance(char *prod, int quant, int dest_x, int dest_y, int *id_drone, int *w_no, int *product_index, ListDrone list_drone){
    float dist_total_final=9999;
    float dist_total=0;
    float dist_warehouse_destination=0;
    float dist_drone_warehouse = 0;
    float dist_drone_warehouse_final=9999;
    for(int i = 0; i<num_warehouses;i++){       //for each warehouse  

        for(int j = 0; j<3;j++){                //for each product type
            if(strcmp((shm_war+i)->products[j].type,prod)==0){

                if((shm_war+i)->products[j].quant >= quant){
                    dist_warehouse_destination = sqrt(pow((dest_x-(shm_war+i)->local.x),2) + pow((dest_y-(shm_war+i)->local.y),2));
                    ListDrone list_cursor = list_drone;
                    
                    while(list_cursor != NULL){     //for each drone 
                        if(list_cursor->info.free){
                            dist_drone_warehouse= sqrt(pow(((shm_war+i)->local.x-list_cursor->info.local.x),2) + pow(((shm_war+i)->local.y-list_cursor->info.local.y),2));
                            if(dist_drone_warehouse < dist_drone_warehouse_final){
                                dist_drone_warehouse_final = dist_drone_warehouse;
                                *id_drone = list_cursor->info.id;
                            }
                        }
                        list_cursor = list_cursor->next;
                    }
                    dist_total = dist_drone_warehouse_final + dist_warehouse_destination;

                    if(dist_total_final > dist_total){
                        dist_total_final = dist_total;
                        *w_no = i+1;
                        *product_index = j;
                    }
                }
            }
        }
    }
}

void update_warehouse_stock(int w_no, char *prod, int quantity){
    int ind = ind_product_warehouse(prod, w_no);        //index of product in the warehouse
    (shm_war+w_no-1)->products[ind].quant += quantity;
}
void update_num_total_orders(){
    shm_stats->num_total_order++;
}
void udate_num_order_delivered(){
    shm_stats->num_order_delivered++;
}
void update_num_products_loaded(int quantity){
    shm_stats->num_products_loaded += quantity;
}
void update_num_products_delivered(int quantity){
    shm_stats->num_products_delivered += quantity;
}
void update_mean_time_order(int time){
    int total = shm_stats->num_order_delivered;
    int media = shm_stats->mean_time_order;
    int soma = media*total;
    shm_stats->mean_time_order = (soma+time)/total;
}

int read_order(char order_message[], Order* order, int order_no) {
    order->id = order_no;
    char key[20], req[20], prod_type[20], g[20], g1[20], g2[20];
    int prod_quant, x, y, new_num_drones=0;
    sscanf(order_message, "%s", key);
    time_t rawtime; time (&rawtime); 
    struct tm *tm_struct = localtime(&rawtime);
    if(strcmp(key, "ORDER") == 0){
        sscanf(order_message, "%s %s %s %s %d %s %d %s %d", key, req, g, prod_type, &prod_quant, g1, &x, g2, &y);
        if(strcmp(req, "Req_1") == 0){
            fprintf(stderr, "%s %s %s %s %d %s %d %s %d\n", key, req, g, prod_type, prod_quant, g1, x, g2, y);
            sem_wait(mutex_log_file);
            fprintf(fp2, "%d:%d:%d [Order %d] - %s received by Central\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, order_no, req);
            sem_post(mutex_log_file);
            fflush(fp2);
            if(x >= 0 && x <= coord_max.x && y >=0 && y <= coord_max.y){
                order->product.quant = prod_quant;
                order->destination.x = x;
                order->destination.y = y;

                if(strcmp(prod_type, "A,") == 0 && product_type_exists("Prod_A") != -1){
                    strcpy(order->product.type, "Prod_A");
                    return 1;
                }
                else if(strcmp(prod_type, "B,") == 0 && product_type_exists("Prod_B") != -1){
                    strcpy(order->product.type, "Prod_B");
                    return 1;  
                }
                else if(strcmp(prod_type, "C,") == 0 && product_type_exists("Prod_C") != -1){
                    strcpy(order->product.type, "Prod_C");
                    return 1;
                }
                else if(strcmp(prod_type, "D,") == 0 && product_type_exists("Prod_D") != -1){
                    strcpy(order->product.type, "Prod_D");
                    return 1;
                }          
                else {
                    fprintf(stderr, "Invalid Order. Please insert new order.\n");
                    time (&rawtime); 
                    tm_struct = localtime(&rawtime);
                    sem_wait(mutex_log_file);
                    fprintf(fp2, "%d:%d:%d Command discarded - Product doesn't exist: %s\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, order_message);
                    sem_post(mutex_log_file);
                    fflush(fp2);
                }
            }
            else{
                fprintf(stderr, "Invalid Order. Please insert new order.\n");
                time (&rawtime); 
                tm_struct = localtime(&rawtime);
                sem_wait(mutex_log_file);
                fprintf(fp2, "%d:%d:%d Command discarded - Invalid coordinates: %s\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, order_message);
                sem_post(mutex_log_file);
                fflush(fp2);
            }
        }
        else{
            fprintf(stderr, "Invalid Order. Please insert new order.\n");
            time (&rawtime); 
            tm_struct = localtime(&rawtime);
            sem_wait(mutex_log_file);
            fprintf(fp2, "%d:%d:%d Command discarded: %s\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, order_message);
            sem_post(mutex_log_file);
            fflush(fp2);
        }
    }
    else if (strcmp(key, "DRONE") == 0){ //update global variable
        sscanf(order_message, "%s %s %d", key, g, &new_num_drones);
        if (strcmp(g, "SET") == 0){

            fprintf(stderr, "%s %s %d\n", key, g, new_num_drones);
            if(new_num_drones<=50 && new_num_drones>=1 && new_num_drones > num_drones){
                printf("Central received configuration change request to %d drones\n", new_num_drones);
                update_num_drones(new_num_drones, list_drone);
                time (&rawtime); 
                tm_struct = localtime(&rawtime);
                sem_wait(mutex_log_file);
                fprintf(fp2, "%d:%d:%d Change in configuration to %d drones\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, num_drones);
                sem_post(mutex_log_file);
                fflush(fp2);
                return 0;
            }
        }
        time (&rawtime); 
        tm_struct = localtime(&rawtime);
        fprintf(stderr, "Invalid Order. Please insert new order.\n");
        sem_wait(mutex_log_file);
        fprintf(fp2, "%d:%d:%d Command discarded: %s\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, order_message);
        sem_post(mutex_log_file);
    }
    else{
        fprintf(stderr, "Invalid Order. Please insert new order.\n");
        time (&rawtime); 
        tm_struct = localtime(&rawtime);
        sem_wait(mutex_log_file);
        fprintf(fp2, "%d:%d:%d Command discarded: %s\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, order_message);
        sem_post(mutex_log_file);
        fflush(fp2);
    }
    return 0;
}
    


void * read_pipe(void * list){
    char order_message[80];
    int order_no = 0, fd;
    Order order; 
    ListOrder new_order;
    ListOrder* list_head = (ListOrder*)list;

     // Create named pipe 
    if ((mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) {
        perror("Cannot create pipe: ");
        exit(0);
    }

    // Opens the pipe for reading
    if ((fd=open(PIPE_NAME, O_RDWR)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0);
    }

    while(!exit_requested){
        read(fd, order_message, sizeof(order_message)); //read from pipe
        order_no++;
        fprintf(stderr, "CENTRAL: Received message: %s\n", order_message);
        int valid_order = read_order(order_message, &order, order_no);
        if (valid_order){
            printf("Valid order!\n");
            new_order = (ListOrder)malloc(sizeof(No_order));
            if(new_order == NULL) {
                printf("Error! Memory not available.");
                exit(2); /*error code memory not available*/
            }
            new_order->next = NULL;
            new_order->info.id = order.id;
            new_order->info.w_no = order.w_no;
            new_order->info.product = order.product;
            new_order->info.destination = order.destination;
            *list_head = add_order(*list_head, new_order);
            #ifdef DEBUG
            print_pending_orders(*list_head);
            #endif
            sem_post(mutex_centr);
        }
        else{
            order_no--;
        } 
    }
    pthread_exit(NULL);
    return NULL;
}


void central_process(){
   
    int id_order = -1;
    int id_drone= -1, w_no = -1, product_index =-1;
  
    list_head = NULL;
    ListOrder list_order = list_head;
    list_drone = NULL;
    ListDrone new_drone;
    ListDrone drone;

    time_t rawtime; time (&rawtime); 
    struct tm *tm_struct = localtime(&rawtime); 
    

    //Create drone threads
	for(int i=0; i<num_drones; i++){

        new_drone = (ListDrone)malloc(sizeof(No_drone));
        if(new_drone == NULL) {
            printf("Error! Memory not available.");
            exit(2); /*error code memory not available*/
        }
        new_drone->next = NULL;
        new_drone->thread_id = i;
        new_drone->info.id = i;
        new_drone->info.free = 1;
        new_drone->info.behaviour = 1;
        
        pthread_create(&new_drone->my_thread, NULL, drone_function, &new_drone->thread_id); //creates the thread and send drone id for which it is responsible
        fprintf(stderr, "[Central %d]: Drone Thread %d created.\n", getpid(), i);
        pthread_mutex_init(&new_drone->mutex, NULL);
        pthread_cond_init(&new_drone->cond, NULL);
        list_drone = add_drone(list_drone, new_drone);
	}

    round_robin_distribution(0, num_drones, list_drone); //distribute drones over existing bases
    //create pipe_thread 
    pthread_create(&pipe_thread, NULL, read_pipe, &list_head); 

    while (1) {
        //printf("Before mutex order\n");
        sem_wait(mutex_centr);
        #ifdef DEBUG
        printf("After mutex order\n");
        printf("-----> Back to TOP\n");
        #endif
        
        list_order = list_head;
        
        while(list_order != NULL){
            #ifdef DEBUG
            printf("id order %d\n", list_order->info.id);
            #endif
            id_order = list_order->info.id; //first order in list
            while(free_drone(list_drone) == 0){ //if there is no drone free
                sleep(1);
            }
            id_drone= -1;
            w_no = -1;
            product_index = -1;
            calculate_shortest_distance(list_order->info.product.type, list_order->info.product.quant, list_order->info.destination.x, list_order->info.destination.y, &id_drone, &w_no, &product_index, list_drone);
            #ifdef DEBUG
            printf("w_no %d, id_drone %d, product_index %d\n", w_no, id_drone, product_index);
            #endif
            if(w_no != -1 && id_drone != -1 && product_index != -1){
                drone = find_drone(list_drone, id_drone);
                fprintf(stderr,"[Central %d]: Resolving order!!!\n", getpid());
                fprintf(stderr, "[Central %d]: Drone %d and Warehouse %d selected \n", getpid(), id_drone, w_no);
                time (&rawtime); 
                tm_struct = localtime(&rawtime);
                sem_wait(mutex_log_file);
                fprintf(fp2, "%d:%d:%d Order %d given to drone %d\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, list_order->info.id, id_drone);
                sem_post(mutex_log_file);
                fflush(fp2);
                sem_wait(mutex_war);
                sem_wait(mutex_stats);
                update_warehouse_stock(w_no, list_order->info.product.type, -(list_order->info.product.quant));
                update_num_total_orders();
                sem_post(mutex_stats);
                sem_post(mutex_war);
                send_to_warehouse(id_drone, w_no, list_order->info, list_drone);
                list_head = remove_order(list_head, id_order);
                #ifdef DEBUG
                printf("Print after removing\n");
                print_pending_orders(list_head);
                #endif
                id_order = -1;
                pthread_cond_signal(&drone->cond);   
            }
            else if (w_no == -1 || product_index == -1){
                fprintf(stderr, "[Central %d]: Cannot find product to fullfill order %d.\n", getpid(), list_order->info.id);
                time (&rawtime); 
                tm_struct = localtime(&rawtime);
                sem_wait(mutex_log_file);
                fprintf(fp2, "%d:%d:%d Order %d suspended due to lack of stock\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, list_order->info.id);
                sem_post(mutex_log_file);
                fflush(fp2);
            }
            if (list_order != NULL){
                list_order = list_order->next;
            }                 
        }
    }   
}

void *drone_function(void* id_drone){
    int fly_status; Queue_Msg msg;
    int my_id = *((int *)id_drone);
    int t_units=0;
    ListDrone drone;
    time_t rawtime; 
    time (&rawtime); 
    struct tm *tm_struct = localtime(&rawtime);
    
    sigset_t block_ctrlc;
    sigemptyset (&block_ctrlc);
    sigaddset (&block_ctrlc, SIGINT);
    drone = find_drone(list_drone, my_id);

    while(!exit_requested) {
        sigprocmask (SIG_BLOCK, &block_ctrlc, NULL);
        pthread_mutex_lock(&drone->mutex);
        while(drone->info.free == 1 && exit_requested==0){
            pthread_cond_wait(&drone->cond, &drone->mutex);
        }
        
        //MOVING TO WAREHOUSE
        if(drone->info.free == 0 && drone->info.behaviour == 2){
            #ifdef DEBUG
            fprintf(stderr, "Hello, I'm thread %d I am going to move my drone\n", my_id);
            #endif
            do{
                fly_status = move_towards(&drone->info.local.x, &drone->info.local.y, drone->info.target.x, drone->info.target.y);
                t_units++;
            }while(fly_status == 1);
            fprintf(stderr, "[Drone %d] Flyed to warehouse %d location x= %lf, y= %lf.\n", drone->info.id, drone->info.order.w_no, drone->info.local.x, drone->info.local.y);  
            drone->info.behaviour = 3;

            msg.type = drone->info.order.w_no;
            #ifdef DEBUG
            printf("Warehouse %d da ordem %d no drone %d\n", drone->info.order.w_no, drone->info.order.id, drone->info.id);
            #endif
            strcpy(msg.name,"ORDER");
            msg.id = drone->info.order.id;
            msg.product = drone->info.order.product;
            
            printf("\n[DRONE %d] Sended arrive message to warehouse %d through message queue.\n", my_id, drone->info.order.w_no);
            msgsnd(mqid, &msg, sizeof(Queue_Msg), 0); //send message to the warehouse through the message queue to say that it arrived
            msgrcv(mqid, &msg, sizeof(Queue_Msg), TYPE_DRONE, 0); //waits for warehouse message
            printf("\n[DRONE %d] Received notification that the warehouse has finished loading the shippment.\n", my_id);
            t_units += msg.product.quant;
            send_to_destination(my_id, list_drone);

        }
        //MOVING TO DESTINATION
        if(drone->info.free == 0 && drone->info.behaviour == 4){ 
            do{
                fly_status = move_towards(&drone->info.local.x, &drone->info.local.y, drone->info.target.x, drone->info.target.y);
                t_units++;
            }while(fly_status ==1);
            fprintf(stderr, "[Drone %d] Flyed to destination x= %lf, y= %lf.\n", my_id, drone->info.local.x, drone->info.local.y);
            t_units++; // plus one unit of time for delivery
            sem_wait(mutex_stats);
            udate_num_order_delivered();
            update_num_products_delivered(drone->info.order.product.quant);
            update_mean_time_order(t_units);
            sem_post(mutex_stats);
            kill(simulation_pid, SIGUSR1); //request printing of statistics
            send_to_base(my_id, list_drone);
            time (&rawtime); 
            tm_struct = localtime(&rawtime);
            sem_wait(mutex_log_file);
            fprintf(fp2, "%d:%d:%d Drone %d with order %d arrived at destination\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, my_id, drone->info.order.id);
            sem_post(mutex_log_file);
            fflush(fp2);
        }
        //MOVING TO BASE
        if(drone->info.free == 1 && drone->info.behaviour == 5){
            do{
                fly_status = move_towards(&drone->info.local.x, &drone->info.local.y, drone->info.target.x, drone->info.target.y);
            }while(fly_status ==1 && drone->info.free ==1);
        }
        pthread_mutex_unlock(&drone->mutex); 
        sigprocmask (SIG_UNBLOCK, &block_ctrlc, NULL);
    }
    fprintf(stderr, "[Central %d]: Drone Thread %d exited.\n", getpid(), my_id);
    pthread_exit(NULL);
    return NULL;
}
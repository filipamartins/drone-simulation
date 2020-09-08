
#define EXTERN
#include "extern.h"


void warehouse_process();


void print_stats(){
    printf("\n*STATS UPDATE*\n");
    printf("Total orders: %d\n", shm_stats->num_total_order);
    printf("Total products loaded: %d\n", shm_stats->num_products_loaded);
    printf("Total orders delivered: %d\n", shm_stats->num_order_delivered);
    printf("Total products delivered: %d\n", shm_stats->num_products_delivered);
    printf("Mean time order: %d\n\n", shm_stats->mean_time_order);
}



void terminate() {
    //SHARED MEMORY
    shmdt(shm_war); 
    shmdt(shm_stats);
    shmctl(shm_id,IPC_RMID,NULL);
    shmctl(shm_id_stats,IPC_RMID,NULL);
    //MESSAGE QUEUE
    msgctl(mqid, IPC_RMID, 0);
    //SEMAPHORES
    sem_close(mutex_war);
    sem_close(mutex_stats);
    sem_close(mutex_centr);
    sem_unlink("MUTEX");
    sem_unlink("MUTEX2");
    sem_unlink("MUTEX3");
    //mallocs
    free(product_types);
    //Linked lists
    list_drone = destroy_drone_list(list_drone);
    list_head = destroy_order_list(list_head);
}


void sigint(int signum) {
    pid_t son_pid;
    exit_requested = 1;

    time_t rawtime; 
    time (&rawtime); 
    struct tm *tm_struct = localtime(&rawtime);
    
    if (getpid() == simulation_pid) {
        fprintf(stderr, "[Simulation %d]: Received signal.\n", getpid());
        pthread_join(supply_thread, NULL);
        fprintf (stderr, "[Simulation %d]: Supply thread has joined.\n", getpid());
        while ((son_pid = wait(NULL)) > 0);
        fprintf(stderr, "[Simulation %d]: Releasing resources..\n", getpid()); //free shared memory
        terminate();
        fprintf(stderr, "[Simulation %d]: Exited.\n", getpid());
        time (&rawtime); 
        tm_struct = localtime(&rawtime);
        sem_wait(mutex_log_file);
        fprintf(fp2, "%d:%d:%d [Simulation %d]: Exited.\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, getpid());
        sem_post(mutex_log_file);
        fclose(fp2);
    } 
    else if (getpid() == central_pid){
        ListDrone list_cursor = list_drone;
        while(list_cursor != NULL){ //sends exit signal to each drone
            pthread_cond_signal(&list_cursor->cond);
            list_cursor = list_cursor->next;
        }
        fprintf(stderr, "[Central %d]: Received signal. Waiting for threads to join..\n", getpid());
        list_cursor = list_drone;
        while(list_cursor != NULL){ //destroys condition variables and mutex for each drone
            pthread_cond_destroy(&list_cursor->cond);
            pthread_mutex_destroy(&list_cursor->mutex);
            pthread_join(list_cursor->my_thread, NULL); //join thread_drones
            fprintf (stderr, "[Central %d]: Another thread has joined.\n", getpid()); //waits for them to die
            list_cursor = list_cursor->next;
        }
        pthread_cancel(pipe_thread);
        pthread_join(pipe_thread, NULL);
        fprintf (stderr, "[Central %d]: Another thread has joined.\n", getpid());
        fprintf(stderr, "[Central %d]: All thread have joined!\n", getpid());
        fprintf(stderr, "[Central %d]: Exited.\n", getpid());
    }
    else {
        fprintf(stderr, "[Warehouse %d]: Received signal.\n", getpid());
        fprintf(stderr, "[Warehouse %d]: Exited.\n", getpid());
        time (&rawtime); 
        tm_struct = localtime(&rawtime);
        sem_wait(mutex_log_file);
        fprintf(fp2, "%d:%d:%d [Warehouse %d]: Exited.\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, getpid());
        sem_post(mutex_log_file);
        fflush(fp2);
    }   
    exit(0);
}

int round_robin_warehouse(int w_no){
    if(w_no >= num_warehouses){
        w_no = w_no % num_warehouses;
        w_no +=1;
    }
    else{
        w_no = w_no +1 ;
    }
    return w_no;
}

void *supply(){
    char prod[30]; Queue_Msg msg_supply;
    int w_no=0, ind_prod;
    time_t rawtime; 
    time (&rawtime); 
    struct tm *tm_struct = localtime(&rawtime);
    //send supply messages
    while(exit_requested == 0){
        sleep(freq_abast);
        w_no = round_robin_warehouse(w_no);  //sequentially chosen warehouse (round robin)
        do{                                    
            ind_prod = rand() % num_prod;
            strcpy(prod, product_types[ind_prod].type);
        }while(ind_product_warehouse(prod, w_no) == -1); //while the product found is not in the warehouse 

        msg_supply.type = w_no;
        strcpy(msg_supply.name, "SUPPLY");
        strcpy(msg_supply.product.type, prod);
        msg_supply.product.quant = quant_abast;
        time (&rawtime); 
        tm_struct = localtime(&rawtime);
        sem_wait(mutex_log_file);
        fprintf(fp2, "%d:%d:%d Warehouse %d received new stock\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, w_no);
        sem_post(mutex_log_file);
        fflush(fp2);
        msgsnd(mqid, &msg_supply, sizeof(Queue_Msg), 0);  
    }
    pthread_exit(NULL);
    return NULL;
}


int main()
{
    int i, fd;
    pid_t pid;
    char order[80];
    Warehouse * warehouses;
    exit_requested = 0; num_prod=1;
    
    time_t rawtime; 
    time (&rawtime); 
    struct tm *tm_struct = localtime(&rawtime);
    //Open config file
    warehouses = open_config_file();


    #ifdef DEBUG
    printf("\nDEBUGGING...\n\n");
    printf("Coord_max:  x- %lf  y- %lf\n",coord_max.x, coord_max.y);
    printf("Products: %s %s %s %s\n", product_types[0].type,product_types[1].type, product_types[2].type, product_types[3].type);
    printf("Num drones %d", num_drones);
    printf(" Freq abastecimento %d, quant_abast %d time_unit %d num_warehouses %d\n",freq_abast,quant_abast,time_unit, num_warehouses);
    
    for (i = 0; i <num_warehouses; i++){
        printf("Warehouse %d name coord x  %s %lf %lf\n", i, warehouses[i].name, warehouses[i].local.x, warehouses[i].local.y);
        printf("Warehouse product 0 quant  %d e nome: %s\n", warehouses[i].products[0].quant, warehouses[i].products[0].type);
        printf("Warehouse product 1 quant  %d e nome: %s\n", warehouses[i].products[1].quant, warehouses[i].products[1].type);
        printf("Warehouse product 2 quant  %d e nome: %s\n", warehouses[i].products[2].quant, warehouses[i].products[2].type);
    }
    printf("Num Prod %d\n\n", num_prod);
    #endif

    drone_bases[0].x = 0;
    drone_bases[0].y = 0;
    drone_bases[1].x = coord_max.x;
    drone_bases[1].y = 0;
    drone_bases[2].x = 0;
    drone_bases[2].y = coord_max.y;
    drone_bases[3].x = coord_max.x;
    drone_bases[3].y = coord_max.y;

    fp2 = fopen("log.txt", "w");
    simulation_pid = getpid();
    
    //print to Log File
    fprintf(stderr, "[Simulation %d]: Running.\n\n", simulation_pid);
    fprintf(fp2, "%d:%d:%d [Simulation %d]: Running.\n\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, simulation_pid);
    fflush(fp2);

    //CTRL-C termination of processes and threads
    signal(SIGINT, sigint);
    //Show statistics at central request
    signal(SIGUSR1, print_stats);

    /* Initialize semaphores to access shared memory warehouses and stats*/
    sem_unlink("MUTEX");
    mutex_war = sem_open("MUTEX", O_CREAT|O_EXCL, 0700, 1);
    sem_unlink("MUTEX2");
    mutex_stats =sem_open("MUTEX2", O_CREAT|O_EXCL, 0700, 1);
    //create mutex for comunication between warehouse and central
    sem_unlink("MUTEX3");
    mutex_centr = sem_open("MUTEX3", O_CREAT|O_EXCL, 0700, 0);
    //mutex to write in log file
    sem_unlink("MUTEX4");
    mutex_log_file = sem_open("MUTEX4", O_CREAT|O_EXCL, 0700, 1);


    /*Create message queue*/
    mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777);
    if (mqid < 0){
        perror("msget error\n");
        exit(0);
    }

    /* Create shared memory warehouses */
    shm_id = shmget(IPC_PRIVATE, num_warehouses*sizeof(Warehouse), IPC_CREAT | 0777); 
    if (shm_id == -1){
        printf("shmget error\n");
        exit(1);
    }

    /* Create shared memory Stats */
    shm_id_stats = shmget(IPC_PRIVATE, 1*sizeof(Stats), IPC_CREAT | 0777);
    if (shm_id_stats == -1){
        printf("shmget error\n");
        exit(1);
    }

    /* Attach shared memory */
    shm_war = (Warehouse *) shmat(shm_id, NULL, 0);
    if (shm_war == NULL) {
        printf("shmat error (server)\n");
        exit(1);
    }

    shm_stats = (Stats *) shmat(shm_id_stats, NULL, 0);
    if (shm_stats == NULL) {
        printf("shmat error (server)\n");
        exit(1);
    }

    fprintf(stderr, "[Simulation %d]: Shared memory (ID:%d) attached...\n\n", getpid(), shm_id);
    fprintf(stderr, "[Simulation %d]: Shared memory (ID:%d) attached...\n\n", getpid(), shm_id_stats);

    //initialize values
    memcpy(shm_war, warehouses, num_warehouses*sizeof(Warehouse));
    free(warehouses); //free the malloc

    shm_stats->num_total_order = 0;
    shm_stats->num_products_loaded = 0;
    shm_stats->num_order_delivered = 0;
    shm_stats->num_products_delivered = 0;
    shm_stats->mean_time_order = 0;

    //create central process
    pid = fork();
    if (pid < 0) { 
        printf("fork error"); 
        exit(1); 
    }
    else if (pid == 0) {
        central_pid = getpid();
        fprintf(stderr, "[Simulation %d]: Central Process %d created.\n", getppid(), central_pid);
        central_process();
        exit(0);
    }

    //create warehouse processes
    for(i=0;i<num_warehouses;i++){
        pid = fork();
        if (pid < 0) { 
            printf("fork error"); 
            exit(1); 
        }
        else if (pid == 0) {
            time (&rawtime); 
            tm_struct = localtime(&rawtime);
            sem_wait(mutex_log_file);
            fprintf(fp2, "%d:%d:%d Warehouse %d pid=%d created.\n", tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec, i+1, getpid());
            sem_post(mutex_log_file);
            fflush(fp2);
            fprintf(stderr, "[Simulation %d]: Warehouse Process %d created.\n", getppid(), getpid());
            (shm_war+i)->w_no = i+1; //unique identifier of warehouse
            warehouse_process(i+1);
            exit(0);
        }

    }    
    if ((fd=open(PIPE_NAME, O_RDWR)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0);
    }
    
    //create thread responsible for warehouse product supply
    pthread_create(&supply_thread, NULL, supply, NULL); 
    fprintf(stderr, "[Simulation %d]: Supply Thread created.\n", getpid());


    while(1){
        fprintf(stderr, "Insert new order: \n");
        fgets(order, 80, stdin);
        fprintf(stderr, "Simulation: Sending message %s\n", order);
        write(fd, order, sizeof(order)); 
    }

    return 0;
}

void warehouse_process(int w_no){
    Queue_Msg msg;

    sigset_t block_ctrlc;
    sigemptyset (&block_ctrlc);
    sigaddset (&block_ctrlc, SIGINT);
    
    while(1){
        msgrcv(mqid, &msg, sizeof(Queue_Msg), w_no, 0);
        #ifdef DEBUG
        printf("warehouse w_no %d reads %ld\n", w_no, msg.type); 
        #endif       
        if(strcmp(msg.name, "ORDER") == 0){
            sigprocmask (SIG_BLOCK, &block_ctrlc, NULL);
            printf("[Warehouse %ld] Received order message from drone.\n", msg.type);
            printf("[Warehouse %d] Loading %d products..\n", w_no, msg.product.quant);
            sleep(msg.product.quant*time_unit);
            msg.type = TYPE_DRONE;
            printf("[Warehouse %d] Loading complete!\n", w_no);
            sem_wait(mutex_war);
            update_num_products_loaded(msg.product.quant);
            sem_post(mutex_war);
            msgsnd(mqid, &msg, sizeof(Queue_Msg), 0);
            sigprocmask (SIG_UNBLOCK, &block_ctrlc, NULL);
        }
        else if(strcmp(msg.name, "SUPPLY") == 0){
            sem_wait(mutex_war);
            int ind = ind_product_warehouse(msg.product.type, w_no);
            int before = (shm_war+w_no -1)->products[ind].quant;
            update_warehouse_stock(w_no, msg.product.type, msg.product.quant);
            int after = (shm_war+w_no -1)->products[ind].quant;
            printf("[Warehouse %ld] Received a supply of %d %s (before:%d after:%d).\n", msg.type, msg.product.quant, msg.product.type, before, after);
            sem_post(mutex_war);
            sem_post(mutex_centr); 
        }
    }
}
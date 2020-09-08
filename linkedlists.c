#include "linkedlists.h"

//ORDERS
ListOrder add_order(ListOrder list, ListOrder new_order) {
    ListOrder aux = list;
    if (list == NULL) {
        list = new_order;
        return list;
    }
    while(aux->next != NULL) {
        aux = aux->next;
    }
    aux->next = new_order;
    return list;
}

ListOrder remove_order(ListOrder list, int id) {
    ListOrder anterior = NULL;
    ListOrder atual = list;
    while((atual != NULL) && (atual->info.id != id)) {
        anterior = atual;
        atual = atual->next;
    }
    if (atual!= NULL) {
        if (anterior!=NULL)
            anterior->next = atual->next;
        else
            list=atual->next;
        free(atual);
    }
    return list;
}


void print_pending_orders(ListOrder list) {
    if (list == NULL)
        printf("There is no pending orders.");
    while(list != NULL) {
        printf("Order Id: %d\n", list->info.id);
        printf("Product name: %s\n", list->info.product.type);
        printf("Product quantity: %d\n", list->info.product.quant);
        printf("Product destination: %f %f\n\n", list->info.destination.x, list->info.destination.y);
        list = list->next;
    }
}


ListOrder destroy_order_list(ListOrder list){
    ListOrder aux;
    while(list != NULL){
        aux = list;
        list = list->next;
        free(aux);
    }
    return NULL;
}


//DRONES
ListDrone add_drone(ListDrone list, ListDrone new_drone) {
    ListDrone aux = list;
    if (list == NULL) {
        list = new_drone;
        return list;
    }
    while(aux->next != NULL) {
        aux = aux->next;
    }
    aux->next = new_drone;
    return list;
}

ListDrone remove_drone(ListDrone list, int id) {
    ListDrone anterior = NULL;
    ListDrone atual = list;
    while((atual != NULL) && (atual->info.id != id)) {
        anterior = atual;
        atual = atual->next;
    }
    if (atual!= NULL) {
        if (anterior!=NULL)
            anterior->next = atual->next;
        else
            list=atual->next;
        free(atual);
    }
    return list;
}


void print_drones(ListDrone list) {
    if (list == NULL)
        printf("There is no drones.");
    while(list != NULL) {
        printf("Drone Id: %d\n", list->info.id);
        printf("Drone free: %d\n", list->info.free);
        printf("Drone behaviour: %d\n", list->info.behaviour);
        printf("Drone local x: %f\n", list->info.local.x);
        printf("Drone local y: %f\n", list->info.local.y);
        list = list->next;
    }
}

ListDrone find_drone(ListDrone list, int id) {
    ListDrone atual = list;
    while((atual != NULL) && (atual->info.id != id)) {
        atual = atual->next;
    }
    return atual;
}

ListDrone destroy_drone_list(ListDrone list){
    ListDrone aux;
    while(list != NULL){
        aux = list;
        list = list->next;
        free(aux);
    }
    return NULL;
}
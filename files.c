#include "files.h"

Warehouse * open_config_file(){
    Warehouse *warehouses;
    FILE * fp;
    int size, i, j, k;
    char str[200], *pch;
    char prod[30];
  
    fp = fopen("config.txt", "r");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END); /*go to the end of the file*/
        size = ftell(fp); /*returns the current read or write position in the file (in bytes)*/
        if (size == 0) {
            printf("\nO ficheiro esta vazio.\n"); 
            exit(1);
        }
        else {
            fseek(fp, 0, SEEK_SET);/*return to the beginning of the file*/
            fgets(str, 200, fp);
             
            pch = strtok(str, ",");
            sscanf(pch, "%lf", &(coord_max.x));
            pch = strtok(NULL, ",");
            sscanf(pch, "%lf", &(coord_max.y));
            fgets(str, 200, fp);
            for (i =0; i< strlen(str); i++){ //count number of products
                if(str[i] == ','){
                    num_prod++;
                }
            }
            product_types = (Product *)malloc(num_prod*sizeof(Product));
            pch = strtok(str, ",");
            for(i=0;i<num_prod;i++){
                strcpy(product_types[i].type, pch);
                pch = strtok(NULL, ",");

            }
            product_types[num_prod-1].type[strlen(product_types[num_prod-1].type)-1] = '\0';

            fgets(str, 200, fp);
            sscanf(str, "%d", &num_drones);

            fgets(str, 200, fp);
            pch = strtok(str, ",");
            sscanf(pch, "%d", &freq_abast);
            pch = strtok(NULL, ",");
            sscanf(pch, "%d", &quant_abast);
            pch = strtok(NULL, ",");
            sscanf(pch, "%d", &time_unit);
            fgets(str, 200, fp);
            fgets(str, 200, fp);
            sscanf(str, "%d", &num_warehouses);
            warehouses = (Warehouse *)malloc(num_warehouses*sizeof(Warehouse));
            
            for(i=0;i<num_warehouses;i++){
                for(j=0;j<num_prod;j++) {
                    warehouses[i].products[j].quant = -1;
                }
                fgets(str, 200, fp); 
                pch = strtok(str, " ");
                strcpy(warehouses[i].name, pch); //warehouse name
                pch = strtok(NULL, " "); 
                pch = strtok(NULL, " "); //move forward up to 2 spaces
                
                sscanf(pch, "%lf", &warehouses[i].local.x);
                pch = strtok(NULL, " ");
                sscanf(pch, "%lf", &warehouses[i].local.y);
                pch = strtok(NULL, ":");

                for (k = 0; k<3; k++){ //quantity of products by warehouse = 3
                    pch = strtok(NULL, ",");
                    strcpy(prod, pch);
                    strcpy(warehouses[i].products[k].type, prod);
                    pch = strtok(NULL, ",");
                    sscanf(pch, "%d", &(warehouses[i].products[k].quant));
                }      
            } 
       }                         
    }
    else {
        printf("\nThe file is not available.");
        exit(1); /*error code file not available*/
    }  
    fclose(fp);
    return warehouses;
}
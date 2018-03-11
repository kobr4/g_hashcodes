#include <stdio.h>
#include <math.h>

typedef struct {
    int r;
    int c;
    int d;
    int de;
    int maxl;
    int T; //num de tours
    int O; //num de order
    int W; //num de warehouse
} T_PARAMS;

T_PARAMS gParams;

#define FLY 1
#define LOAD 2
#define DELIVER 3
#define NOOP 0

#define N_TYPES_STOCK 16

typedef struct {
 int r;
 int c;
} T_POS;

typedef struct {
    int type;
    int p1;
    int p2;
} T_MV; //mouvement


typedef struct {
    int r;
    int c;
    int * warehouse_stock;
} T_WAREHOUSE; //warehouse

typedef struct {
    int r;
    int c;
    T_MV * mv;
    int mvcount;
} T_DRONE;

typedef struct {
    T_POS client_pos;
    int pcount;
    int * plist;
    int * delivered;
    int type_items[N_TYPES_STOCK];
} T_ORDER;




int nb_items;
int *weight;
int nb_warehouse;
int nb_orders;
T_WAREHOUSE *warehouse;
T_ORDER *set_orders;



int distance(int r1, int c1, int r2, int c2) {
    return (int)sqrt( (r1 - r2) * (r1 - r2) + (c1 - c2) * (c1 - c2) );
}

int getWhP(T_WAREHOUSE * wh, int p) {
    for (int i = 0; i < gParams.W;i++) {
        if (wh[i].warehouse_stock[p] > 0) {
            wh[i].warehouse_stock[p] --;
            return i;
        }
    }
}


int assignDroneLoad(T_DRONE * drone, int wh,int p, int t) {
    drone->mv[drone->mvcount].type = LOAD;
    drone->mv[drone->mvcount].p2 = wh;
    drone->mv[drone->mvcount].p1 = p;
    drone->mvcount++;
    return 0;
}

int assignDroneDeliver(T_DRONE * drone, int c, int p, int t) {
    drone->mv[drone->mvcount].type = DELIVER;
    drone->mv[drone->mvcount].p2 = c;
    drone->mv[drone->mvcount].p1 = p;
    drone->mvcount++;
    return 0;
}
int computeMoves(T_DRONE * drone,T_ORDER * order,T_WAREHOUSE * wh) {
    int count = 0;
    for (int i = 0;i < drone->mvcount;i++) {
        switch(drone->mv[i].type) {
            case DELIVER : count += distance(drone->c, drone->r, order[drone->mv[i].p2].client_pos.c,  order[drone->mv[i].p2].client_pos.r) + 1;break;
            case LOAD : count += distance(drone->c, drone->r,  wh[drone->mv[i].p2].c, wh[drone->mv[i].p2].r) + 1;break;
        }
    }
    return count;
}

int getAvailableDrone(T_DRONE * dList,T_ORDER * order, T_WAREHOUSE * wh)  {
    for(int i = 0;i < gParams.d;i++) {
        if (computeMoves(&dList[i], order, wh) < gParams.de) {
            return i;
        }
        //if (dList[i].mv[0].type == NOOP) return i;
    }
    return -1;
}

int fillOrder(T_ORDER * order, int oi,T_WAREHOUSE * wh, T_DRONE * dList) {
    for (int i = 0;i < order->pcount;i++) {
        int di = getAvailableDrone(dList, order, wh);
        if (di == -1) return 0;
        int wi = getWhP(wh, order->plist[i]);
        printf("d: %d, w:%d\n", di, wi);
        assignDroneLoad(&dList[di], wi, order->plist[i],0);
        assignDroneDeliver(&dList[di], oi,order->plist[i],0);
    }
}

int solver(T_ORDER * oList, T_WAREHOUSE * wList, T_DRONE * dList) {
    for (int i = 0; i <  gParams.O;i++) {
        fillOrder(&oList[i], i,wList,dList);
    }
}

void printOutput(T_DRONE * dList) {
   for (int i = 0; i <  gParams.d;i++) {
       for (int j = 0;j < gParams.T;j++) {
            if (dList[i].mv[j].type != NOOP) {
                switch(dList[i].mv[j].type) {
                    case LOAD :
                        printf("%d %c %d %d %d\n",i,'L', dList[i].mv[j].p2, dList[i].mv[j].p1, 1);
                        break;
                    case DELIVER :
                        printf("%d %c %d %d %d\n",i,'D', dList[i].mv[j].p2, dList[i].mv[j].p1, 1);
                        break;
                }
            }
       }
   }
}

int main(int argc, char **argv) {

   int ret_items;

   FILE * f = fopen("C:\\temp\\input_set.dat","r");

   if (f==NULL) {
        printf("fopen err\n");
        exit(1);
   }
   //vsize = (int*)malloc(sizeof(int)*gParams.v);

	fscanf(f,"%d %d %d %d %d\n", &gParams.r, &gParams.c, &gParams.d, &gParams.de, &gParams.maxl);
	printf("%d %d %d %d %d\n", gParams.r, gParams.c, gParams.d, gParams.de, gParams.maxl);

	fscanf(f,"%d\n", &nb_items);
	printf("nb_items=%d\n", nb_items);

	weight = (int*)malloc(sizeof(int)*nb_items);
	for (int i=0; i<nb_items; i++)
    {
        fscanf(f,"%d ", &weight[i]);
    }

    for (int i=0; i<nb_items; i++)
    {
        printf("w[%d]=%d  \n", i, weight[i]);
    }

    printf("\nnow processing warehouse...\n");

    fscanf(f,"%d\n", &nb_warehouse);
    printf("nb_warehouse=%d\n", nb_warehouse);

    warehouse=(T_WAREHOUSE *)malloc(sizeof(T_WAREHOUSE) * nb_warehouse);

    for (int i=0; i<nb_warehouse; i++) {
        fscanf(f,"%d %d\n", &warehouse[i].r, &warehouse[i].c);
        printf("warehouse pos : %d %d\n", warehouse[i].r, warehouse[i].c);

        warehouse[i].warehouse_stock=(int *)malloc(sizeof(int)*N_TYPES_STOCK);
        for (int n=0; n<nb_items; n++) {
            fscanf(f,"%d ", &warehouse[i].warehouse_stock[n]);
        }

        fscanf(f,"\n");

        for (int j=0;j<nb_items;j++) {
            printf("type %d has %d\n", j, warehouse[i].warehouse_stock[j]);
        }

    }

    //
    printf("now processing order...\n");

    fscanf(f,"%d\n", &nb_orders);
    set_orders = (T_ORDER *)malloc(sizeof(T_ORDER)*nb_orders);
    printf("nb_orders=%d\n", nb_orders);

    for (int k=0;k<nb_orders;k++) {
        fscanf(f,"%d %d\n", &set_orders[k].client_pos.r, &set_orders[k].client_pos.c);
        printf("\npos: %d %d\n", set_orders[k].client_pos.r, set_orders[k].client_pos.c);
        int n_items;
        fscanf(f,"%d\n", &n_items);
        printf("n_items=%d\n", n_items);

        int count = 0;
        for (int m=0;m<n_items;m++) {

            fscanf(f,"%d ", &set_orders[k].type_items[m]);
            count++;
        }
        set_orders[k].pcount = count;
        set_orders[k].plist = (int *)malloc((sizeof(int) * count));
        for (int m=0;m<n_items;m++) {
            set_orders[k].plist[m] = set_orders[k].type_items[m];
        }

        for (int m=0;m<n_items;m++) {
            printf("%d has item %d\n", m, set_orders[k].type_items[m]);
        }
        fscanf(f,"\n");
    }
    gParams.O = nb_orders;
    gParams.T = gParams.de;
    gParams.W = nb_warehouse;
    T_DRONE * dList = (T_DRONE*)malloc(sizeof(T_DRONE)*gParams.d);
    for(int i = 0;i < gParams.d;i++) {
        dList[i].c = warehouse[0].c;
        dList[i].r = warehouse[0].r;
        dList[i].mvcount = 0;
        dList[i].mv = (T_MV*)calloc(1,sizeof(T_MV)*gParams.de);
        for (int j = 0;j < gParams.de;j++) dList[i].mv[j].type = NOOP;
    }

    solver(set_orders, warehouse, dList);
    printOutput(dList);
    return 0;

}

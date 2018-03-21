// MULTITHREADED SOLVER GOOGLE HASHCODE 2018 - AMD EDITION
// RUN ON AMD THREADRIPPER 1950X FOR COMPETITION
// BY MICHEL HE, NICOLAS MY, FENG YANG - MAR. 2018

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define Win32

#if defined (Win32)
#  include <windows.h>
#  define psleep(sec) Sleep ((sec))
#elif defined (Linux)
#  include <unistd.h>
#  define psleep(sec) sleep ((sec))
#endif

int NB_CLIENTS;
int COPIES_BY_THREAD;

#define RATE_BONUS_FLEET 0.7

#define DEBUG_INFO printf

#define MAX_RIDES 100000
#define MAX_SIM_STEPS 10000000000

const int bonus_factor = 10;    //adapt it to 15 for dataset E highbonus
const int div_ride_factor = 2; //adapt it to 10 in dataset A,B
const int step_cost = 100;
const int cost_limit = 100000;

typedef struct
{
	int R; //row
	int C; //col
	int F; //number of vehicles in the fleet
	int N; //number of rides
	int B; //per-ride bonus for starting the ride on time
	int T; //num de tours
} T_PARAMS;

T_PARAMS gParams;


typedef struct
{
	int r;
	int c;
} T_POS;

typedef struct
{
	int type;
	int p1;
	int p2;
} T_MV; //mouvement

typedef struct
{
	int r;
	int c;
	T_MV * mv;
	int mvcount;
	int t;
} T_VEHICULE;

typedef struct
{
	T_POS ride_from;
	T_POS ride_to;
	int earliest;
	int latest;
	int idx;
	int filled;
	int filled_at;
	int length;
	int in_range;
	int latest_start;
} T_RIDE;


T_RIDE * rides;
T_RIDE * Master_ride;
T_VEHICULE * vcs;
int *score_table;


typedef struct
{
    int stock;

    pthread_t thread_store;
    pthread_t *thread_clients;

    pthread_mutex_t mutex_store;
    pthread_cond_t cond_store;
    pthread_mutex_t mutex_clients;
    pthread_cond_t cond_clients;
    pthread_mutex_t mutex_ready;
    pthread_cond_t cond_ready;

}
store_t;

static store_t store =
{
    .mutex_store = PTHREAD_MUTEX_INITIALIZER,
    .mutex_clients = PTHREAD_MUTEX_INITIALIZER,
    .cond_store = PTHREAD_COND_INITIALIZER,
    .cond_clients = PTHREAD_COND_INITIALIZER,
};

// Solving start here
int distance(int r1, int c1, int r2, int c2)
{
	return (abs(r1 - r2) + abs(c1 - c2));
}

int distance_vc_ride(T_VEHICULE vc, T_RIDE ride) {
	int d = distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c);
	if (d < (ride.earliest - vc.t)) {
		d = ride.earliest - vc.t;
	}

	return d;
}

int distance_ride(T_RIDE ride) {
	return distance(ride.ride_from.r, ride.ride_from.c, ride.ride_to.r, ride.ride_to.c);
}

char is_candidate(T_RIDE ride, T_VEHICULE vc, int dist_cost) {
	return (vc.t + dist_cost) < ride.latest;
}

static void * fn_clients (void * p_data)
{
    int nb = (int) p_data;
    int i;

    for (i = nb*COPIES_BY_THREAD; i < (nb+1)*COPIES_BY_THREAD; i++) {
        int d = distance_vc_ride(vcs[i], Master_ride[nb]);
		if (is_candidate(Master_ride[nb], vcs[i], d + Master_ride[nb].length)) {
			int cost = Master_ride[nb].length/div_ride_factor + d - (((vcs[i].t + d) == Master_ride[nb].earliest) ? gParams.B * bonus_factor : 0);
			score_table[i] = cost;
		}
		else {
            score_table[i] = -1;
		}
	}

	if ((nb+1)==NB_CLIENTS) {
        for(i = (nb+1)*COPIES_BY_THREAD; i < gParams.F; i++) {
            int d = distance_vc_ride(vcs[i], Master_ride[nb]);
            if (is_candidate(Master_ride[nb], vcs[i], d + Master_ride[nb].length)) {
                int cost = Master_ride[nb].length/div_ride_factor + d - (((vcs[i].t + d) == Master_ride[nb].earliest) ? gParams.B * bonus_factor : 0);
                score_table[i] = cost;
            }
            else {
                score_table[i] = -1;
            }
        }
	}


    return NULL;
}

int select_car(T_RIDE ride, T_VEHICULE * vcs, int max_cost) {
	int lowest = cost_limit;
	int selected = -1;


    for (int i = 0; i < NB_CLIENTS; i++)
    {
        Master_ride[i] = ride;
    }

    for (int i = 0; i < NB_CLIENTS; i++)
    {
        int ret = pthread_create (
                  & store.thread_clients[i], NULL,
                  fn_clients, (void *) i);

        if (ret)
        {
            fprintf (stderr, "ERR : create thr %s", strerror (ret));
        }
    }

    for (int i = 0; i < NB_CLIENTS; i++)
    {
        int ret = pthread_join (store.thread_clients[i], NULL);
        if (ret)
        {
            fprintf (stderr, "ERR : join thr %s", strerror (ret));
        }
    }

	for (int i = 0; i < gParams.F; i++) {
            int cost = score_table[i];
			if (cost != -1 && lowest > cost) {
				selected = i;
				lowest = cost;
			}
	}

	return selected;
}

void assignRide(T_VEHICULE * vc, T_RIDE ride, int ri) {

	vc->mv[vc->mvcount].p1 = ri;
	vc->mvcount++;

	vc->t += distance_vc_ride(*vc, ride) + distance_ride(ride);

	vc->c = ride.ride_to.c;
	vc->r = ride.ride_to.r;
}


int compare(void const *a, void const *b) {
    T_RIDE * req1 = (T_RIDE*)a;
    T_RIDE * req2 = (T_RIDE*)b;
    if (req1->earliest == req2->earliest)
    {

        int latest1 = req1->latest;
        int latest2 = req2->latest;
        if (latest1==latest2)
        {
			int dist1 = distance(0,0,req1->ride_from.r,req1->ride_from.c);
			int dist2 = distance(0,0,req2->ride_from.r,req2->ride_from.c);
			if (dist1==dist2) {
					int run_dist1 = distance(req1->ride_from.r,req1->ride_from.c, req1->ride_to.r,req1->ride_to.c);
					int run_dist2 = distance(req2->ride_from.r,req2->ride_from.c, req2->ride_to.r,req2->ride_to.c);
					return run_dist1 - run_dist2;
			}
			else
				return dist1 - dist2;
			}
        else
            return latest1 - latest2;

    }
    else
        return req1->earliest - req2->earliest;
}

int compare_ls(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	// upward
	if (req1->latest_start==req2->latest_start) {
        int run_dist1 = distance(req1->ride_from.r,req1->ride_from.c, req1->ride_to.r,req1->ride_to.c);
        int run_dist2 = distance(req2->ride_from.r,req2->ride_from.c, req2->ride_to.r,req2->ride_to.c);
        return run_dist2 - run_dist1;
	}
    else
	  return req1->latest_start - req2->latest_start;
}

int compare_ride_length(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	return req1->length - req2->length;
}


void fillRideOrder(T_RIDE * rides, T_VEHICULE * vcs) {


	T_RIDE * oListSorted = (T_RIDE *)malloc(sizeof(T_RIDE)*gParams.N);
	memcpy(oListSorted, rides, sizeof(T_RIDE) * gParams.N);
	qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare);

	for (int i = 0; i < gParams.N*RATE_BONUS_FLEET; i++) {
		if (rides[oListSorted[i].idx].filled == 1) continue;

		int vci = select_car(oListSorted[i], vcs, 1);

		if (vci != -1) {
			//DEBUG_INFO("vc: %d ride: %d \n", vci, i);
			rides[oListSorted[i].idx].filled_at = vcs[vci].t + distance_vc_ride(vcs[vci], oListSorted[i]);
			assignRide(&vcs[vci], oListSorted[i], oListSorted[i].idx);
			rides[oListSorted[i].idx].filled = 1;
		}
	}

	int bonus_count = 0;
	for (int i = 0; i < gParams.N; i++) {
		if (rides[i].filled) bonus_count++;
	}
	DEBUG_INFO("Dispatched for bonus: %d\n", bonus_count);

	qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare_ls);
    int ls_count = 0;
	for (int i = 0; i < gParams.N; i++) {
		if (rides[oListSorted[i].idx].filled == 1) continue;

		int vci = select_car(oListSorted[i], vcs, 0);

		if (vci != -1) {
			//DEBUG_INFO("vc: %d ride: %d \n", vci, i);
			rides[oListSorted[i].idx].filled_at = vcs[vci].t + distance_vc_ride(vcs[vci], oListSorted[i]);
			assignRide(&vcs[vci], oListSorted[i], oListSorted[i].idx);
			rides[oListSorted[i].idx].filled = 1;
			ls_count++;
		}
	}

	DEBUG_INFO("Dispatched for ls: %d\n", ls_count);
	//DEBUG_INFO("ride 0: %d ride 1: %d\n", oListSorted[0].earliest, oListSorted[1].earliest);
}

void solver(T_RIDE * rides, T_VEHICULE * vcs) {
	fillRideOrder(rides, vcs);
}
// Solving end here
void output(T_VEHICULE * vcs, char * filename) {
	DEBUG_INFO("printing output\n");
	FILE * f = fopen(filename, "w");

	for (int i = 0; i < gParams.F; i++) {
		fprintf(f, "%d ", vcs[i].mvcount);
		for (int j = 0; j < vcs[i].mvcount; j++) fprintf(f, "%d ", vcs[i].mv[j].p1);
		fprintf(f, "\n");
	}

	fclose(f);

}


int score_vc(T_VEHICULE vc, T_RIDE * rides) {
	int score = 0;
	int r = 0;
	int c = 0;
	int t = 0;
	for(int j = 0;j < vc.mvcount;j++) {
		int ri = vc.mv[j].p1;
		vc.r = r;
		vc.c = c;
		vc.t = t;
		score += rides[ri].length + ((vc.t + distance_vc_ride(vc, rides[ri]) == rides[ri].earliest) ? gParams.B : 0);
		t = vc.t + distance_vc_ride(vc, rides[ri])+ rides[ri].length;
		if (rides[ri].latest < t) {
			puts("ERROR");
			return -1;
		}

		r = rides[ri].ride_to.r;
		c = rides[ri].ride_to.c;
	}

	return score;
}


static void * score_clients (void * p_data)
{
    int nb = (int) p_data;
    int i;

    for (i = nb*COPIES_BY_THREAD; i < (nb+1)*COPIES_BY_THREAD; i++) {
        score_table[i] = score_vc(vcs[i], rides);
	}

	if ((nb+1)==NB_CLIENTS) {
        for(i = (nb+1)*COPIES_BY_THREAD; i < gParams.F; i++) {
         score_table[i] = score_vc(vcs[i], rides);
        }
	}


    return NULL;
}

int score_vcs(T_VEHICULE * vcs, T_RIDE * rides) {
	int score = 0;


    for (int i = 0; i < NB_CLIENTS; i++)
    {
        int ret = pthread_create (
                  & store.thread_clients[i], NULL,
                  score_clients, (void *) i);

        if (ret)
        {
            fprintf (stderr, "ERR : create thr %s", strerror (ret));
        }
    }

    for (int i = 0; i < NB_CLIENTS; i++)
    {
        int ret = pthread_join (store.thread_clients[i], NULL);
        if (ret)
        {
            fprintf (stderr, "ERR : join thr %s", strerror (ret));
        }
    }


	for (int i = 0;i < gParams.F;i++) {
		int a = score_table[i];
		score += a;
	}
	return score;
}

int main(int argc, char **argv)
{
    int nb_rides;
	int ret_item;
	char filename[256];
	if (argc >= 2)
	{
		strcpy(filename, argv[1]);
	}
	else
	{
		strcpy(filename, "c:\\temp\\a_example.in");
	}
	DEBUG_INFO("open %s\n", filename);

	FILE * fin = fopen(filename, "r");

	if (fin == NULL)
	{
		DEBUG_INFO("fopen err\n");
		exit(1);
	}

	fscanf(fin, "%d %d %d %d %d %d\n", &gParams.R, &gParams.C, &gParams.F, &gParams.N, &gParams.B, &gParams.T);
	DEBUG_INFO("map %d %d, number of vehicles :%d, number of rides %d, per-ride bonus %d, number of steps %d\n",
		gParams.R, gParams.C, gParams.F, gParams.N, gParams.B, gParams.T);

	assert(gParams.N<MAX_RIDES);
	assert(gParams.T<MAX_SIM_STEPS);
	if (gParams.F < 50) {
		NB_CLIENTS = 1;
	}
	else {
		NB_CLIENTS = gParams.F / 50 + 1;
	}

	Master_ride = (T_RIDE*)malloc(sizeof(T_RIDE) * NB_CLIENTS);
	store.thread_clients = (pthread_t *)malloc(sizeof(pthread_t) * NB_CLIENTS);

	nb_rides = gParams.N;
	rides = (T_RIDE*)malloc(sizeof(T_RIDE) * gParams.N);

	for (nb_rides = 0; nb_rides < gParams.N; nb_rides++)
	{
		ret_item = fscanf(fin, "%d %d %d %d %d %d\n",
			&rides[nb_rides].ride_from.r,
			&rides[nb_rides].ride_from.c,
			&rides[nb_rides].ride_to.r,
			&rides[nb_rides].ride_to.c,
			&rides[nb_rides].earliest,
			&rides[nb_rides].latest);


		if (ret_item == 0)
			break;

		DEBUG_INFO("ride %d :%d %d %d %d %d %d\n", nb_rides,
			rides[nb_rides].ride_from.r,
			rides[nb_rides].ride_from.c,
			rides[nb_rides].ride_to.r,
			rides[nb_rides].ride_to.c,
			rides[nb_rides].earliest,
			rides[nb_rides].latest);
	}

	fclose(fin);

	COPIES_BY_THREAD = gParams.F / NB_CLIENTS;
	DEBUG_INFO("NB_CLIENTS=%d COPIES_BY_THREAD=%d\n", NB_CLIENTS, COPIES_BY_THREAD);

	score_table = (int *)malloc(sizeof(int)*gParams.F);

	vcs = (T_VEHICULE *)malloc(sizeof(T_VEHICULE)*gParams.F);
	if (vcs == NULL) {
		puts("Not enough memory");
	}
	for (int i = 0; i < gParams.F; i++) {
		vcs[i].c = 0;
		vcs[i].r = 0;
		vcs[i].t = 0;
		vcs[i].mv = (T_MV*)malloc(sizeof(T_MV)*gParams.T);
		vcs[i].mvcount = 0;
	}

	for (int i = 0; i < gParams.N; i++) {
		rides[i].idx = i;
		rides[i].filled = 0;
		rides[i].length = distance_ride(rides[i]);
		rides[i].latest_start = rides[i].latest - rides[i].length;
	}

	solver(rides, vcs);

	char str[255];

	int score = score_vcs(vcs, rides);
	sprintf(str, "%s.%d.out", filename, score);
	output(vcs, str);

	DEBUG_INFO("score: %d\n",score);
	return 0;
}




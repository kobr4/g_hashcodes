#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define MAX_RIDES 100000
#define MAX_SIM_STEPS 10000000000

const int bonus_factor = 7;
const int step_cost = 40;
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
int nb_rides;

T_VEHICULE * vcs;

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

int compute_ride_cost(T_RIDE ride, T_VEHICULE vc) {
	return distance_vc_ride(vc, ride) + ride.length;
}

bool is_candidate(T_RIDE ride, T_VEHICULE vc) {
	return (vc.t + compute_ride_cost(ride, vc)) < ride.latest;
}

int select_car(T_RIDE ride, T_VEHICULE * vcs, int max_cost) {
	int lowest = -1;
	int selected = -1;

	for (int i = 0; i < gParams.F; i++) {
		if (is_candidate(ride, vcs[i])) {
	
			int cost = ride.length/5 + distance_vc_ride(vcs[i], ride) - (((vcs[i].t + distance_vc_ride(vcs[i],ride)) == ride.earliest) ? gParams.B * bonus_factor : 0);			
			if ((selected == -1 || lowest > cost) && (cost < max_cost)) {
				selected = i;
				lowest = cost;			
			}
		}
	}

	return selected;
}

void assign_ride(T_VEHICULE * vc, T_RIDE ride, int ri) {

	vc->mv[vc->mvcount].type = 1;
	vc->mv[vc->mvcount].p1 = ri;
	vc->mv[vc->mvcount].p2 = vc->t;
	vc->mvcount++;

	vc->t += distance_vc_ride(*vc, ride) + ride.length;

	vc->c = ride.ride_to.c;
	vc->r = ride.ride_to.r;
}

int fill_ride_order(T_RIDE * rides, T_VEHICULE * vcs) {
	int vc_selected = -1;
	int max_cost = 0;
	while (vc_selected == -1 && max_cost < cost_limit) {
		for (int i = 0; i < gParams.N; i++) {
			if (rides[rides[i].idx].filled == 1) continue;

			vc_selected = select_car(rides[i], vcs, max_cost);

			if (vc_selected != -1) {
				printf("vc: %d -> ride: %d\n", vc_selected,rides[i].idx);
				rides[rides[i].idx].filled_at = vcs[vc_selected].t + distance_vc_ride(vcs[vc_selected], rides[i]);
				assign_ride(&vcs[vc_selected], rides[i], rides[i].idx);
				rides[rides[i].idx].filled = 1;
				break;
			}
		}

		max_cost += step_cost;
	}
	return max_cost;
}

void solver(T_RIDE * rides, T_VEHICULE * vcs) {
	int ret = 0;
	while (ret < cost_limit) {
		ret = fill_ride_order
	(rides, vcs);
	}
}

// Solving end here
void output(T_VEHICULE * vcs, char * filename) {
	printf("printing output\n");
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

int score_vcs(T_VEHICULE * vcs, T_RIDE * rides) {
	int score = 0;
	for (int i = 0;i < gParams.F;i++) {
		int a = score_vc(vcs[i], rides);
		score += a;
	}
	return score;
}

int main(int argc, char **argv)
{
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
	printf("open %s\n", filename);

	FILE * fin = fopen(filename, "r");

	if (fin == NULL)
	{
		printf("fopen err\n");
		exit(1);
	}

	fscanf(fin, "%d %d %d %d %d %d\n", &gParams.R, &gParams.C, &gParams.F, &gParams.N, &gParams.B, &gParams.T);
	printf("map %d %d, number of vehicles :%d, number of rides %d, per-ride bonus %d, number of steps %d\n",
		gParams.R, gParams.C, gParams.F, gParams.N, gParams.B, gParams.T);

	assert(gParams.N<MAX_RIDES);
	assert(gParams.T<MAX_SIM_STEPS);

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

		printf("ride N %d :%d %d %d %d %d %d\n", nb_rides,
			rides[nb_rides].ride_from.r,
			rides[nb_rides].ride_from.c,
			rides[nb_rides].ride_to.r,
			rides[nb_rides].ride_to.c,
			rides[nb_rides].earliest,
			rides[nb_rides].latest);
	}

	fclose(fin);

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
	char filenamecat[256];
	char str[255];
	int score = score_vcs(vcs, rides);
	sprintf(str, "%s.%d.out", filename, score);
	output(vcs, str);

	printf("score: %d\n",score);
	return 0;
}

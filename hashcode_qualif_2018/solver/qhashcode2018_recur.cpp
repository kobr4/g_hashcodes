#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define MAX_RIDES 100000
#define MAX_SIM_STEPS 10000000000

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

#define WAITING 0
#define RUNNING 1
#define BOOKED  2
#define ARRIVED 3


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

int computeRideCost(T_RIDE ride, T_VEHICULE vc) {
	return distance_vc_ride(vc, ride) + ride.length;
}

bool is_candidate(T_RIDE ride, T_VEHICULE vc) {
	return (vc.t + computeRideCost(ride, vc)) < ride.latest;
}

int selectCar(T_RIDE ride, T_VEHICULE * vcs) {
	int lowest = -1;
	int selected = -1;

	for (int i = 0; i < gParams.F; i++) {
		if (is_candidate(ride, vcs[i])) {
		//int cost = computeRideCost(ride, vcs[i]);
		int cost = distance_vc_ride(vcs[i], ride);
	
		//int cost = ride.earliest - (vcs[i].t + distance_vc_ride(vcs[i], ride));
		// - (((vcs[i].t + distance_vc_ride(vcs[i],ride)) == ride.earliest) ? gParams.B : 0);
			if (selected == -1 || lowest > cost) {
				selected = i;
				lowest = cost;			
			}
		}
	}

	return selected;
}

void assignRide(T_VEHICULE * vc, T_RIDE ride, int ri) {

	vc->mv[vc->mvcount].type = RUNNING;
	vc->mv[vc->mvcount].p1 = ri;
	vc->mv[vc->mvcount].p2 = vc->t;
	vc->mvcount++;

	vc->t += distance_vc_ride(*vc, ride) + ride.length;

	vc->c = ride.ride_to.c;
	vc->r = ride.ride_to.r;
}


int compare(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	if (req1->earliest != req2->earliest)
		return req1->earliest - req2->earliest;
	else 
		return (req1->latest - req1->length) - (req2->latest - req2->length);
}

void fillRideOrder(T_RIDE * rides, T_VEHICULE * vcs, T_RIDE * oListSorted) {
	for (int i = 0; i < gParams.N; i++) {
		if (rides[oListSorted[i].idx].filled == 1) continue;

		int vci = selectCar(oListSorted[i], vcs);

		if (vci != -1) {
			rides[oListSorted[i].idx].filled_at = vcs[vci].t + distance_vc_ride(vcs[vci], oListSorted[i]);
			assignRide(&vcs[vci], oListSorted[i], oListSorted[i].idx);
			rides[oListSorted[i].idx].filled = 1;
		}
	}
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

void reset_lists(T_VEHICULE * vcs, T_RIDE * rides) {
	for (int i = 0;i < gParams.F;i++) {
		vcs[i].mvcount = 0;
		vcs[i].c = 0;
		vcs[i].r = 0;
		vcs[i].t = 0;
	}
	for (int i = 0;i < gParams.N;i++) rides[i].filled = 0;
}


void apply_blacklist(T_RIDE * rides, int * blacklist, int nb) {
	for(int i = 0;i < nb; i++) {
		rides[blacklist[i]].filled = 1;
	}
}

int fillRideOrderWithBlacklist(T_RIDE * rides, T_VEHICULE * vcs, int * blacklist, int nb, T_RIDE * oListSorted) {
	reset_lists(vcs, rides);
	apply_blacklist(rides, blacklist, nb);
	fillRideOrder(rides, vcs, oListSorted);
	return score_vcs(vcs,rides);
}


int recur_solver(int score, T_RIDE * rides, T_VEHICULE * vcs, int * blacklist, int nb, T_RIDE * oListSorted) {
	int best_score = score; 
	int selected = -1;
	for (int i = 0;i < gParams.N;i++) {
		blacklist[nb] = i;
		int c_score = fillRideOrderWithBlacklist(rides, vcs, blacklist, nb+1, oListSorted);
		
		if (c_score > score) {
			if (nb+1 < gParams.N) {
				//c_score = recur_solver(c_score, rides, vcs, blacklist, nb+1, oListSorted);
				if (c_score > best_score) {
					best_score = c_score;
					selected = i;
					printf("Improve remove ride %d %d(%d)\n",i, c_score, best_score);
				}

		//		printf("Improve remove ride %d %d(%d)\n",i, c_score, best_score);
			}
		}
	}

	
	blacklist[nb] = selected;
	if (selected != -1) {
		return recur_solver(best_score, rides, vcs, blacklist, nb+1, oListSorted);
	}
	else {
		return best_score;
	}

}

int permut(T_RIDE * rides, int i, int j) {
	T_RIDE tmp = rides[i];
	rides[i] = rides[j];
	rides[j] = tmp; 	
}

int recur_solver_order(int score, T_RIDE * rides, T_VEHICULE * vcs, T_RIDE * oListSorted, int n) {
	int best_score = score;
	int selected = -1;
	for (int i = n+1;i < gParams.N && i < (n+100000);i++) {
		permut(oListSorted, n, i);
		int c_score = fillRideOrderWithBlacklist(rides, vcs, NULL, 0, oListSorted);
		if (c_score > best_score) {
			best_score = c_score;
			selected = i;
		}
		permut(oListSorted, i, n);
	}
	if (selected == -1 || (n+1 >= gParams.N)) return best_score;
	else {
		permut(oListSorted, n, selected);
		printf("best_score: %d n=%d selected: %d\n",best_score, n, selected);
		//return recur_solver_order(best_score, rides, vcs, oListSorted,n+1);
		return best_score;
	}
}

void solver(T_RIDE * rides, T_VEHICULE * vcs) {
	T_RIDE * oListSorted = (T_RIDE *)malloc(sizeof(T_RIDE)*gParams.N);
	memcpy(oListSorted, rides, sizeof(T_RIDE) * gParams.N);
	qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare);

	fillRideOrderWithBlacklist(rides, vcs, NULL, 0, oListSorted);
	int score = score_vcs(vcs,rides);
	printf("score = %d\n", score);
	int * blacklist = (int *) malloc(sizeof(int)*gParams.F);


	for (int i = 0;i < gParams.F;i++) {
		blacklist[i] = -1;
	}

/*
	int improve_score = recur_solver(score, rides, vcs, blacklist, 0, oListSorted);
	printf("improved score = %d\n", improve_score);

	int depth =  0;
	for (int i = 0;i < gParams.F;i++) {
		if (blacklist[i] != -1) depth++;
	}

	fillRideOrderWithBlacklist(rides, vcs, blacklist, depth, oListSorted);
*/


	int improve_score = score;
	for (int i = 0;i < gParams.N;i++) {
		int improve_score = recur_solver_order(improve_score, rides, vcs, oListSorted,i);
		printf("[%d] improved score = %d\n", i,improve_score);
	}
	
	fillRideOrderWithBlacklist(rides, vcs, blacklist, 0, oListSorted);

}

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

// Solving end here

void stats(T_RIDE * rides, T_VEHICULE * vcs) {
	int rideMissed = 0;
	int rideBonus = 0;
	int rideScore = 0;
	int rideAllLength = 0;
	int vcDeadtime = 0;
	for(int i = 0;i < gParams.N;i++) {
		rides[i].filled = 0;
	}


	for(int i = 0;i < gParams.F;i++) {
		int r = 0;
		int c = 0;
		int t = 0;
		T_VEHICULE v = vcs[i];
		for(int j = 0;j < vcs[i].mvcount;j++) {
			int ri = vcs[i].mv[j].p1;
			rides[ri].filled = 1;
			v.r = r;
			v.c = c;
			v.t = t;
			vcDeadtime += distance_vc_ride(v, rides[ri]);
			rides[ri].filled_at = v.t + distance_vc_ride(v, rides[ri]);
			t = rides[ri].filled_at + rides[ri].length;
			if (rides[ri].latest < t) {
				printf("ERROR on ride: %d vc: %d",ri,i);
			}
			r = rides[ri].ride_to.r;
			c = rides[ri].ride_to.c;
		}
	}
	
	for (int i = 0; i < gParams.N; i++) {
		rideAllLength += rides[i].length;
		if (rides[i].filled == 0) rideMissed++;
		if (rides[i].filled == 1 && rides[i].filled_at == (rides[i].earliest)) rideBonus++;
		if (rides[i].filled == 1) rideScore += distance_ride(rides[i]);
	}

	printf("Rides missed: %d/%d | Rides bonus : %d B=%d | Score=%d/%d | Avg Ride Length=%d/%d | Avg Deadtime=%d\n", rideMissed, gParams.N, rideBonus, gParams.B, rideScore + rideBonus * gParams.B, rideAllLength + gParams.N * gParams.B, rideScore/gParams.N, rideAllLength /gParams.N, vcDeadtime/(gParams.N-rideMissed));
	printf("score: %d\n", rideScore + rideBonus * gParams.B);	
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
	//vsize = (int*)malloc(sizeof(int)*gParams.v);

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
	sprintf(str, "%s.out", filename);
	output(vcs, str);

	//permut(vcs, rides);

	stats(rides, vcs);

	return 0;
}

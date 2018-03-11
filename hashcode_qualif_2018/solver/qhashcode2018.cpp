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
	return distance_vc_ride(vc, ride) + distance_ride(ride);
}

bool is_candidate(T_RIDE ride, T_VEHICULE vc) {
	return (vc.t + computeRideCost(ride, vc)) < ride.latest;
}

void compute_in_range(T_RIDE * rides, T_VEHICULE * vcs) {
	for (int i = 0;i < gParams.N; i++) {
		if (rides[i].filled == 1) continue;
		rides[i].in_range = 0;
		for (int j = 0;j < gParams.F;j++) {
			if (is_candidate(rides[i], vcs[j])) rides[i].in_range++;
		}
	}
}

int count_available(T_VEHICULE vc, T_RIDE * rides) {
	int count = 0;
	for (int i = 0;i < gParams.N;i++) {
		if (is_candidate(rides[i], vc)) count++;
	}
	return count;
}

int selectCar(T_RIDE ride, T_VEHICULE * vcs) {
	int lowest = -1;
	int selected = -1;

	for (int i = 0; i < gParams.F; i++) {
		//int cost = vcs[i].t + computeRideCost(ride, vcs[i]);
		int cost = computeRideCost(ride, vcs[i]) - ((vcs[i].t + distance_vc_ride(vcs[i],ride)) == ride.earliest) ? gParams.B : 0;
		//int cost = vcs[i].t;
		//int cost = count_available(vcs[i], rides);

		//T_VEHICULE vc = vcs[i];
		//if (distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c) >(ride.earliest - vc.t)) cost += distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c) * 2;


/*
		if ((selected == -1 || lowest > cost) && (vcs[i].t + cost <= ride.latest)) {
			selected = i;
			lowest = cost;
		}
*/
		if (is_candidate(ride, vcs[i]) && (selected == -1 || lowest > cost)) {
			selected = i;
			lowest = cost;			
		}
	}

/*
	if ( (selected != -1) && ( (vcs[selected].t + lowest) > ride.latest)) {
		return -1;
	}
*/
	return selected;
}

int min(int a, int b) {
	return 	(a < b) ? a : b;
}

void assignRide(T_VEHICULE * vc, T_RIDE ride, int ri) {

	vc->mv[vc->mvcount].type = RUNNING;
	vc->mv[vc->mvcount].p1 = ri;
	vc->mv[vc->mvcount].p2 = vc->t;
	vc->mvcount++;
	/*
	int d = distance_vc_ride(*vc, ride);

	if (d < (ride.earliest - vc->t)) {
	d = ride.earliest - vc->t;
	}
	*/

	vc->t += distance_vc_ride(*vc, ride) + distance_ride(ride);

	vc->c = ride.ride_to.c;
	vc->r = ride.ride_to.r;
}


int compare(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	return req2->earliest - req1->earliest;
}

int compare_in_range(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	
	
	if ((req1->in_range > 10 && req2->in_range > 10) || (req1->in_range == req2->in_range)) {
		return (req2->latest - req2->length) - (req1->latest - req1->length);
		//return req2->earliest - req1->earliest;
	} else
	
	return req2->in_range - req1->in_range;
}

/*
int compare_length(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	if (req2->earliest == req1->earliest) {
		return req1->earliest - req2->length;
	}
	return req2->earliest - req1->earliest;
}
*/
/*
int compare_d(void const *a, void const *b) {
T_RIDE * req1 = (T_RIDE*)a;
T_RIDE * req2 = (T_RIDE*)b;
return req2->latest - req1->latest;
}
*/
/*
int compare_l(void const *a, void const *b) {
T_RIDE * req1 = (T_RIDE*)a;
T_RIDE * req2 = (T_RIDE*)b;
return req1->latest - req2->latest;
}
*/
int getFirst(T_RIDE * rides) {
	for (int i = 0; i < gParams.N;i++) {
		if (rides[i].filled == 0) return i;
	}
	return -1;
}


void fillRideOrder(T_RIDE * rides, T_VEHICULE * vcs) {
	//T_RIDE * oListSorted = (T_RIDE *)malloc(sizeof(T_RIDE)*gParams.N);
	//memcpy(oListSorted, rides, sizeof(T_RIDE) * gParams.N);
	//qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare);


	int count = 0;


	for (int i = 0; i < gParams.N; i++) {


		int selected = -1;
		compute_in_range(rides, vcs);
		while ((selected = getFirst(rides)) != -1) {

			
			//qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare_in_range);
			T_RIDE rideSelect = rides[selected];
			for (int j = selected+1;j < gParams.N;j++) {
				if (rides[j].filled == 0 && compare_in_range(&rideSelect, &rides[j]) < 0) {
					rideSelect = rides[j];
				}
			}


			int vci = selectCar(rideSelect, vcs);

			if (vci != -1) {
				printf("vc: %d ride: %d  [%d/%d]\n", vci, rideSelect.idx,count++,gParams.N);
				assignRide(&vcs[vci], rideSelect, rideSelect.idx);
				rides[rideSelect.idx].filled = 1;
				printf("ride filled:  %d in_range: %d\n", rideSelect.idx, rideSelect.in_range);
				//getchar();
			} else {
				printf("Failed ride: %d latest start: %d (%d,%d)\n",rideSelect.idx, rideSelect.latest-rideSelect.length, rideSelect.ride_from.c,rideSelect.ride_from.r );
				rides[rideSelect.idx].filled = 1;
				//getchar();
			}
			compute_in_range(rides, vcs);
		}
	}
}

void solver(T_RIDE * rides, T_VEHICULE * vcs) {
	fillRideOrder(rides, vcs);
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

void permut(T_VEHICULE * vcs, T_RIDE * rides) {

	int vc = rand()%gParams.F;

	int mv_split = 1 + rand()%(vcs[vc].mvcount - 1);

	T_RIDE ride = rides[vcs[vc].mv[mv_split].p1];

	T_RIDE prev_ride = rides[vcs[vc].mv[mv_split-1].p1];

	int d1 = distance(ride.ride_from.r,ride.ride_from.c, prev_ride.ride_to.r, prev_ride.ride_to.c);

	for (int i = 0;i < gParams.F;i++) {

		if (i == vc) continue;

		for (int j = 0;j < vcs[i].mvcount;j++) {
			T_RIDE ride_candidate = rides[vcs[i].mv[j].p1];
			if (vcs[i].mv[j].p2 >= vcs[vc].mv[mv_split-1].p2) {
				int d2 = distance(ride.ride_from.r,ride.ride_from.c, ride_candidate.ride_to.r, ride_candidate.ride_to.c);
				if (d2 < d1) {
					printf("Permutation candidate found ! \n");
				}
			}
		}
	}
}

// Solving end here


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
		score += rides[ri].length + (vc.t + distance_vc_ride(vc, rides[ri]) == rides[ri].earliest) ? gParams.B : 0; 
		t = vc.t + distance_vc_ride(vc, rides[ri])+ rides[ri].length;
		if (rides[ri].latest < t) {
			return -1;
		}

		
		r = rides[ri].ride_to.r;
		c = rides[ri].ride_to.c;
	}	

	return score;
}


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


	for (int i = 0;i < gParams.N;i++) {
		if (rides[i].filled == 0) {
			for(int k = 0;k < gParams.F;k++) {

				T_VEHICULE v = vcs[k];
				int score = score_vc(v, rides);
				for(int j = 0;j < vcs[k].mvcount;j++) {
					int tmp = vcs[k].mv[j].p1;
					vcs[k].mv[j].p1 = i;
					if (score_vc(v, rides) > score) {
						printf("score improve ! \n");
					}
					vcs[k].mv[j].p1 = tmp;
				}

			}			
		}
	}


	//printf("ride 0: %d ride 1: %d\n", rides[0].earliest, rides[1].earliest);
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

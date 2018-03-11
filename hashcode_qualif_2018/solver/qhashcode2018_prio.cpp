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
	int length;
	int filled_at;
	int latest_start;
	int priority;
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
	return distance_ride(ride) +  distance_vc_ride(vc, ride);
}

int count_rides_for_car(T_RIDE * rides, T_VEHICULE vc) {
	int count = 0;
	for (int i = 0; i < gParams.N; i++) {
		if (rides->filled == 1) continue;
		if (vc.t + distance_vc_ride(vc, rides[i]) < rides[i].latest_start) {
			count++;
		}
	}
	return count;
}

int count_rides_in_range(T_RIDE * rides, T_VEHICULE vc, int range) {
	int count = 0;
	for (int i = 0; i < gParams.N; i++) {
		if (rides->filled == 1) continue;
		if (distance(vc.r, vc.c, rides[i].ride_from.r, rides[i].ride_from.c) < range) {
			
			count++;
		}
	}
	return count;	
}

bool is_candidate(T_RIDE ride, T_VEHICULE vc) {
	return (vc.t + distance_vc_ride(vc, ride) + ride.length) < ride.latest;
}

bool is_candidate_restricted(T_RIDE ride, T_VEHICULE vc) {
	return (vc.t + distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c) ) <= ride.earliest;
}


void assignRide(T_VEHICULE * vc, T_RIDE ride, int ri) ;
int selectCar(T_RIDE ride, T_RIDE * rides, T_VEHICULE * vcs, int onlyBonus) {
	int lowest = -1;
	int selected = -1;
	static int * vcs_candidate = (int*)malloc(sizeof(int) * gParams.F);
	int vcs_c_count = 0;
	for (int i = 0; i < gParams.F; i++) {

		if (onlyBonus && (vcs[i].t  + distance_vc_ride(vcs[i], ride) > ride.earliest)) {
			continue;
		}

		if (vcs[i].t + distance_vc_ride(vcs[i], ride) + ride.length < ride.latest) {
			vcs_candidate[vcs_c_count] = i;
			vcs_c_count++;
		}

	}


	if (vcs_c_count == 0) {
		
		for(int i = 0;i < gParams.F;i++) {
			//printf("v[%d] t=%d m=%d (%d,%d)(%d)\n",i, vcs[i].t, vcs[i].mvcount, vcs[i].c, vcs[i].r,(vcs[i].t + distance_vc_ride(vcs[i], ride)));
		}
		//printf("Ride: %d has no candiate. [%d,%d][%d]\n", ride.idx, ride.ride_from.c, ride.ride_to.r, ride.latest_start);
		//getchar();
	}

	for (int i = 0; i < vcs_c_count; i++) {
		
		T_VEHICULE vc = vcs[vcs_candidate[i]];
		
		/*
		T_VEHICULE vc_after;
		vc_after.c = ride.ride_to.c;
		vc_after.r = ride.ride_to.r;
		vc_after.t = vc.t + computeRideCost(ride, vc);
		int cost = count_rides_for_car(rides, vc_after) - count_rides_for_car(rides, vc);
		*/
		//int cost = count_rides_for_car(rides, vc);
		//int cost = count_rides_in_range(rides, vc, 500);
		//int cost = vcs[i].t + computeRideCost(ride, vc);
		
		
		int cost = computeRideCost(ride, vc);

		//int cost = abs(vcs[i].t - ride.latest) +  distance_vc_ride(vc, ride);

		// Closest from start
		//int cost = abs(ride.earliest - (vc.t + distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c)));
	
		// Most busy
		//int cost = abs(ride.latest_start - (vc.t + distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c)));

		//int cost = vc.t;
		//int cost = vc.mvcount;

		//int cost = distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c) / (vc.t+1);
		//int cost = abs(ride.latest_start - vc.t);

		// Least physical distance
		//int cost = distance(vc.r, vc.c, ride.ride_from.r, ride.ride_from.c);

		if (selected == -1 || lowest >  cost) {
			selected = vcs_candidate[i];
			lowest = cost;
		}
	}

	/*
	static int drop_counter = 0;
	if (distance_ride(ride) > 15000) {
		printf("ride dropped: %d %d\n", ride.idx, ++drop_counter);
		return -1;
	}
	*/


	if (selected != -1 && vcs[selected].t + distance_vc_ride(vcs[selected],ride) + ride.length > ride.latest) {
		puts("ERROR");
		return -1;
	}
	
	//printf("nb candidates for ride #%d: %d lowest:%d\n", ride.idx,vcs_c_count, lowest);

/*
	static int opportunityRides = 0;
	if (selected != -1) {
		for (int i = 0;i < gParams.N;i++) {
			if ( (rides[i].idx != ride.idx) && (rides[i].filled == 0) && (rides[i].earliest < ride.latest_start) ) {
				if (is_candidate(rides[i], vcs[selected])) {
					T_VEHICULE vc_after;
					vc_after.c = rides[i].ride_to.c;
					vc_after.r = rides[i].ride_to.r;
					vc_after.t = vcs[selected].t + computeRideCost(rides[i], vcs[selected]);
					if (is_candidate_restricted(ride, vc_after)) {
						opportunityRides++;
						//printf("[before: %d]\n",vcs[selected].t + distance_vc_ride(vcs[selected], ride) + distance_ride(ride));
						//printf("vc: %d ride: %d target ride -> %d [%d]\n",selected,i, ride.idx, opportunityRides);
						assignRide(&vcs[selected], rides[i], rides[i].idx);
						//printf("[after: %d]\n",vcs[selected].t + distance_vc_ride(vcs[selected], ride) + distance_ride(ride));
						rides[i].filled = 1;
					}
				}
			}
		}
 	}
*/
 
	static int legitRides = 0;
	if (selected != -1) {
		legitRides++;
		/*
		printf("ride %d [%d] [%d] score[%d]\n",ride.length, legitRides,opportunityRides, lowest);
		if (lowest == 0 && vcs[selected].mvcount > 0) {
			printf("vs:(%d,%d [%d])\n", vcs[selected].c,vcs[selected].r,count_rides_in_range(rides, vcs[selected],2000));
		}
		*/
	}

	return selected;
}

int min(int a, int b) {
	return 	(a < b) ? a : b;
}

void assignRide(T_VEHICULE * vc, T_RIDE ride, int ri) {

	vc->mv[vc->mvcount].type = RUNNING;
	vc->mv[vc->mvcount].p1 = ri;
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
	return req1->earliest - req2->earliest;
}

int compare_priority(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	return req2->priority - req1->priority;
}


int compare_ls(void const *a, void const *b) {
		//int cost = abs(vcs[i].t - ride.latest) +  
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	// upward
	return req1->latest_start - req2->latest_start;
}

/*
int compare(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	// upward
	return req2->latest - req1->latest;
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

int compute_priority(T_RIDE * rides) {
	int max = 0;
	int max_length = 0;
	for (int i = 0;i < gParams.N;i++) {
		if (max < rides[i].latest_start) max = rides[i].latest_start;
		if (max_length < rides[i].length) max_length = rides[i].length;
	}

	int range = 100000;
	for (int i = 0;i < gParams.N;i++) {
		int latest_start = (max - rides[i].latest_start) * range / max;
		int length = (rides[i].length) * range / max_length;
		//length = 0; 
		rides[i].priority = latest_start + length / 4; 
	}
} 


int compare_ride_length(void const *a, void const *b) {
	T_RIDE * req1 = (T_RIDE*)a;
	T_RIDE * req2 = (T_RIDE*)b;
	return req2->length - req1->length;
}


void firstDispatch(T_RIDE * ridesSorted, T_RIDE * rides, T_VEHICULE * vcs) {
	int rideCount = 0;
	for (int i = 0; i < gParams.F; i++) {
		assignRide(&vcs[i], ridesSorted[rideCount], ridesSorted[rideCount].idx);
		rides[ridesSorted[rideCount].idx].filled = 1;
		rideCount++;
		
	}
}

void fillRideOrder(T_RIDE * rides, T_VEHICULE * vcs) {


	T_RIDE * oListSorted = (T_RIDE *)malloc(sizeof(T_RIDE)*gParams.N);
	memcpy(oListSorted, rides, sizeof(T_RIDE) * gParams.N);
	qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare_ls);
	/*
	for (int i = 0; i < gParams.N; i++) {
		if (rides[i].length > 15000) {
			rides[i].filled = 1;
		}

		if (rides[i].length < 200) {
			rides[i].filled = 1;
		}

	}
	*/



//	firstDispatch(oListSorted, rides, vcs);

	for (int i = 0; i < gParams.N; i++) {
		if (rides[oListSorted[i].idx].filled == 1) continue;

		int vci = selectCar(oListSorted[i], rides, vcs, 0);

		if (vci != -1) {
			//printf("vc: %d ride: %d \n", vci, i);
			rides[oListSorted[i].idx].filled_at = vcs[vci].t + distance_vc_ride(vcs[vci], oListSorted[i]);
			assignRide(&vcs[vci], oListSorted[i], oListSorted[i].idx);
			rides[oListSorted[i].idx].filled = 1;
		}
	}
	

	//firstDispatch(oListSorted, rides, vcs);
	
	
	int bonus_count = 0;
	for (int i = 0; i < gParams.N; i++) {
		if (rides[i].filled) bonus_count++;
	}
	printf("Dispatched for bonus: %d\n", bonus_count);
	

	qsort(oListSorted, gParams.N, sizeof(T_RIDE), compare_ls);
	
	for (int i = 0; i < gParams.N; i++) {
		if (rides[oListSorted[i].idx].filled == 1) continue;

		int vci = selectCar(oListSorted[i], rides, vcs, 0);

		if (vci != -1) {
			//printf("vc: %d ride: %d \n", vci, i);
			rides[oListSorted[i].idx].filled_at = vcs[vci].t + distance_vc_ride(vcs[vci], oListSorted[i]);
			assignRide(&vcs[vci], oListSorted[i], oListSorted[i].idx);
			rides[oListSorted[i].idx].filled = 1;
		}
	}
	
	//printf("ride 0: %d ride 1: %d\n", oListSorted[0].length, oListSorted[1].length);
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

// Solving end here
void stats(T_RIDE * rides, T_VEHICULE * vcs) {
	int rideMissed = 0;
	int rideBonus = 0;
	int rideScore = 0;
	int rideAllLength = 0;
	
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
		if (rides[i].filled_at == (rides[i].earliest)) rideBonus++;
		if (rides[i].filled == 1) rideScore += distance_ride(rides[i]);
	}

	//printf("ride 0: %d ride 1: %d\n", rides[0].earliest, rides[1].earliest);
	printf("Rides missed: %d/%d | Rides bonus : %d B=%d | Score=%d/%d | Avg Ride Length=%d/%d \n", rideMissed, gParams.N, rideBonus, gParams.B, rideScore + rideBonus * gParams.B, rideAllLength + gParams.N * gParams.B, rideScore/gParams.N, rideAllLength /gParams.N);
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
		rides[i].filled_at = 0;
		rides[i].latest_start = rides[i].latest - rides[i].length;
	}
	compute_priority(rides);

	solver(rides, vcs);
	char filenamecat[256];
	char str[255];
	sprintf(str, "%s.out", filename);

	

	int matching = 0;
	for(int i = 0;i < gParams.N;i++) {
		for (int j = 0;j < gParams.N;j++) {
			if (i != j) {
				if (rides[i].priority == rides[j].priority) {
					matching++;
				}
			}
		}
	}
	printf("Matching sort: %d\n",matching);

	
	output(vcs, str);

	stats(rides, vcs);

	return 0;
}

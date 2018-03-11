#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
 int v;
 int e;
 int r;
 int c;
 int x;
} T_PARAMS;

T_PARAMS g_params;

typedef struct {
  int cid;
  int latency;
} T_CONNECTION;

typedef struct {
  int latency;
  int concount;
  T_CONNECTION * conlist;
} T_ENDPOINT;


typedef struct {
  int vid;
  int endid;
  int nreq;
  int vsize;
  char served;
} T_REQUEST;


typedef struct {
  int vcount;
  int * vlist;
  int free;
} T_CACHE;

int * vsize;

T_CACHE * init_cache(T_PARAMS params) {
	T_CACHE * cachelist = (T_CACHE*)malloc(sizeof(T_CACHE)*params.c);
	for (int i = 0;i < params.c;i++) {
		cachelist[i].vcount = 0;
		cachelist[i].vlist = (int *)malloc(sizeof(int)*params.v);
		cachelist[i].free = params.x;
	}
	return cachelist;
}

int add_video(T_CACHE * cachelist,int i, int vid,int w) {
	if (cachelist[i].free < w) return -1;
	cachelist[i].vlist[cachelist[i].vcount] = vid;
	cachelist[i].vcount++;
	cachelist[i].free -= w;
	return 0;
}


int read_int(FILE * f) {
	char str[10];
	int c = fgetc(f);
	int count = 0;
	while (c != EOF && c != ' ' && c!= '\n') {
		str[count] = c; 
		count++;
		c = fgetc(f);
	}
	str[count] = 0;
	return atoi(str);
}

int compare(void const *a, void const *b) {
	T_REQUEST * req1 = (T_REQUEST*)a;
	T_REQUEST * req2 = (T_REQUEST*)b;
	return req2->nreq - req1->nreq;
}

int cache_contains(T_CACHE cache, int vid) {
	for (int i =0;i < cache.vcount;i++) {
		if (cache.vlist[i] == vid) return 1;
	}
	return 0;
}

double score_cache(T_REQUEST * reqlist, T_ENDPOINT * endpointlist, T_CACHE * cachelist,int vid) {
	double sc = 0;
	for (int i = 0;i < g_params.r;i++) {
		if (vid == reqlist[i].vid) {
			unsigned int l = endpointlist[reqlist[i].endid].latency;

		
			for (int j = 0;j < endpointlist[reqlist[i].endid].concount;j++) {
			
				T_CONNECTION con = endpointlist[reqlist[i].endid].conlist[j];
			
				for (int k = 0;k < cachelist[con.cid].vcount;k++) {
				
					if (cachelist[con.cid].vlist[k] == reqlist[i].vid) {
						if (l > con.latency) {
							l = con.latency;
						}
					}
				}	
			}
		
			sc += (endpointlist[reqlist[i].endid].latency - l) * reqlist[i].nreq;
		}
	}

	return sc;
}

double delta_score_cache(T_REQUEST * reqlist, T_ENDPOINT * endpointlist, T_CACHE * cachelist,int cid,int vid, double ori_sc) {	
	cachelist[cid].vlist[cachelist[cid].vcount] = vid;
	cachelist[cid].vcount++;
	double sc = score_cache(reqlist,endpointlist,cachelist,vid) - ori_sc;
	cachelist[cid].vcount--;
	return sc;
}

void propagate(T_REQUEST * reqlist,T_ENDPOINT * endpointlist,int cid, int vid) {
	for (int i = 0;i < g_params.r;i++) {
		if (reqlist[i].vid == vid) {
			T_ENDPOINT endp = endpointlist[reqlist[i].endid];
			for (int j = 0;j < endp.concount;j++) {
				if (endp.conlist[j].cid == cid) {
					reqlist[i].served = 1;
				}
			}
		}
	}
}

void solverR(T_REQUEST * reqlist, T_ENDPOINT * endpointlist, T_CACHE * cachelist, int * vsize, int (*compare) (void const *a, void const *b)) {
	qsort(reqlist,g_params.r,sizeof(T_REQUEST),compare);
	for (int i=0;i<g_params.r;i++) {
		if (reqlist[i].served == 1) 
			continue;

		double score = 0;
		int selec_c = 0;
		int vid = reqlist[i].vid;
		printf("Analyze request %d/%d nb cache=%d score=%.0f\n",i,g_params.r,g_params.c,score);
		double ori_sc = score_cache(reqlist,endpointlist,cachelist,i);
		for (int k = 0;k < g_params.c;k++) {
			if ((cachelist[k].free > vsize[vid]) && !cache_contains(cachelist[k],vid)) {
				double curr_score = delta_score_cache(reqlist, endpointlist, cachelist,k,vid,ori_sc);
				if (curr_score > score) {
					score = curr_score;
					selec_c = k;
				}				
			}
		}
		if (score != 0) {
			add_video(cachelist,selec_c,vid,vsize[vid]);
			propagate(reqlist,endpointlist,selec_c,vid);
		}
	}
}


unsigned int score_simple(T_REQUEST * reqlist, T_ENDPOINT * endpointlist, T_CACHE * cachelist) {
	double sc = 0;
	double totnr = 0;
	for (int i = 0;i < g_params.r;i++) {
		unsigned int l = endpointlist[reqlist[i].endid].latency;

		
		for (int j = 0;j < endpointlist[reqlist[i].endid].concount;j++) {
			
			T_CONNECTION con = endpointlist[reqlist[i].endid].conlist[j];
			
			for (int k = 0;k < cachelist[con.cid].vcount;k++) {
				
				if (cachelist[con.cid].vlist[k] == reqlist[i].vid) {
					if (l > con.latency) {
						l = con.latency;
					}
				}
			}	
		}
		
		sc += (endpointlist[reqlist[i].endid].latency - l) * reqlist[i].nreq;
		totnr += reqlist[i].nreq;
	}

	return (sc * 1000.) / totnr ;
}



void output_solution(T_PARAMS g_params,T_CACHE * cachelist, char * filename) {
	FILE * f = fopen(filename,"w");

	int count = 0;
	for (int i = 0;i < g_params.c;i++) {
		if (cachelist[i].vcount > 0) count++;
	}

	fprintf(f,"%d\n",count);

	for (int i = 0;i < g_params.c;i++) {
		if (cachelist[i].vcount > 0) {
			fprintf(f,"%d",i+1);
			for (int j = 0;j < cachelist[i].vcount;j++) {
				fprintf(f," %d",cachelist[i].vlist[j]);
			}
			fprintf(f,"\n");
		}
	}

	fclose(f);
}


int main(int argc, char **argv) {
	FILE * f = fopen(argv[1],"r");
	
	fscanf(f,"%d %d %d %d %d\n",&g_params.v,&g_params.e,&g_params.r,&g_params.c,&g_params.x);

	vsize = (int*)malloc(sizeof(int)*g_params.v);
	
	for (int i = 0;i < g_params.v;i++) {
		vsize[i] = read_int(f);	
		printf("video %d : %d\n",i,vsize[i]);
	}

	T_ENDPOINT * endpointlist = (T_ENDPOINT*)malloc(sizeof(T_ENDPOINT)*g_params.e);

	for (int i = 0;i < g_params.e;i++) {
		fscanf(f,"%d %d\n",&endpointlist[i].latency,&endpointlist[i].concount);
		endpointlist[i].conlist = (T_CONNECTION*)malloc(sizeof(T_CONNECTION)*endpointlist[i].concount);
		for (int j = 0;j < endpointlist[i].concount;j++) {
			fscanf(f,"%d %d\n",&endpointlist[i].conlist[j].cid,&endpointlist[i].conlist[j].latency);
		}
	}

	T_REQUEST * reqlist = (T_REQUEST*)malloc(sizeof(T_REQUEST)*g_params.r);

	for (int i = 0;i < g_params.r;i++) {
		fscanf(f,"%d %d %d\n",&reqlist[i].vid,&reqlist[i].endid,&reqlist[i].nreq);
		reqlist[i].vsize = vsize[reqlist[i].vid];
		reqlist[i].served = 0;
	}
	fclose(f);


	T_CACHE * cachelist = init_cache(g_params);

	solverR(reqlist,endpointlist,cachelist,vsize,&compare);
	for (int i = 0;i < g_params.r;i++) {
		reqlist[i].served = 0;
	}

	printf("SOLVED\n");
	unsigned int score = score_simple(reqlist,endpointlist,cachelist);
	printf("Score: %d\n",score);
	
	char str[255];
	sprintf(str,"%s.%d.txt",argv[1],score);
	output_solution(g_params,cachelist,str);
	return 0;
}
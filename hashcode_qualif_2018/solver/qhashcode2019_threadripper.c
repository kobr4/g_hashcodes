
// MULTITHREADED SOLVER GOOGLE HASHCODE 2019 - AMD EDITION
// RUN ON AMD THREADRIPPER 1950X FOR COMPETITION
// BY MICHEL HE, NICOLAS MY, OLIVIER GERVOT - MAR. 2019

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
// PARAMETERS
#define false 0
#define true 1
#define TYPE_H 0
#define TYPE_V 1
#define TOTAL_PHOTOS 100000  // actually only 90000 in google hashcode 2019 in d_pet_pictures.txt
#define MAX_ELEM_IN_PHOTO 60 // xx elements / photo <- there's a line of 51 element in b set
#define UNDEF_VALUE 100010001 // some very big arbitrary value
#define MULTIPLIER (36) // used in hash(), alpha min + digit = 36 combinations, be sure that 36^6 = 2.1e9 < 2^32 = 4.2e9
#define MAX_CARDINAL 6 // max number of letters of an element
#define MAX_PAIR_LEVEL 1000 // 1 is fastest = only one candidates

// DEBUG MESSAGES
#define DEBUG_LEVEL_0
#define DEBUG_LEVEL_1
#define DEBUG_LEVEL_2 printf

#define MAX_CLIENTS 32 // max number of threads
#define DATASET_GRANULARITY 2500 // how many data for each thread

#define SAVE_FILE_OUTPUT 1

struct photo_container {    
    unsigned long linebuf[MAX_ELEM_IN_PHOTO]; //MAX_ELEM_IN_PHOTO elements in photo
};

// The slide show contains each photo with left and right neighbors
struct slide_container {
    struct slide_container *next;
    unsigned long this_photo;
};

struct th_pdata {
    int k;
    int nb;
    int from_idx;
};

/***************
global variables
***************/
struct photo_container *photo;
// number of photo
int nbelem;
// number of elements in a photo
int *photo_elem;
// boolean to know which photo is sorted
char *photo_sorted;
// photo orientation is H or V
char *photo_orient;
// candidates who have potential to match
int *photo_matched;
// photo list, maybe empty
struct slide_container **photo_list;
// counter of total number of lines in output.txt
int n_global_slides = 0;
//table for scoring in multithread context
int *score_table;
int NB_CLIENTS;
int COPIES_BY_THREAD;

typedef struct
{
    int stock;

    pthread_t thread_store;
    pthread_t *thread_clients;
    struct th_pdata thread_pdata[MAX_CLIENTS];   //MAX_CLIENTS Thread

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

/***************
   Functions
***************/
// hash allows fast comparison
unsigned long hash(const char *s)
{
   int cardinal = 0;
   unsigned long h;
   unsigned const char *us;

   us = (unsigned const char *) s;

   h = 0;
   while(*us != '\0') {
       char ch = *us;
       if ((ch>='0') && (ch<='9')) ch-='0';
       else if ((ch>='a') && (ch<='z')) ch=ch-'a'+10;
       else { printf("Error hash out of bound\n"); }
       h = h * MULTIPLIER + ch;
       us++;
    cardinal++;
    if (cardinal > MAX_CARDINAL) printf("ERR: hash(%s) overflows\n", s);
   }
   return h;
}

void load_photo(char * filename)
{
    char HorizVerti[2]; // H, or V
    char linebuf[MAX_ELEM_IN_PHOTO][12]; //MAX_ELEM_IN_PHOTO elements in photo currently 30 max found in b, 12 is element string max
    int lineelem;

    for(int ll=0; ll<TOTAL_PHOTOS; ll++){
        photo_sorted[ll] = false;
    }

    for(int ll=0; ll<TOTAL_PHOTOS; ll++){
        photo_matched[ll] = 0;
    }

    printf("loading %s...\n", filename);
    FILE *f=fopen(filename,"r");
    if (f==NULL)
    {
        perror("fopen error\n");
        exit(1);
    }
    fscanf(f, "%d\n", &nbelem);
    printf("Load %d photos\n", nbelem);

    if (nbelem>TOTAL_PHOTOS) {
            printf("Error excess nbelem=%d\n", nbelem);
            nbelem = TOTAL_PHOTOS;
    }

    for (int k=0; k<nbelem; k++) {
        fscanf(f, "%s %d ", HorizVerti, &lineelem);
        DEBUG_LEVEL_0("%s %d\n", HorizVerti, lineelem);
        DEBUG_LEVEL_0("photo %d\n", k);
        if (strcmp(HorizVerti, "H")==0) {
            photo_orient[k] = TYPE_H;
        } else {
            photo_orient[k] = TYPE_V;
        }
        if (lineelem>MAX_ELEM_IN_PHOTO)
        {
            printf("Error excess lineelem=%d\n", lineelem);
            lineelem = MAX_ELEM_IN_PHOTO;
        }
        photo_elem[k] = lineelem;

        for (int j=0; j<lineelem; j++) {
            fscanf(f, "%s ", linebuf[j]);
            unsigned long elem_hash = hash(&linebuf[j]);
            photo[k].linebuf[j] = elem_hash;
            DEBUG_LEVEL_0("%d\n", elem_hash);
        }

    }

    fclose(f);
    printf("all lines read\n");
}

int min_val (int a, int b)
{
    if (a<b)
        return a;
    else
        return b;
}

int max_val (int a, int b)
{
    if (a<b)
        return b;
    else
        return a;
}


int score_pair(int p1, int p2)
{
    int same_elm = 0;
    int diff_elm;

    for (int em = 0; (em < photo_elem[p1]); em++)
    {
        for (int p = 0; (p < photo_elem[p2]); p++) {
            if (photo[p1].linebuf[em] == photo[p2].linebuf[p])
            {
                same_elm++;
                break;
            }
        }
    }

    diff_elm = min_val(photo_elem[p1], photo_elem[p2]) - same_elm;

    return min_val(same_elm, diff_elm);
}

static void * fn_clients (void * fn_args)
{
    struct th_pdata *p_data = (struct th_pdata *)fn_args;
    int k = p_data->k;
    int nb = p_data->nb;
    int from_idx = p_data->from_idx;
    int i;
    int v;
    /*
    pthread_mutex_lock(&store.mutex_clients);
    printf("RUN THREAD with k=%d nb=%d from_idx=%d\n", k, nb, from_idx);
    pthread_mutex_unlock(&store.mutex_clients);
    */
    if ((nb+1)*COPIES_BY_THREAD < from_idx) return NULL;

    for (v = nb*COPIES_BY_THREAD; v < (nb+1)*COPIES_BY_THREAD; v++) {
        if (v<from_idx) continue;
        if (photo_sorted[v] == true)
        {
            DEBUG_LEVEL_1("v = %d already sorted\n", v);
            score_table[v] = 0;
            continue;
        }
        if (photo_orient[v] != photo_orient[k])
        {
            DEBUG_LEVEL_1("k=%d and v=%d differs orient ** drop\n", k, v);
            score_table[v] = 0;
            continue;
        }
        if (v==k) {
            DEBUG_LEVEL_1("same photo\n", v);
            score_table[v] = 0;
            continue;
        }
        score_table[v] = score_pair(k, v);
    }

    // the last segment may overshoot
	if ((nb+1)==NB_CLIENTS) {
    for(v = (nb+1)*COPIES_BY_THREAD; v < nbelem; v++) {
        if (v<from_idx) continue;
        if (photo_sorted[v] == true)
        {
            DEBUG_LEVEL_1("v = %d already sorted\n", v);
            score_table[v] = 0;
            continue;
        }
        if (photo_orient[v] != photo_orient[k])
        {
            DEBUG_LEVEL_1("k=%d and v=%d differs orient ** drop\n", k, v);
            score_table[v] = 0;
            continue;
        }
        if (v==k) {
            DEBUG_LEVEL_1("same photo\n", v);
            score_table[v] = 0;
            continue;
        }
        score_table[v] = score_pair(k, v);
        }
    }


    return NULL;
}

int match(int k, int from_idx)
{
    int found_pair_level = 0;
    int current_score = 0;
    int v;
    int elect_em;
    int elect_photo = UNDEF_VALUE;
    struct th_pdata *pdata = malloc(sizeof(struct th_pdata));

    for (int i = 0; i < NB_CLIENTS; i++)
    {
        store.thread_pdata[i].k = k;
        store.thread_pdata[i].nb = i;
        store.thread_pdata[i].from_idx = from_idx;
        int ret = pthread_create (
                  &store.thread_clients[i], NULL,
                  fn_clients, (void *) &store.thread_pdata[i]);

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

    int s;
    current_score = 1;
    for (v=from_idx; v<nbelem; v++)
    {
        s = score_table[v];
        if (current_score <= s) {
            current_score = s;
            found_pair_level++;
            elect_photo = v;
        }
        if (found_pair_level>=MAX_PAIR_LEVEL) {
            photo_matched[k] = found_pair_level;
            return v; //return asap
        }
    }
    DEBUG_LEVEL_1("select %d for photo = %d\n", elect_photo, k);
    photo_matched[k] = found_pair_level;
    return elect_photo;
}

void drop_photo(int k)
{
    photo_sorted[k] = false;
}

int sort_photo()
{
    int found_pair_level; //depth of level
    int elect = 0;
    int photo_idx = 0;
    int n_lists = 0;
    int ensemble_photo = 0;
    struct slide_container *p_list;
    DEBUG_LEVEL_2("+ photo_idx 0, new list %d\n", n_lists);
    p_list = malloc(sizeof(struct slide_container));
    photo_list[n_lists] = p_list;
    // PAIR MATCHING
    for (int k = 0; k < nbelem; ) {
        DEBUG_LEVEL_1("search to pair photo %d\n", k);

        //search one of the elements in photo k among photo v = complexity O(n) = nbelem^2
        elect = match(k, photo_idx);

        if (elect == UNDEF_VALUE) {
            if ((photo_orient[k]==TYPE_V) && (ensemble_photo%2)) {
                printf("drop single photo %d\n", k);
                //Google Error: Vertical photos must be used in pairs. Found single then drop
                drop_photo(k);
                p_list->this_photo=UNDEF_VALUE;
            }
            p_list->next = NULL;

            photo_idx++;
            while ((photo_sorted[photo_idx] == true) && (photo_idx<nbelem)) photo_idx++;
            k = photo_idx;
            //set new list
            n_lists++;
            DEBUG_LEVEL_2("+ photo_idx %d, new list %d\n", k, n_lists);
            p_list = malloc(sizeof(struct slide_container));
            photo_list[n_lists] = p_list;
            p_list->this_photo = k;
            ensemble_photo = 1;
        }
        else {
            DEBUG_LEVEL_1("best match elect = %d, for photo = %d\n", elect, k);
            //int d = score_pair(elect, k);
            //DEBUG_LEVEL_1("score %d & %d = %d\n", k, elect, d);

            photo_sorted[k] = true;
            photo_sorted[elect] = true;
            p_list->this_photo = k;
            p_list->next = malloc(sizeof(struct slide_container));
            p_list = p_list->next;
            p_list->this_photo = elect;
            k = elect; //go to next life
            ensemble_photo++;
        }
    }

    DEBUG_LEVEL_2("have found %d lists\n", n_lists);
    return n_lists;
}

int show_list_H(FILE *fo, struct slide_container *p_list)
{
    int score = 0;
    int pair_show = 0;
    int pv = p_list->this_photo;
    while ((p_list->next!=NULL) && (p_list->this_photo<nbelem)) {

        fprintf(fo,"%d\n", p_list->this_photo);
        n_global_slides++;
        score += score_pair(p_list->this_photo, pv);
        pv = p_list->this_photo;

        p_list = p_list->next;
        pair_show++;
    }
    if (p_list->this_photo!=UNDEF_VALUE) {
        fprintf(fo,"%d\n", p_list->this_photo);
        n_global_slides++;
        score += score_pair(p_list->this_photo, pv);
    }
    return score;
}

int show_list_V(FILE *fo, struct slide_container *p_list)
{
    int score = 0;
    int pair_show = 1;
    int pv = p_list->this_photo;
    while (p_list->next!=NULL) {
        p_list = p_list->next;
        if (pair_show%2) {
            if (p_list->this_photo!=UNDEF_VALUE) {
                fprintf(fo,"%d %d\n", pv, p_list->this_photo);
                score += score_pair(p_list->this_photo, pv);
                n_global_slides++;
            }
        }
        pv = p_list->this_photo;
        pair_show++;
    }
    return score;
}

void output(char * filename, int n_photoslide)
{
    int total_score = 0;

    #ifdef SAVE_FILE_OUTPUT
        FILE *fo = fopen(filename, "w");
    #else
        FILE *fo = stdout;
    #endif

    int photo_idx = 0;

    struct slide_container *p_list;

    printf("save results to %s...\n", filename);

    for (int j=0; j < n_photoslide; j++)
    {
        p_list = photo_list[j];
        if (p_list->next==NULL) {
            //fprintf(fo,"only one item : %d\n", p_list->this_photo);
            //fprintf(fo,"%d\n", p_list->this_photo);
        }
        else {
            if (photo_orient[p_list->this_photo]==TYPE_H) {
                    //fprintf(fo,"--\n");
                    DEBUG_LEVEL_1("*** show_list_H %d\n", j);
                    total_score += show_list_H(fo, p_list);
            }
            else {
                    //fprintf(fo,"--\n");
                    DEBUG_LEVEL_1("*** show_list_V %d\n", j);
                    total_score += show_list_V(fo, p_list);
            }
        }
    }

    #ifdef SAVE_FILE_OUTPUT
        fclose(fo);
    #endif

    printf("total score %d\n", total_score);
}


/***************
      MAIN
***************/
int main(int argc, char **argv)
{
    char result_name[128];
    //measure of time of the algo
    struct timeval tv1, tv2;
    struct timezone tz1, tz2;

	//Init.
    photo = malloc(sizeof(struct photo_container)*TOTAL_PHOTOS); //max TOTAL_PHOTOS photos
    photo_elem = malloc(sizeof(int)*TOTAL_PHOTOS);
    photo_sorted = malloc(sizeof(char) *TOTAL_PHOTOS);
    photo_orient = malloc(sizeof(char) *TOTAL_PHOTOS);
    photo_matched = malloc(sizeof(int)*TOTAL_PHOTOS);
    photo_list = malloc(sizeof(struct slide_container *)*TOTAL_PHOTOS);
    score_table = malloc(sizeof(int)*TOTAL_PHOTOS);


    gettimeofday(&tv1, &tz1);

    if (argc<2)
    {
        //load_photo("C:\\TEMP\\h19\\bin\\Debug\\a_example.txt");
        //load_photo("C:\\TEMP\\h19\\bin\\Debug\\b_lovely_landscapes.txt");
        //load_photo("C:\\TEMP\\h19\\bin\\Debug\\c_memorable_moments.txt");
        load_photo("C:\\TEMP\\h19\\bin\\Debug\\d_pet_pictures.txt");
        //load_photo("C:\\TEMP\\h19\\bin\\Debug\\e_shiny_selfies.txt");
        strcpy(result_name, "result");
    }
    else
    {
        load_photo(argv[1]);
        strcpy(result_name, argv[1]);
    }

    strcat(result_name, "_res.txt");

    if (nbelem < DATASET_GRANULARITY) {
		NB_CLIENTS = 1;
	}
	else {
		NB_CLIENTS = (int)(nbelem / DATASET_GRANULARITY);
		if (NB_CLIENTS*DATASET_GRANULARITY!=nbelem) NB_CLIENTS++;
	}
	if (NB_CLIENTS>MAX_CLIENTS ) NB_CLIENTS = MAX_CLIENTS ;

	COPIES_BY_THREAD = nbelem / NB_CLIENTS;
	if (COPIES_BY_THREAD==0) COPIES_BY_THREAD = 1;
	printf("pthread NB_CLIENTS=%d COPIES_BY_THREAD=%d\n", NB_CLIENTS, COPIES_BY_THREAD);
	store.thread_clients = (pthread_t *)malloc(sizeof(pthread_t) * NB_CLIENTS);

    int ls_photo = sort_photo();

    gettimeofday(&tv2, &tz2);
    printf("Done. pass %d us.\n", (tv2.tv_sec-tv1.tv_sec) * 1000000 + (tv2.tv_usec-tv1.tv_usec));
    printf("get %d lists\n", ls_photo);

    output(result_name, ls_photo);

    FILE *fp2 = fopen(result_name, "r");
    // Open file to store the result
    FILE *fp3;
    if (argc<3)
    {
        fp3 = fopen("output.txt", "w");
    }
    else
    {
        fp3 = fopen(argv[2], "w");
    }
    char c;

    if (fp2 == NULL || fp3 == NULL)
    {
         printf("Could not open files\n");
         exit(0);
    }
    fprintf(fp3, "%d\n", n_global_slides);

    while ((c = fgetc(fp2)) != EOF)
      fputc(c, fp3);

    fclose(fp2);
    fclose(fp3);

    free(photo);
    free(photo_elem);
    free(photo_sorted);
    free(photo_orient);
    free(photo_matched);
    free(photo_list);
    free(score_table);
    return 0;
}

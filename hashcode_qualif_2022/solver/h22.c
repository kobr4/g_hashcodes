// SOLVER OF GOOGLE HASHCODE 2022 - AMD EDITION
// RUN ON AMD THREADRIPPER 1950X FOR COMPETITION
// Authors : Michel HE, Nicolas MY
// The fastest known solver in Google Hashcode written in the C language

// Our scoreboard :
// A: 30     (sort prj by skill level)
// B: 550048 (sort prj by skill level)
// C: 199497 (sort prj by award)
// D: 112947  (sort prj by skill level)
// E: 1639934 (sort prj by deadline)
// F: 354527  (sort prj by deadline, and sort max skill level for each contributor)
// -----------
// Total : 2 856 983
/*
h22 a_an_example.in.txt 1 e 0
h22 b_better_start_small.in.txt 1 e 0
h22 c_collaboration.in.txt 0 s 0
h22 d_dense_schedule.in.txt 1 e 0
h22 e_exceptional_skills.in.txt 1 d 0
h22 f_find_great_mentors.in.txt 0 d 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define uint128_t __uint128_t  //  we choose the 128 bits hashcode for strings compare
#define MAX_CARDINAL 21 // max number of letters of a string element, 21 * 6 = 126 bits, the last 2 bits are not coded and if the last-1 bit is set, it'd stands for "Error: hash out of bound"
#define MAX_DATE 0xFFFFFFFF
#define MAX_ROLES 64    // max required number of team members for a project
#define MAX_SKILLS 1024 // max skills for a contributor
#define MAX_NAME_LEN 32
#define MAX_PROJ_LEN 32

#define DEBUG_PRINT

struct proj {
    unsigned int id;
    char prj_name[MAX_PROJ_LEN];
    unsigned int n_days_to_complete;
    unsigned int score_award;
    unsigned int best_before;
    unsigned int n_role;
    uint128_t role_skill_hash[MAX_ROLES]; //required number of skills
    unsigned int skill_level_required[MAX_ROLES]; //required skill level

    unsigned int contribs[MAX_ROLES];
    unsigned int contrib_skill_index[MAX_ROLES];
    unsigned int c_count;
    char bFilled;
    unsigned int end_date;
};


struct contrib {
    unsigned int id;
    char name[MAX_NAME_LEN];
    unsigned int n_skills;
    uint128_t skill_hash[MAX_SKILLS]; //skills of dev.
    unsigned int skill_level[MAX_SKILLS]; //skills level of dev.

    unsigned int available;
};


/***************
global variables
***************/

static struct contrib *contrib_list;
static struct proj *prj_list;

int nbcontrib = 0;
int nbprj = 0;
char bUpgradeMode = 0;

/***************
    functions
***************/

// hash allows fast comparison
uint128_t hash(const char *s)
{
   uint128_t h;
   unsigned const char *us;

   us = (unsigned const char *) s;
   if (strlen(s) > MAX_CARDINAL) { printf("ERR: hash(%s) overflows\n", s); exit(1);}
   h = 0;
   while(*us != '\0') {
       char ch = *us;
       if ((ch>='0') && (ch<='9')) ch=ch-'0'+1;
       else if ((ch>='a') && (ch<='z')) ch=ch-'a'+11;
       else if ((ch>='A') && (ch<='Z')) ch=ch-'A'+37;
       else if ((ch=='-') || (ch=='+')) ch=63;
       else { ch=64; printf("Error: try hash a char out of bound\n");}
       h = (h << 6) + ch;   // '<< 6' is equivalent to mul by 64 and shift is faster than mul operation
       us++;
   }
   return h;
}

// hash allows fast comparison
char *revert_hash(uint128_t h)
{
   char s[MAX_CARDINAL+1];
   char *r = (char*)malloc(MAX_CARDINAL+1);
   unsigned int us;
   int len=0;
   for (int i=0; i<=MAX_CARDINAL; i++) {
       char c = 0;
       us = (h & 0x3F);  // 6-bits
       if ((us>=1) && (us<=10)) c=us-1+'0';
       else if ((us>=11) && (us<=36)) c=us-11+'a';
       else if ((us>=37) && (us<=62)) c=us-37+'A';
       else if (us==63) c='*';  // whatever '+' or '-', doesn't matters
       else { c='0'; len=i; break;}
       s[i] = c;
       h = (h >> 6);
   }
   // Intel is little endian-format, so we reverse the string
   for (int i=0; i<len; i++)
   {
       r[len-i-1]=s[i];
   }
   r[len] = 0;

   return r;
}

int compareContribBySkill( const void* p1, const void* p2) {
    struct contrib* c1 = (struct contrib*)p1;
    struct contrib* c2 = (struct contrib*)p2;

    return (c1->n_skills-c2->n_skills);
}

int compareContribBySkillLevel( const void* p1, const void* p2) {
    struct contrib* c1 = (struct contrib*)p1;
    struct contrib* c2 = (struct contrib*)p2;

    int sk1 = 0;
    int sk2 = 0;

    for (int i = 0;i < c1->n_skills;i++) {
        if (sk1 < c1->skill_level[i])
            sk1 = c1->skill_level[i];
    }

    for (int i = 0;i < c2->n_skills;i++) {
        if (sk2 < c2->skill_level[i])
            sk2 = c2->skill_level[i];
    }

    return (sk1-sk2);
}


// for dataset E, F
int compareProjByDeadline( const void* p1, const void* p2) {
    struct proj* pr1 = (struct proj*)p1;
    struct proj* pr2 = (struct proj*)p2;
    //score smaller is better
    int scoring_p1 = pr1->best_before;
    int scoring_p2 = pr2->best_before;

    return (scoring_p1-scoring_p2);
}

// for dataset C
int compareProjByScore( const void* p1, const void* p2) {
    struct proj* pr1 = (struct proj*)p1;
    struct proj* pr2 = (struct proj*)p2;

    int scoring_p1 = pr1->score_award;
    int scoring_p2 = pr2->score_award;
    return (scoring_p2-scoring_p1);

}


int compareProjByDuration( const void* p1, const void* p2) {
    struct proj* pr1 = (struct proj*)p1;
    struct proj* pr2 = (struct proj*)p2;

    int scoring_p1 = pr1->n_days_to_complete;
    int scoring_p2 = pr2->n_days_to_complete;

    return (scoring_p1-scoring_p2);
}

int compareProjBySkill( const void* p1, const void* p2) {
    struct proj* pr1 = (struct proj*)p1;
    struct proj* pr2 = (struct proj*)p2;

    int scoring_p1 = pr1->n_role;
    int scoring_p2 = pr2->n_role;

    return (scoring_p1-scoring_p2);
}

// for dataset B, D
int compareProjBySkillLevel( const void* p1, const void* p2) {
    struct proj* pr1 = (struct proj*)p1;
    struct proj* pr2 = (struct proj*)p2;

    int sk1 = 0;
    int sk2 = 0;

    for (int i = 0;i < pr1->n_role;i++) {
        if (sk1 < pr1->skill_level_required[i])
            sk1 = pr1->skill_level_required[i];
    }

    for (int i = 0;i < pr2->n_role;i++) {
        if (sk2 < pr2->skill_level_required[i])
            sk2 = pr2->skill_level_required[i];
    }

    return (sk1-sk2);
}


int (*compareProjByBest)(const void*, const void*);

int (*compareContribByBest)(const void*, const void*);

void assign(struct proj * project, int idc) {

    // a project requires project->n_role
    if (project->c_count < project->n_role) {
        project->contribs[project->c_count] = idc;
        project->c_count++;
    }

}

void set_contrib(struct proj * project, struct contrib * c, int skill_level_required, int end_date, int role_i)
{
    c->available = end_date;

    if ((skill_level_required>=c->skill_level[project->contrib_skill_index[role_i]]) && bUpgradeMode)
        c->skill_level[project->contrib_skill_index[role_i]]++;
}

int already_assigned(struct proj * project, int idc) {
    for (int i = 0;i < project->c_count;i++) {
        if (contrib_list[project->contribs[i]].id == idc)
            return -1;
    }

    return 0;
}

int find_contrib(struct proj * project, struct contrib * contriblist, uint128_t role_skill_hash, int skill_level, int start, int roles[], int role_i) {
    int select = -1;
    int best_start_date;
    char bHasSkill;

    DEBUG_PRINT("search candidate with skill %s, level %d, c_count = %d\n", revert_hash(role_skill_hash), skill_level, project->c_count);
    for(int i = 0;i < nbcontrib; i++) {
        bHasSkill = 0;

        // let's define a date probable to start a prj - which is not the final real date
        best_start_date = project->best_before - project->n_days_to_complete;

        if ((already_assigned(project,contriblist[i].id)==0)
            && (contriblist[i].available <= (best_start_date))) {
            DEBUG_PRINT("candidate %s eligible\n", contriblist[i].name);
        }
        else continue;
        DEBUG_PRINT("check candidate %s\n", contriblist[i].name);
        for(int j = 0;j < contriblist[i].n_skills;j++) {
                if (contriblist[i].skill_hash[j] != role_skill_hash) continue;

                if (contriblist[i].skill_level[j] == (skill_level - 1)) {
                     // search mentor in the team
                    for (int mentor = 0; mentor < project->c_count; mentor++) {
                        struct contrib * c = &contriblist[roles[mentor]];
                        for(int r = 0;r < c->n_skills;r++) {
                            if ((c->skill_hash[r] == role_skill_hash)
                                && (c->skill_level[r] >= skill_level)) {
                                    select = i;
                                    project->contrib_skill_index[role_i] = j;
                                    DEBUG_PRINT("found with skill %s with mentor %s has skill level %d\n", revert_hash(role_skill_hash), c->name, c->skill_level[r]);
                                    DEBUG_PRINT("candidate with skill %s assigned to %s +selected\n", revert_hash(role_skill_hash), contriblist[i].name);
                                    return select;
                            }
                        }
                    } // mentor
                }
                else if (contriblist[i].skill_level[j] >= skill_level) {
                        select = i;
                        project->contrib_skill_index[role_i] = j;
                        DEBUG_PRINT("no mentor, candidate with skill %s assigned to %s +selected\n", revert_hash(role_skill_hash), contriblist[i].name);
                        return select;
                }
                else {
                    DEBUG_PRINT("candidate not selected due to his level %d required is %d, already assigned %d, available %d, but prj should start %d\n", contriblist[i].skill_level[j], skill_level, already_assigned(project,contriblist[i].id), contriblist[i].available, best_start_date);
                }

                bHasSkill = 1;

        }

        if (bHasSkill == 0) {
                int k = contriblist[i].n_skills;
                if (k>=MAX_SKILLS) {
                    printf("ERROR candidate can't have more skills\n");
                    exit(1);
                }
                DEBUG_PRINT("add new 0 skill %s to %s\n", revert_hash(role_skill_hash), contriblist[i].name);
                contriblist[i].skill_hash[k] = role_skill_hash;
                contriblist[i].skill_level[k] = 0;
                contriblist[i].n_skills++;
        }

    }
    return select;
}

int assign_project(struct proj * project, struct contrib * contriblist) {
    int start = 0;
    int roles[MAX_ROLES];
    struct proj *test_proj = (struct proj *)malloc(sizeof (struct proj));
    memcpy(test_proj, project, sizeof(struct proj));
    DEBUG_PRINT("try assign prj %s\n", project->prj_name);

    // we test if this project can be done by gathering all mandatory ppl
    for (int i = 0;i < test_proj->n_role;i++) {
        int c = find_contrib(test_proj, contriblist, test_proj->role_skill_hash[i], test_proj->skill_level_required[i], start, roles, i);

        if (c == -1) {
            free(test_proj);
            return -1;
        }
        if (start < contriblist[c].available)
            start = contriblist[c].available;

        assign(test_proj, c);
        roles[i] = c;
    }

    // found everybody, now ready to assign
    project->end_date = start + project->n_days_to_complete;
    for (int i = 0;i < project->n_role;i++) {
        assign(project, roles[i]);
        set_contrib(project, &contriblist[roles[i]], project->skill_level_required[i], project->end_date, i);
    }
    free(test_proj);
    return 0;
}

int solve(struct proj * projlist, int projcount, struct contrib * contriblist) {
    qsort(contriblist, nbcontrib, sizeof(struct contrib), compareContribByBest);
    for (int n=0; n<nbcontrib; n++) contriblist[n].id = n;

    qsort(projlist, projcount, sizeof(struct proj), compareProjByBest);

    int sum_assigned_prj = 0;
    for (int i = 0;i < projcount; i++) {
        if (assign_project(&projlist[i], contriblist) == 0) {
            projlist[i].bFilled = 1;
            sum_assigned_prj++;
        }
    }

    if (sum_assigned_prj == 0) {
        printf("ERROR no prj was allocated\n");
    }
    return sum_assigned_prj;
}

void output(char * filename, int filled_count) {
    FILE * f = fopen(filename, "w");
    fprintf(f, "%d\n", filled_count);
     for (int i = 0;i < nbprj;i++) {
        if (prj_list[i].bFilled == 1) {
            fprintf(f, "%s\n", prj_list[i].prj_name);
            for (int j = 0;j < prj_list[i].n_role;j++) {
                fprintf(f, "%s ", contrib_list[prj_list[i].contribs[j]].name);
            }
            fprintf(f,"\n");
        }
    }
    fclose(f);
}


void load_prj(char * filename)
{
    printf("loading %s...\n", filename);
    FILE *f=fopen(filename,"r");
    if (f==NULL)
    {
        perror("fopen error\n");
        exit(1);
    }
    fscanf(f, "%d %d\n", &nbcontrib, &nbprj);
    printf("Load %d contrib, %d prj\n", nbcontrib, nbprj);
    printf("will alloc %d bytes for contributor and %d bytes for prj\n", sizeof(struct contrib)*nbcontrib, sizeof(struct proj)*nbprj);
    contrib_list = (struct contrib *)malloc(sizeof(struct contrib)*nbcontrib);
    prj_list = (struct proj *)malloc(sizeof(struct proj)*nbprj);

    printf("+ loading contrib...\n");
    for (int ii=0; ii<nbcontrib; ii++) {
        char sk[MAX_CARDINAL];
        fscanf(f, "%s %d\n", &contrib_list[ii].name, &contrib_list[ii].n_skills);
        contrib_list[ii].id = ii;
        contrib_list[ii].available = 0;

        if (contrib_list[ii].n_skills>MAX_SKILLS) {
                printf("ERROR : MAX SKILLS BY CONTRIBUTOR REACHED !!\n");
                printf("ERROR contrib id: %d, name :%s\n", contrib_list[ii].id, contrib_list[ii].name);
                exit(1);
        }

        for (int jj=0; jj<contrib_list[ii].n_skills; jj++) {
            fscanf(f, "%s %d\n", sk, &contrib_list[ii].skill_level[jj]);
            contrib_list[ii].skill_hash[jj] = hash(sk);
            DEBUG_PRINT("candidate %s has skill %s with level %d\n", contrib_list[ii].name, revert_hash(contrib_list[ii].skill_hash[jj]), contrib_list[ii].skill_level[jj]);
        }
    }

    printf("+ loading prj...\n");
    DEBUG_PRINT("name, n_days_to_complete, score_award, best_before, n_role\n");

    for (int pp=0; pp<nbprj; pp++) {
        prj_list[pp].id = pp;
        prj_list[pp].c_count = 0;
        prj_list[pp].bFilled = 0;
        prj_list[pp].end_date = MAX_DATE;
        fscanf(f, "%s %d %d %d %d\n", prj_list[pp].prj_name, &prj_list[pp].n_days_to_complete, &prj_list[pp].score_award, &prj_list[pp].best_before, &prj_list[pp].n_role);
        DEBUG_PRINT("%s %d %d %d %d\n", prj_list[pp].prj_name, prj_list[pp].n_days_to_complete, prj_list[pp].score_award, prj_list[pp].best_before, prj_list[pp].n_role);

        if (prj_list[pp].n_role>MAX_ROLES) {
                printf("ERROR : MAX ROLES REACHED\n");
                printf("ERROR proj id: %d, name :%s\n", prj_list[pp].id, prj_list[pp].prj_name);
                exit(1);
        }


        for (int rr=0; rr< prj_list[pp].n_role; rr++) {
            char sk[MAX_CARDINAL];
            fscanf(f, "%s %d\n", sk, &prj_list[pp].skill_level_required[rr]);
            prj_list[pp].role_skill_hash[rr] = hash(sk);
            DEBUG_PRINT("%s %d\n", sk, prj_list[pp].skill_level_required[rr]);
        }
    }
    fclose(f);
    printf("input all lines read\n");
}

int main(int argc, char **argv)
{
    char result_name[128];
    //measure of time of the algo
    struct timeval tv1, tv2;
    struct timezone tz1, tz2;
    gettimeofday(&tv1, &tz1);

    if (argc<2)
    {
        load_prj("C:\\TEMP\\h22\\bin\\Debug\\a_an_example.in.txt");
        //load_prj("C:\\TEMP\\h22\\bin\\Debug\\b_better_start_small.in.txt");
        //load_prj("C:\\TEMP\\h22\\bin\\Debug\\c_collaboration.in.txt");
        //load_prj("C:\\TEMP\\h22\\bin\\Debug\\d_dense_schedule.in.txt");
        //load_prj("C:\\TEMP\\h22\\bin\\Debug\\e_exceptional_skills.in.txt");
        //load_prj("C:\\TEMP\\h22\\bin\\Debug\\f_find_great_mentors.in.txt");

        strcpy(result_name, "result");
    }
    else
    {
        load_prj(argv[1]);
    }

    // default modes
    compareProjByBest = compareProjByDuration;
    compareContribByBest = compareContribBySkill;

    if (argc>=3) {
        if (argv[2][0] == '1') {
            printf("The contributor skill upgrade mode enable for this dataset\n");
            bUpgradeMode = 1;
        }
    }
    if (argc>=4) {
         if (argv[3][0] == 's')
         {
             printf("sort prj by award score\n");
             compareProjByBest = compareProjByScore;
         }
         else if (argv[3][0] == 'd')
         {
             printf("sort prj by deadline\n");
             compareProjByBest = compareProjByDeadline;
         }
         else if (argv[3][0] == 'u')
         {
             printf("sort prj by duration\n");
             compareProjByBest = compareProjByDuration;
         }
         else if (argv[3][0] == 'k')
         {
             printf("sort prj by total skills\n");
             compareProjByBest = compareProjBySkill;
         }
         else if (argv[3][0] == 'e')
         {
             printf("sort prj by level\n");
             compareProjByBest = compareProjBySkillLevel;
         }
         else
         {
             printf("sort prj by duration\n");
         }
    }
    if (argc>=5) {
        if (argv[4][0] == '1') {
            printf("sort contributors by his max skill level\n");
            compareContribByBest = compareContribBySkillLevel;
        }
        else {
            printf("sort contributors by number of skills\n");
        }
    }
    int filled_count = solve(prj_list, nbprj, contrib_list);

	char str[100];
	sprintf(str, "%s.out", argv[1]);
    output(str, filled_count);

    printf("filled projects %d\n", filled_count);
    gettimeofday(&tv2, &tz2);
    printf("Done. pass %ld us.\n", (tv2.tv_sec-tv1.tv_sec) * 1000000 + (tv2.tv_usec-tv1.tv_usec));
    free(contrib_list);
    free(prj_list);
}

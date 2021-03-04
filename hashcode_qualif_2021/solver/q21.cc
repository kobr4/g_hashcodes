// SOLVER OF GOOGLE HASHCODE 2021 - AMD EDITION
// RUN ON AMD THREADRIPPER 1950X FOR COMPETITION
// Authors : Michel HE, Nicolas MY
// The fastest known solver in Google Hashcode

#include <map>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <list>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define MAX_LEN 11000   // due to f.txt

#define DEBUG_COUT cout

#define L_FACTOR 1
#define K_FACTOR 300   // the epic number of spartans
#define R_FACTOR 1

using namespace std;

typedef struct
{
	int D; // the duration of the simulation, in seconds,
	int I; // the number of intersections
	int S; // the number of streets,
	int V; // the number of cars
	int F; // the bonus points for each car that reaches its destination before time D .
} T_PARAMS;

T_PARAMS gParams;

typedef struct
{
	unsigned int inter_s;
	unsigned int inter_d;
	char name[16];
	unsigned int time;
} T_STREET;


typedef struct {
    unsigned int street_count;
    T_STREET ** streets;
} T_INTERSEC;

 void removeNonAscii(char line[],char modifiedline[])
 {
     int i=-1;
     int j=-1;
     while(line[++i]!='\0'){
        if(!(line[i]>=32&&line[i]<=127))continue;
        else modifiedline[++j]=line[i];
     }
     modifiedline[++j]='\0';
 }

int main(int argc, char **argv)
{
    ofstream myfile;

    map<string, int> s;
    map<int, string> rev_s;
    multimap <int, int> map_inters_s, map_inters_d;
    list<vector<int>> lst;

	int ret_item;
	char filename[256];
	if (argc >= 2)
	{
		strcpy(filename, argv[1]);
	}
	else
	{
		strcpy(filename, "a.txt");
	}
	printf("open %s\n", filename);

	FILE * fin = fopen(filename, "r");

	if (fin == NULL)
	{
		printf("fopen err\n");
		exit(1);
	}

	fscanf(fin, "%d %d %d %d %d\n", &gParams.D, &gParams.I, &gParams.S, &gParams.V, &gParams.F);
	printf("Duration %d, N intersec %d, N streets %d, N cars %d, Bonus point %d\n",
		gParams.D, gParams.I, gParams.S, gParams.V, gParams.F);

	T_STREET *citystreet = (T_STREET*)malloc(sizeof(T_STREET) * gParams.S);

	for (int n_street = 0; n_street < gParams.S; n_street++)
	{
		ret_item = fscanf(fin, "%d %d %s %d\n",
			&citystreet[n_street].inter_s,
			&citystreet[n_street].inter_d,
			&citystreet[n_street].name,
			&citystreet[n_street].time);

      if (s.find(citystreet[n_street].name) == s.end()) {
         char clean_txt[16];
         removeNonAscii(citystreet[n_street].name, clean_txt);
         //DEBUG_COUT << "add street " << n_street << " "<< clean_txt << " into map\n";
         s[clean_txt] = n_street;
         rev_s[n_street] = clean_txt;
      }

      map_inters_s.insert(pair <int, int> (citystreet[n_street].inter_s, s[citystreet[n_street].name]));
      map_inters_d.insert(pair <int, int> (citystreet[n_street].inter_d, s[citystreet[n_street].name]));
	}

    vector<int> vl;
    int segment=0;
    int *ref_count = (int*)malloc(sizeof(int)*gParams.S);
    memset(ref_count, 0, sizeof(int)*gParams.S);
    int factor_deep = 0;

    for (int k=0; k<gParams.V; k++) {
        char line_data[MAX_LEN];
        int len;
        fgets(line_data, MAX_LEN - 1, fin);
        segment = atoi(strtok(line_data, " "));
        //DEBUG_COUT <<  "segment of " << k << " has " << segment << " n_wp" <<endl;
        factor_deep = 0;
        for (int j=0; j<segment; j++)
        {
            char *stname = strtok(NULL, " ");
            char clean_txt[16];
            removeNonAscii(stname, clean_txt);
            std::map<std::string, int>::iterator i = s.find(clean_txt);
            assert(i != s.end());
            ref_count[s[clean_txt]] = ref_count[s[clean_txt]] + 1;
            //DEBUG_COUT << "str:" << clean_txt << " number:" << s[clean_txt] <<  " ref_count = " << ref_count[s[clean_txt]] << " factor_deep = " << factor_deep << endl;
            vl.push_back(i->second);
            factor_deep++;
        }
        fscanf(fin, "\n");
        lst.push_back(vl);
        vl.clear();
    }

    string fileout_name = (string)filename + ".out";
    myfile.open(fileout_name);

    // at this moment, we don't know how many inters are countable
    myfile << "        " << endl;

    int max_st_by_inter = 0;
    for (int n_inter = 0; n_inter < gParams.I; n_inter++)
	{
            int n_streets_by_inters = map_inters_d.count(n_inter);
            if (n_streets_by_inters > max_st_by_inter) max_st_by_inter = n_streets_by_inters;
	}

    DEBUG_COUT << " max number streets / inter = " << max_st_by_inter << endl;
    int *score_table = (int *)malloc(gParams.I*max_st_by_inter*sizeof(int));

    // compute the score table
    for (int n_inter = 0; n_inter < gParams.I; n_inter++)
	{
        multimap <int, int> :: iterator itr;

        if (map_inters_d.find(n_inter) == map_inters_d.end()) {
                //DEBUG_COUT << "ERROR : no dest !" << '\n';
        }
        else  {
            int num_street_by_inter = map_inters_d.count(n_inter);
            //DEBUG_COUT << "inter " << n_inter << " has number street : " << num_street_by_inter << endl;

            // making of scores table
            int ss = 0;
            for (itr = map_inters_d.equal_range(n_inter).first; itr != map_inters_d.equal_range(n_inter).second; ++itr)
            {
                int sc = 0;
                if (ref_count[itr->second] == 0) score_table[n_inter*max_st_by_inter+ss] = sc;
                else {
                        sc = gParams.D / (K_FACTOR*num_street_by_inter) + R_FACTOR*ref_count[itr->second] - L_FACTOR*citystreet[itr->second].time;
                        // check the availability range
                        // overshoot
                        if (sc > gParams.D/K_FACTOR) sc = gParams.D/K_FACTOR;
                        // undershoot
                        if (sc <= 0) sc = 1;
                        score_table[n_inter*max_st_by_inter+ss] = sc;
                }
                //DEBUG_COUT << " score of " << n_inter << "@" << ss << " = " << score_table[n_inter*max_st_by_inter+ss];
                ss++;
            }
        }
	}

	int total_countable_inter = 0;
    // output score now
    for (int n_inter = 0; n_inter < gParams.I; n_inter++)
	{
        multimap <int, int> :: iterator itr;

        if (map_inters_d.find(n_inter) == map_inters_d.end()) {
                //DEBUG_COUT << "ERROR : no dest !" << '\n';
        }
        else  {

            int num_street_by_inter = map_inters_d.count(n_inter);
            int countable_street = 0;
            int ss = 0;
            for (itr = map_inters_d.equal_range(n_inter).first; itr != map_inters_d.equal_range(n_inter).second; ++itr)
            {
                if (score_table[n_inter*max_st_by_inter+ss] > 0) {
                        countable_street++;
                }
                ss++;
            }
            if (countable_street>0) {
                myfile << n_inter << endl;
                myfile << countable_street << endl;
                ss = 0;
                for (itr = map_inters_d.equal_range(n_inter).first; itr != map_inters_d.equal_range(n_inter).second; ++itr)
                {
                    int sc = score_table[n_inter*max_st_by_inter+ss];
                    if (sc > 0) {
                         myfile << rev_s[itr->second] << " " << sc << endl;
                    }
                    ss++;
                }

                total_countable_inter++;
            }
        }
	}

    myfile.seekp (0, ios::beg);
    myfile << total_countable_inter;
    myfile.close();
    return 0;
}

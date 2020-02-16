
/*
Author: Michel HE
   GOOGLE HASHCODE 2019 FINAL ROUND - RUN ON AMD 1950X
*/

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <map>
#include <functional>


#define DEBUG_PRINT cout
#define DEBUG_INFO cout
#define DEBUG_ERROR cout

#define MAX_SCORE_RC 500000
#define UNDEF_VALUE 0xffffffff

using namespace std;

// Global params
int NumberObj = 0;
int NumberTarget = 0;
int num_files = UNDEF_VALUE, num_targets = UNDEF_VALUE, num_servers = UNDEF_VALUE;

// *** Define Class
class CIndividu
{
public:
	string name;
	int compile_time;
	int replicate_time;
    int num_dep;
	vector<string> dependencies;

    bool operator< (const CIndividu& userObj) const
    {
        if(userObj.num_dep < this->num_dep)
            return true;
        else return false;
    }
};

struct s_sort_compile_time
{
	bool operator()(const CIndividu &left, const CIndividu &right)
	{
		return (left.compile_time < right.compile_time);
	}
} sort_compile_time;

struct s_sort_replicate_time
{
	bool operator()(const CIndividu &left, const CIndividu &right)
	{
		return (left.replicate_time < right.replicate_time);
	}
} sort_replicate_time;

struct s_sort_num_dep
{
	bool operator()(const CIndividu &left, const CIndividu &right)
	{
		return (left.num_dep > right.num_dep);
	}
} sort_num_dep;


// *** Define Class
class CTarget
{
public:
	string name;
	int deadline;
	int global_points;

    bool operator< (const CTarget& userObj) const
    {
        if(userObj.deadline < this->deadline)
            return true;
        else return false;
    }
};

struct s_sort_deadline
{
	bool operator()(const CTarget &left, const CTarget &right)
	{
		return (left.deadline < right.deadline);
	}
} sort_deadline;

struct s_sort_global_points
{
	bool operator()(const CTarget &left, const CTarget &right)
	{
		return (left.global_points > right.global_points);
	}
} sort_global_points;

vector<string> split(const string &s, char delim)
{
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim))
	{
		result.push_back(item);
	}

	return result;
}

void uniq(vector<string> &v)
{
	auto end = v.end();
	for (auto it = v.begin(); it != end; ++it) {
		end = remove(it + 1, end, *it);
	}

	v.erase(end, v.end());
}

unsigned int get_diff(vector<string> v1, vector<string> v2)
{
    int sameobj = 0;
    for (unsigned int i = 0; i < v1.size(); i++) {
        for (unsigned int k = 0; k < v2.size(); k++) {
          if (!v1[i].compare(v2[k])) {
            sameobj++;
            break;
          }
        }
    }
    return sameobj;
}

void create_depmap(string dep2, vector<string>& objList_target, map<string, CIndividu>& objMap)
{
    if(objMap.find(dep2) == objMap.end()) return;
    CIndividu individu = objMap[dep2];
    //DEBUG_PRINT << dep2 << "has dep as " << individu.num_dep << endl;
    if (individu.num_dep>0)
    {
        for (vector<string>::iterator itObj = individu.dependencies.begin(); itObj != individu.dependencies.end(); ++itObj)
        {
            string dep2 = *itObj;
            create_depmap(dep2, objList_target, objMap);
            objList_target.push_back(dep2);
        }
    }
}


void parser(vector<CIndividu>& objList, map<string, CIndividu>& objMap,
    vector<CTarget>& targetList, vector<vector<string>>& target_server, string in_filename)
{
	string line, line2;
	ifstream myfile(in_filename);
    int param; //used to parse a line

	if (myfile.is_open())
	{
	    getline(myfile, line);
        vector<string> v = split(line, ' ');
        param = 0;
        for (vector<string>::iterator it = v.begin(); it != v.end(); ++it, ++param)
        {
        switch (param)
			  {
				case 0:
				{
				    num_files = stoi(*it);
				    break;
				}
				case 1:
				{
				    num_targets = stoi(*it);
				    break;
				}
				case 2:
				{
				    num_servers = stoi(*it);
				    break;
				}
				default:
            break;
        }
        }

	    DEBUG_INFO << "source files " << num_files << " to num_targets " << num_targets << " on num_servers " << num_servers << endl;
        int current_file = 0;
		while (current_file < num_files)
		{
		    getline(myfile, line);
		    getline(myfile, line2);
		    current_file++;
		    line = line + " " + line2;

			CIndividu individu;
			vector<string> v = split(line, ' ');

			//parse a line
			param = 0;
			for (vector<string>::iterator it = v.begin(); it != v.end(); ++it, ++param)
			{
				switch (param)
				{
				case 0:
				{
					individu.name = *it;
					break;
				}
				case 1:
				{
					individu.compile_time = stoi(*it);
					break;
				}
				case 2:
				{
					individu.replicate_time = stoi(*it);
					break;
				}
				case 3:
				{
					individu.num_dep = stoi(*it);
					break;
				}
				default:
					string dep = *it;
					individu.dependencies.push_back(dep);
					break;
				}
			} //parse line end

			objList.push_back(individu);
			objMap.insert(make_pair(individu.name, individu));
			NumberObj++;
		}

		if (NumberObj!=num_files)
        {
            DEBUG_ERROR << "Number of Obj incorrect !" << endl;
        }
		//DEBUG_PRINT << endl;

		//sort obj by number of dep - useless in this algorithm
		//sort(objList.begin(), objList.end(), sort_num_dep);
/* for debug
		for (int i = 0; i < NumberObj; i++)
		{
			//DEBUG_PRINT << objList[i].name << " " << objList[i].num_dep << " " << endl;
		}
*/

		while (getline(myfile, line))
		{
            vector<string> v = split(line, ' ');
            param = 0;
            CTarget ptarget;
            for (vector<string>::iterator it = v.begin(); it != v.end(); ++it, ++param)
            {
                switch (param)
                {
                    case 0:
                    {
                        ptarget.name = *it;
                        break;
                    }
                    case 1:
                    {
                        ptarget.deadline = stoi(*it);
                        break;
                    }
                    case 2:
                    {
                        ptarget.global_points = stoi(*it);
                        break;
                    }
                    default:
                        break;
                }
            }
            targetList.push_back(ptarget);
            NumberTarget++;
		}

		if (NumberTarget!=num_targets)
        {
            DEBUG_ERROR << "Number of Target incorrect !" << endl;
        }
        //DEBUG_PRINT << endl;
        myfile.close();
	}
	else
    {
        DEBUG_ERROR << "read file fatal error" << endl;
        exit(1);
	}
}


void processor(vector<CIndividu>& objList, map<string, CIndividu>& objMap,
    vector<CTarget>& targetList, vector<vector<string>>& target_server, string out_filename)
{
        unsigned int max_similar = 0;
        unsigned int serv1, serv2;
        int serv = 0;
        int Time_index;

        ofstream outfile;
        outfile.open (out_filename);

		// sort by target deadlines - useful for later
		sort(targetList.begin(), targetList.end(), sort_deadline);

		// may try this instead of deadline
		//sort(targetList.begin(), targetList.end(), sort_global_points);

/*  for debug
		for (int i = 0; i < NumberTarget; i++)
		{
			//DEBUG_PRINT << targetList[i].name << " " << targetList[i].deadline << " " << endl;
		}
*/

		// at beginning, each target has its own server
        for (int i = 0; i < NumberTarget; i++)
		{
		    for (int j = 0; j < NumberObj; j++)
            {
                if (!targetList[i].name.compare(objList[j].name))
                {
                    //DEBUG_PRINT << endl << "found target : " << targetList[i].name << " " << targetList[i].deadline << " " << endl;
                    vector<string> objList_target;

                    create_depmap(targetList[i].name, objList_target, objMap);
                    objList_target.push_back(targetList[i].name);
                    target_server.push_back(objList_target);
                }
            }
		}
/*	for debug
        //DEBUG_PRINT << "-- List of targets" <<endl;
		for (int j = 0; j < NumberTarget; j++)
        {
            //DEBUG_PRINT << "j=" << j << endl;
            vector<string> objList_onServer = target_server[j];
            for (vector<string>::iterator it = objList_onServer.begin(); it != objList_onServer.end(); ++it)
            {
                string dep = *it;
                //DEBUG_PRINT << "-" << dep;
            }
            //DEBUG_PRINT << endl;
        }
*/
        DEBUG_INFO << "** reducing the number of target servers" << endl;
        if ((NumberTarget>num_servers) && (NumberTarget>1))
        {
            int ReducedTarget = target_server.size();

            while ((ReducedTarget>num_servers) && (ReducedTarget>1))
            {
                max_similar = 0;
                // for processing small array ok, but not likely with f_big.in
                if  (target_server.size()<MAX_SCORE_RC)
                {
                    // trying to gather different servers using mostly same obj
                    for (unsigned int elem=0; elem<target_server.size(); elem++)
                    {
                        for (unsigned int i=elem; i<target_server.size()-1; i++)
                        {
                            unsigned int sim;
                            sim = get_diff(target_server[elem], target_server[i+1]);
                            if (max_similar < sim) { max_similar = sim; serv1=elem; serv2=i+1;}
                        }
                    }
                }

                if (max_similar==0)
                {
                    serv1 = 0;
                    // random still is the arbitrary choice
                    serv2 = (rand() % target_server.size()) + 1;
                    if (serv2==target_server.size()) serv2 = target_server.size()-1;

                    // trying the load balancing
                    unsigned int least_occup1 = UNDEF_VALUE;
                    unsigned int least_occup2 = UNDEF_VALUE;
                    unsigned int occupancy = UNDEF_VALUE;
                    for (unsigned int elem=0; elem<target_server.size(); elem++)
                    {
                            unsigned int vlen = target_server[elem].size();
                            if (occupancy>vlen)
                            {
                                least_occup2 = least_occup1;
                                least_occup1 = elem;
                                occupancy = vlen;
                            }
                    }
                    if ((least_occup1!=UNDEF_VALUE) && (least_occup2!=UNDEF_VALUE))
                    {
                        DEBUG_INFO << "** servers load balacing" << endl;
                        serv1 = least_occup1;
                        serv2 = least_occup2;
                        //DEBUG_PRINT << "found 2 lazy servers " << least_occup1 << " and " << least_occup2 << endl;
                    }
                }

                if (serv1>serv2)
                {
                    swap(serv1,serv2);
                }

                //DEBUG_PRINT << "max_similar = " << max_similar << " between " << serv1 << " " << serv2 << endl;

                vector<string> v1 = target_server[serv1];
                vector<string> v2 = target_server[serv2];
                vector<string> vfusion(v1);
                vfusion.insert(vfusion.end(), v2.begin(), v2.end());
                uniq(vfusion);

                target_server.erase(target_server.begin()+serv2);
                target_server[serv1]=vfusion;

                ReducedTarget = target_server.size();
            }
        }

        //create time map
        DEBUG_INFO << "** start computing time map" << endl;
        map<string, int> TimeMap;
        for (vector<vector<string>>::iterator it = target_server.begin(); it != target_server.end(); ++it, ++serv)
        {
            vector<string> objList_target = *it;
            if (objList_target.size()>0)
            {
                Time_index = 0;

                //DEBUG_PRINT << endl << "server " << serv << endl;
                for (vector<string>::iterator it2 = objList_target.begin(); it2 != objList_target.end(); ++it2)
                {
                    string dep = *it2;
                    CIndividu ob = objMap[dep];
                    Time_index += ob.compile_time;
                    if(TimeMap.find(dep) != TimeMap.end())
                    {
                            if (Time_index + ob.replicate_time < TimeMap[dep])
                            {
                                //DEBUG_PRINT << " old dep " << dep << "has time " << TimeMap[dep] << endl;
                                TimeMap[dep] = Time_index + ob.replicate_time;
                                //DEBUG_PRINT << " new dep " << dep << "has new avail time " << TimeMap[dep] << endl;
                            }
                    }
                    else {
                            TimeMap.insert(make_pair(dep, Time_index + ob.replicate_time));
                    }
                    //DEBUG_PRINT << "/" << dep << " at " << Time_index + ob.replicate_time << " ";
                }

            }
        }

        DEBUG_INFO << "** end computing time map" << endl;

        DEBUG_INFO << "** prune useless steps" << endl;
        serv = 0;
        // prune unused steps of compilation
        for (vector<vector<string>>::iterator it = target_server.begin(); it != target_server.end(); ++it, ++serv)
        {
            vector<string> objList_target = *it;
            if (objList_target.size()>0)
            {
                Time_index = 0;

                //DEBUG_PRINT << "server " << serv << endl;
                for (vector<string>::iterator it2 = objList_target.begin(); it2 != objList_target.end(); ++it2)
                {
                    string dep = *it2;
                    CIndividu ob = objMap[dep];
                    Time_index += ob.compile_time;
                    if (TimeMap.find(dep) != TimeMap.end() && (Time_index >= TimeMap[dep]))
                    {
                        //DEBUG_PRINT << "prune " << dep << endl;
                        objList_target.erase(it2);
                    }
                }
                *it = objList_target;
            }
        }

        //show list
        DEBUG_INFO << endl << "** final list" << endl;
        unsigned int NumberStep = 0;
        unsigned int MaxStepByServer = 0;
        serv = 0;
        for (vector<vector<string>>::iterator it = target_server.begin(); it != target_server.end(); ++it, ++serv)
        {
            vector<string> objList_target = *it;
            if (MaxStepByServer < objList_target.size())
            {
                MaxStepByServer = objList_target.size();
            }

            if (objList_target.size()>0)
            {
                uniq(objList_target);
                *it = objList_target;
                NumberStep += objList_target.size();
                /* for debug x
                DEBUG_PRINT << endl << "server " << serv << endl;
                for (vector<string>::iterator it2 = objList_target.begin(); it2 != objList_target.end(); ++it2)
                {
                    string dep = *it2;
                    DEBUG_INFO << dep << " ";
                }
                */
            }
        }
        DEBUG_INFO << endl;
        DEBUG_INFO << "NumberStep " << NumberStep << endl;
        DEBUG_INFO << "MaxStepByServer " << MaxStepByServer << endl;

        // output final format
        outfile << NumberStep << endl;
        for (unsigned int s=0; s<MaxStepByServer; s++)
        {
            serv = 0;
            for (vector<vector<string>>::iterator it = target_server.begin(); it != target_server.end(); ++it, ++serv)
            {
                vector<string> objList_target = *it;
                if (s < objList_target.size())
                {
                    outfile << objList_target[s] << " " << serv << endl;
                }
            }
        }

		outfile.close();
}


int main(int argc, char** argv)
{
    char *filename;
    if (argc>1)
    {
        filename = argv[1];
    }
    else
    {
        filename = "a_example.in";
    }
    string in_filename(filename);
    string out_filename(filename);
    out_filename += ".out";

	vector<CIndividu> objList;
	map<string, CIndividu> objMap;
  vector<CTarget> targetList;
  vector<vector<string>> target_server;

  parser(objList,objMap,targetList,target_server,in_filename);
  processor(objList,objMap,targetList,target_server,out_filename);
}

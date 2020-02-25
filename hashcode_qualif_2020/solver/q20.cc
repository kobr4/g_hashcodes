
// MULTITHREADED SOLVER GOOGLE HASHCODE 2020 - AMD EDITION
// RUN ON AMD THREADRIPPER 1950X FOR COMPETITION
// Authors : Michel HE, Eric BELLONI, Jamal KHEYYAD, Nicolas MY
// The fastest known solver in Google Hashcode 

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <map>
#include <functional>
#include <thread>

#define DEBUG_PRINT cout
#define DEBUG_INFO cout
#define DEBUG_ERROR cout

#define MAX_SCORE_RC 500000
#define UNDEF_VALUE 0xffffffff
#define DATASET_GRANULARITY 1000

using namespace std;

// Global params
int NumberObj = 0;
int NumberTarget = 0;
int num_books = UNDEF_VALUE, num_libraries = UNDEF_VALUE, num_days = UNDEF_VALUE;
bool gflag = false;
vector<int> books_order;
int NB_CLIENTS;

// *** Define Class

class CLibrary
{
	public:
		int lib_index;
		double score;
		int total_books;
		int days_for_signup;
		int book_ship_by_day;
		vector<int> book;

		bool operator< (const CLibrary& userObj) const
		{
			if(userObj.total_books < this->total_books)
				return true;
			else return false;
		}

		void doScore() {
			int sum = 0;
			for (auto& booki: book) {
				sum += books_order[booki];
			}

			score = (double) sum * book_ship_by_day / (days_for_signup*days_for_signup);
		}
};

vector<CLibrary> objList;

struct s_sort_by_score
{
	bool operator()(const CLibrary &left, const CLibrary &right)
	{
		return (left.score > right.score);
	}
} sort_by_score;


// *** Define Class
class CSubmission
{
	public:
		int total_libraries;
		// this is the output
		vector<int> library;
		vector<vector<int>> book;

		bool operator< (const CSubmission& userObj) const
		{
			if(userObj.total_libraries < this->total_libraries)
				return true;
			else return false;
		}
};




void write_output(CSubmission &sub, string out_filename) 
{

	ofstream outfile;

	outfile.open(out_filename);

	outfile << sub.total_libraries << endl;
	for(int i = 0; i < sub.library.size(); i++) {
		outfile << sub.library[i] << " " << sub.book[i].size() << endl;


		for (auto& b: sub.book[i]) {

			outfile << b << " ";
		}
		outfile << endl;
	}

	outfile.close();
}

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

void parser(vector<CLibrary>& objList, string in_filename)
{
	string line, line2;
	vector<string> v;
	vector<string> v2;
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
						num_books = stoi(*it);
						break;
					}
				case 1:
					{
						num_libraries = stoi(*it);
						break;
					}
				case 2:
					{
						num_days = stoi(*it);
						break;
					}
				default:
					break;
			}
		}

		DEBUG_INFO << "source files " << num_books << " to num_libraries " << num_libraries << " on num_days " << num_days << endl;

		getline(myfile, line);
		v = split(line, ' ');
		param = 0;

		//cout << "order :" << endl;
		for (vector<string>::iterator it = v.begin(); it != v.end(); ++it, ++param)
		{
			books_order.push_back(stoi(*it));
			//cout << stoi(*it) << " " ;
		}
		//cout << endl;
		DEBUG_INFO << "total score: " << books_order.size() << endl;
		cout << endl;
		NumberObj = 0;

		while (NumberObj < num_libraries)
		{
			getline(myfile, line);

			CLibrary individu;
			//parse a line
			param = 0;
			vector<string> v = split(line, ' ');
			for (vector<string>::iterator it = v.begin(); it != v.end(); ++it, ++param)
			{
				switch (param)
				{
					case 0:
						{
							individu.total_books = stoi(*it);
							break;
						}
					case 1:
						{
							individu.days_for_signup = stoi(*it);
							break;
						}
					case 2:
						{
							individu.book_ship_by_day = stoi(*it);
							break;
						}
					default:
						break;
				}
			}

			//parse a line
			param = 0;

			getline(myfile, line2);
			v2 = split(line2, ' ');

			for (vector<string>::iterator it = v2.begin(); (it != v2.end()) && (param<individu.total_books); ++it, ++param)
			{
				individu.book.push_back(stoi(*it));
			} //parse line end

			// library ready
			objList.push_back(individu);

			NumberObj++;
		}

		cout << "Number of Library " << NumberObj << endl;
		/* for debug
		   cout << "check lib " << endl;

		   for (vector<CLibrary>::iterator it = objList.begin(); (it != objList.end()); ++it)
		   {
		   CLibrary lib = *it;
		   for (vector<int>::iterator it2 = lib.book.begin(); (it2 != lib.book.end()); ++it2)
		   {
		   cout << " " << *it2;
		   }
		   cout << endl;
		   }
		   */
		myfile.close();
	}
	else
	{
		DEBUG_ERROR << "read file fatal error" << endl;
		exit(1);
	}
}

void thr(int num_thr)
{
	int begin = num_thr * DATASET_GRANULARITY;
	int end;

	if (num_thr == (NB_CLIENTS-1))
	{
		std::cout << "last segment " << objList.size() << endl;
		end = objList.size();
	}
	else
	{
		end = begin+DATASET_GRANULARITY;
	}

	std::cout << "num_thr = " << num_thr << " fr " << begin << " to " << end << std::endl;

	for (int i = begin; i < end; i++) {
		objList[i].lib_index = i;
		objList[i].doScore();
	}
}


int main(int argc, char** argv)
{
	char *cvalue;
	int c;

	std::vector<std::thread> mythreads;

	opterr = 0;

	while ((c = getopt (argc, argv, "f:")) != -1)
		switch (c)
		{			
			case 'f':
				cvalue = optarg;
				break;
			case '?':
				if (optopt == 'f')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
							"Unknown option character `\\x%x'.\n",
							optopt);
				return 1;
			default:
				break;
		}

	printf ("readfile %s\n", cvalue);

	string in_filename(cvalue);
	string out_filename(cvalue);
	out_filename += ".out";

	parser(objList, in_filename);

	// solver
	CSubmission s;

	s.total_libraries = objList.size();


	if (s.total_libraries < DATASET_GRANULARITY) 
	{
		NB_CLIENTS = 1;
	}
	else 
	{
		NB_CLIENTS = (int)(s.total_libraries / DATASET_GRANULARITY);
		if (NB_CLIENTS*DATASET_GRANULARITY!=s.total_libraries) {
			NB_CLIENTS++;
		}
	}


	for (int i = 0; i < NB_CLIENTS; i++) {
		mythreads.push_back(std::thread (thr, i));
	}

	auto originalthread = mythreads.begin();

	//end here
	while (originalthread != mythreads.end())
	{
		originalthread->join();
		originalthread++;
	}

	// max gain time is money 
	sort(objList.begin(), objList.end(), sort_by_score);

	for (int i = 0;i < objList.size(); i++) {
		s.library.push_back(objList[i].lib_index);
		s.book.push_back(objList[i].book);
	}

	write_output(s, out_filename);

	return 0;
}

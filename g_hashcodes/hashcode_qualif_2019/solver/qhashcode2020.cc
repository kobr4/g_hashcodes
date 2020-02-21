/*
Author: Michel HE, Eric BELLONI, Jamal KHEYYAD, Nicolas MY
   GOOGLE HASHCODE 2020 Qualif. ROUND - RUN ON AMD 1950X
*/
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


#define DEBUG_PRINT cout
#define DEBUG_INFO cout
#define DEBUG_ERROR cout

#define MAX_SCORE_RC 500000
#define UNDEF_VALUE 0xffffffff

using namespace std;

// Global params
int NumberObj = 0;
int NumberTarget = 0;
int num_books = UNDEF_VALUE, num_libraries = UNDEF_VALUE, num_days = UNDEF_VALUE;
bool gflag = false;

// new orders
vector<int> books_order;
vector<int> libraries_order;

// *** Define Class

class CLibrary
{
public:
  int lib_index;
  int score;
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

        score = sum;
    }
};

vector<CLibrary> objList;

struct s_sort_days_for_signup
{
	bool operator()(const CLibrary &left, const CLibrary &right)
	{
		return (left.days_for_signup < right.days_for_signup);
	}
} sort_days_for_signup;

struct s_sort_book_ship_by_day
{
	bool operator()(const CLibrary &left, const CLibrary &right)
	{
		return (left.book_ship_by_day < right.book_ship_by_day);
	}
} sort_book_ship_by_day;

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
    vector<int> library;
    vector<vector<int>> book;

    bool operator< (const CSubmission& userObj) const
    {
        if(userObj.total_libraries < this->total_libraries)
            return true;
        else return false;
    }
};




void write_output(CSubmission &sub, string out_filename) {
    
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

    printf("OUT= %d \n",sub.book[0][0]);
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

void uniq(vector<string> &v)
{
	auto end = v.end();
	for (auto it = v.begin(); it != end; ++it) {
		end = remove(it + 1, end, *it);
	}

	v.erase(end, v.end());
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
        cout << "order :" << endl;
        for (vector<string>::iterator it = v.begin(); it != v.end(); ++it, ++param)
        {
                books_order.push_back(stoi(*it));
                cout << stoi(*it) << " " ;
        }
        cout << endl;
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


int main(int argc, char** argv)
{

  int bflag = 0;
  int hflag = 0;
  char *cvalue;
  int index;
  int c;

  opterr = 0;

  while ((c = getopt (argc, argv, "hbf:")) != -1)
    switch (c)
      {
      case 'h':
        hflag = 1;
        break;
      case 'b':
        bflag = 1;
        break;
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

    printf ("bigdata = %d, readfile %s\n", bflag, cvalue);

    string in_filename(cvalue);
    string out_filename(cvalue);
    out_filename += ".out";

    parser(objList, in_filename);
    
    CSubmission s;

    s.total_libraries = objList.size();


    for (int i = 0;i < objList.size(); i++) {
        objList[i].lib_index = i;
        objList[i].doScore();
    }

    sort(objList.begin(), objList.end(), sort_by_score);

    printf("OUT= %d \n",objList[0].book[0]);

    for (int i = 0;i < objList.size(); i++) {
        s.library.push_back(objList[i].lib_index);
        s.book.push_back(objList[i].book);
    }
    
    // lack of a sort for s.library by sort_days_for_signup
    
    printf("OUT= %d \n",s.book[0][0]);

    write_output(s, out_filename);

    return 0;
}

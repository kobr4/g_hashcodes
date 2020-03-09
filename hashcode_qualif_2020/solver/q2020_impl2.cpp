
// MULTITHREADED SOLVER GOOGLE HASHCODE 2020 - AMD EDITION
// RUN ON AMD THREADRIPPER 1950X FOR COMPETITION
// Authors : Michel HE, Eric BELLONI, Jamal KHEYYAD, Nicolas MY
// The +26M points solver in Google Hashcode 2020
// g++ -lpthread -lm -o q2020 q2020_impl2.cpp

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
#include <set>
#include <cmath>
#include <thread>

#define DATASET_GRANULARITY 1000

using namespace std;

int NB_CLIENTS;
int MaxOccur=20;
vector<thread> mythreads;

template <class C, class It> It erase_with_last_swap(C &cont, It it) {
    if (&(*it) == &(cont.back()))
    {
        cont.pop_back();
        return end(cont);
    }
    swap(*it, cont.back());
    cont.pop_back();
    return it;
}

template<typename KeyType, typename ValueType>
std::pair<KeyType,ValueType> get_max( const std::map<KeyType,ValueType>& x ) {
    using pairtype=std::pair<KeyType,ValueType>;
    return *std::max_element(x.begin(), x.end(), [] (const pairtype & p1, const pairtype & p2) {
        return p1.second < p2.second;
    });
}

class Book
{
public:
    long long id;
    long long s;
    bool is_sent = false;

    Book(long long a = 0, long long b = 0) {
        this->id = a;
        this->s = b;
    }

    long long score() const {
        if(is_sent) return 0;

        return s;
    }

    friend ostream& operator<<(ostream& os, const Book& b) {
        os << "Book " << b.id << ", " << b.s << ", " << b.is_sent;
        return os;
    }

};

map<int, int> FamousBook;

class Library
{
public:
    long long id;
    double allscore;
    long long number_of_books;
    long long number_of_days_for_signup;
    long long number_of_books_sent_by_day;

    vector<Book *> books_in_librairy;

    Library(long long i = 0, long long a = 0, long long b = 0, long long c = 0) {
        this->id = i;
        this->number_of_books = a;
        this->number_of_days_for_signup = b;
        this->number_of_books_sent_by_day = c;
    }

    Library(Library *l, long long days) {
        this->id = l->id;

        this->number_of_days_for_signup = l->number_of_days_for_signup;
        this->number_of_books_sent_by_day = l->number_of_books_sent_by_day;

        long long available_books_to_send = min(max((days - number_of_days_for_signup), (long long)0) * number_of_books_sent_by_day, (long long)l->books_in_librairy.size());
        long long i = 0;
        for(auto it = l->books_in_librairy.begin(); it != l->books_in_librairy.end() && i <= available_books_to_send; ++it) {
            //send book
            if((*it)->score() == 0) continue;

            (*it)->is_sent = true;
            this->books_in_librairy.push_back(*it);
            ++i;
        }

        this->number_of_books = i;
    }

    void add_book(Book *p) {
        books_in_librairy.push_back(p);
    }

    void order_books() {
        struct compareBooks{
            bool operator()(const Book *a, const Book *b)
            {
                return (a->score() > b->score());
            }
        } compare;

        sort(books_in_librairy.begin(), books_in_librairy.end(), compare);
    }

    friend ostream& operator<<(ostream& os, const Library& l) {
        os << "Librairie " << l.id << "." << endl;
        os << "\tnumber of books : " << l.number_of_books << " [" << l.books_in_librairy.size() << "]." << endl;
        os << "\tnumber of days for sign up : " << l.number_of_days_for_signup << "." << endl;
        os << "\tnumber of books sent by day : " << l.number_of_books_sent_by_day << "." << endl;

        /*for(long long i = 0; i < l.books_in_librairy.size(); ++i) {
            os << *(l.books_in_librairy[i]);
            if(i < l.books_in_librairy.size() - 1) os << endl;
        }*/

        return os;
    }

    long long score(long long days_left) const {
        long long number_of_books_sent = max((days_left - number_of_days_for_signup + 1), (long long)0) * number_of_books_sent_by_day;

        //Prerequis, tri des livres par ordre decroissant
        //Si le livre est deja envoye, ne l'envoi pas
        long long score = 0;
        long long i = 0;
        for(auto it = books_in_librairy.begin(); it != books_in_librairy.end() && (i <= number_of_books_sent); ++it) {
            long long book_score = (*it)->score();
            if(book_score == 0) continue;

            score += (*it)->score();
            // Bonus si le livre est rare
            if (FamousBook[(*it)->id]<2) score++;
            ++i;
        }
        score += books_in_librairy.size();
        return score;
    }

    void doScore(long long days_left) {
        long long non_normalized_score = score(days_left);

        // Eviter la perte de precision des grands nombres
        if(this->number_of_days_for_signup > 0) allscore = (double)non_normalized_score / (double)this->number_of_days_for_signup;
        else allscore = 0;
    }



};

struct Base
{
    vector<Library *> mylibrary;
    int days;
};

class PDefinitions
{
public:
    long long number_of_librairies;
    long long number_of_books;
    long long number_of_days;

    vector<Book> books_definition;
    vector<Library> librairies_definition;
    vector<Library> solution;

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

    void parser(string in_filename) {
        string line;
        vector<string> v;
        ifstream myfile(in_filename);
        if (myfile.is_open()) {
            //read header
            getline(myfile, line);
            vector<string> v = split(line, ' ');

            if(v.size() != 3) throw runtime_error("error reading header line.");
            number_of_books = stoll(v[0]);
            number_of_librairies = stoll(v[1]);
            number_of_days = stoll(v[2]);

            //Read books definition
            getline(myfile, line);
            v = split(line, ' ');
            if(v.size() != number_of_books) throw runtime_error("error reading books defintion.");

            int n = 0;
            for(long long i = 0; i < number_of_books; ++i) {
                Book b(n, stoll(v[i]));
                books_definition.push_back(b);
                n++;
            }

            n = 0;
            for(long long i = 0; i < number_of_librairies; i++) {
                getline(myfile, line);
                v = split(line, ' ');
                if(v.size() != 3) throw runtime_error("error reading librairy defintion.");
                Library l(n, stoll(v[0]), stoll(v[1]), stoll(v[2]));
                //read books in library
                getline(myfile, line);
                v = split(line, ' ');
                if(v.size() != l.number_of_books) throw runtime_error("error reading books in librairy.");
                for (vector<string>::iterator it = v.begin(); it != v.end(); ++it) {
                    long long indice = stoll(*it);
                    l.add_book(&books_definition[indice]);
                    if (!FamousBook.count(indice)) {
                        FamousBook.insert(make_pair(indice, 1));
                    }
                    else {
                        FamousBook[indice]++;
                        //cout << "book.id :" << indice << " times " << FamousBook[indice] << endl;
                    }
                }
                l.order_books();
                librairies_definition.push_back(l);
                n++;
            }
            auto max=get_max(FamousBook);
            std::cout << max.first << "=>" << max.second << std::endl;
            MaxOccur = max.second;
            myfile.close();
        }
    }


    static void thr(int num_thr, std::shared_ptr<Base> p)
    {
        int begin_seg = num_thr * DATASET_GRANULARITY;
        int end_seg;
        std::shared_ptr<Base> lp = p;
        vector<Library *> library = lp->mylibrary;
        int number_of_days_left = lp->days;

        if (num_thr == (NB_CLIENTS-1))
        {
            //cout << "last segment " << library.size() << endl;
            end_seg = library.size();
        }
        else
        {
            end_seg = begin_seg + DATASET_GRANULARITY;
        }

        //cout << "num_thr = " << num_thr << " fr " << begin_seg << " to " << end_seg << std::endl;

        for (vector<Library *>::iterator it = library.begin()+begin_seg; it != library.begin()+end_seg; ++it)
        {
            Library *a = *it;
            a->doScore(number_of_days_left);
        }
    }


    long long solve() {
        long long number_of_days_left = number_of_days;
        int ithread;
        vector<Library *> library_left;
        std::shared_ptr<Base> p = std::make_shared<Base>();

        solution.clear();
        library_left.clear();
        for(auto it = librairies_definition.begin(); it != librairies_definition.end(); ++it) {
            library_left.push_back(&(*it));
        }

        struct compareLibraries{
            long long days_left = 0;
            bool operator()(Library *a, Library *b)
            {
                return ( a->allscore < b->allscore );
            }
        } compare;

        //cout << "Starting pool : " << library_left.size() << "[" << number_of_days_left << "]." << endl;
        while(library_left.size() > 0 && number_of_days_left > 0) {
            compare.days_left = number_of_days_left;
            p->mylibrary = library_left;
            p->days = number_of_days_left;

            //multi-threading
            int s_total_libraries = library_left.size();


            if (s_total_libraries < DATASET_GRANULARITY) {
                NB_CLIENTS = 1;
            }
            else
            {
                NB_CLIENTS = (int)(s_total_libraries / DATASET_GRANULARITY);
                if (NB_CLIENTS*DATASET_GRANULARITY!=s_total_libraries) {
                    NB_CLIENTS++;
                }
            }

            mythreads.clear();

            for (ithread = 0; ithread < NB_CLIENTS; ithread++) {
                mythreads.push_back(thread(thr, ithread, p));
            }

            auto originalthread = mythreads.begin();

            //multi-threading end here
            while (originalthread != mythreads.end())
            {
                originalthread->join();
                originalthread++;
            }


            auto max_elt = max_element(library_left.begin(), library_left.end(), compare);

            if(max_elt != library_left.end()) {
                if((*max_elt)->number_of_days_for_signup > number_of_days_left) break;

                Library first_pick(*max_elt, number_of_days_left);
                solution.push_back(first_pick);
                number_of_days_left -= (*max_elt)->number_of_days_for_signup;
                erase_with_last_swap(library_left, max_elt);
            } else {
                break;
            }
        }

        /*cout << "Solution :" << endl;
        for(auto it = solution.begin(); it != solution.end(); ++it) {
            cout << *it << endl;
        }*/

        long long final_result = 0;
        for(auto it = solution.begin(); it != solution.end(); ++it) {
            for(auto it2 = it->books_in_librairy.begin(); it2 != it->books_in_librairy.end(); ++it2) {
                final_result += (*it2)->s;
            }
        }
        //cout << "Total : " << final_result << "." << endl;
        return final_result;
    }

    friend ostream& operator<<(ostream& os, const PDefinitions& p) {
        os << "Number of librairies : " << p.number_of_librairies << "." << endl;
        os << "Number of books : " << p.number_of_books << "." << endl;
        os << "Number of days : " << p.number_of_days << "." << endl;

        for(long long i = 0; i < p.librairies_definition.size(); ++i) {
            os << p.librairies_definition[i];
            if(i < p.librairies_definition.size() - 1) os << endl;
        }

        return os;
    }

    void write_output(string out_filename) {
        ofstream outfile;
        outfile.open(out_filename);

        outfile << solution.size() << endl;
        for(long long i = 0; i < solution.size(); i++) {
            outfile << solution[i].id << " " << solution[i].books_in_librairy.size() << endl;
            for(long long j = 0; j  < solution[i].books_in_librairy.size(); ++j) {
                outfile << solution[i].books_in_librairy[j]->id;
                if(j < solution[i].books_in_librairy.size() - 1) outfile << " ";
            }
            outfile << endl;
        }
        outfile.close();
    }
};


int main(int argc, char** argv)
{
    vector<string> filenames = {"a_example.txt", "b_read_on.txt", "c_incunabula.txt", "d_tough_choices.txt", "e_so_many_books.txt", "f_libraries_of_the_world.txt"};
    //vector<string> filenames = {"a_example.txt"};
    for(auto it = filenames.begin(); it != filenames.end(); ++it) {
        PDefinitions def;
        def.parser(*it);
        cout << "Problem " << *it << " : " << def.solve() << endl;
        def.write_output(*it + ".out");
    }

    return 0;
}

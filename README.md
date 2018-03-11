# Google Hashcode codes


## Hashcode Qualification 2018

** Car rides scheduling problem. **

Multiples attempts. Starting from the rides is an ok anf fast solution but best score was obtainted by looking both the best fit ride for the vehicles.
The best solver is "qhashcode2018_simple.cpp" albeit a long execution time up to 10 minutes on an AMD Threadripper 1950X (not multithreaded).


| Input                     |  Standard      | Specific tuning|
|---------------------------|----------------|----------------|
| a_example.in              | 10             | 10             |
| b_should_be_easy.in       | 173452         | 176877         |
| c_no_hurry.in             | 15474709       | 15814765       |
| d_metropolis.in           | 11635998       | 11677987       |
| e_high_bonus.in           | 21395951       | 21435975       |
| total                     | 48680120       | 49105614       | 


## Hashcode Qualification 2017

** Video caches against clients optimization. **

CPP solver that did not make it to Google Hashcode 2017 qualification contest

Dynamic solution with served request propagation/flagging

| Input                     |  SolverR       | SolverR + prop | SolverR + prop x2 |
|---------------------------|----------------|----------------|-------------------|
| trending_today.in         |  346999        | 499969         | 499989            |
| videos_worth_spreading.in | 533895         | 538999         | 539011            |
| kittens.in                | 664952         | 842618         | 843235            |
| me_at_the_zoo.in          | 454342         | 484496         | 485719            |
| total / theoritical rank  | 2000188 #~600 | 2366082 #~215  | 2367954 #~215      |


## Hashcode Qualification 2016

** Drone delivery travel optimization. **

Greedy solver that starts from de delivery order and looks for the nearest warehouse that has the product and the nearest available drone, delivery starts after each order.

| Input                      | Deliver on each load| Deliver on each order| 
|----------------------------|---------------------|----------------------|
| mother_of_all_warehouses.in|  58683              | 70192                |         
| busy_day.in | 533895       | 64162               | 86350                |
| redundancy.in              | 76971               | 87949                | 
| total score                | 199816              | 244491               | 
| Theoritical rank#cut-off   | 338/1054#58         | 203/1054#58          | 

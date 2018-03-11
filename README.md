# Google Hashcode codes


## Hashcode Qualification 2018

**Car rides scheduling problem.**

Multiples attempts. Starting from the rides is an ok and fast solution but best score was obtainted by looking for the best fit ride/vehicle at each step.
The best solver is "qhashcode2018_simple.cpp", albeit a long execution time up to 10 minutes on an AMD Threadripper 1950X (not multithreaded).


| Input                     |  Standard        | Specific tuning  |
|---------------------------|------------------|------------------|
| a_example.in              | 10               | 10               |
| b_should_be_easy.in       | 173 452          | 176 877          |
| c_no_hurry.in             | 15 474 709       | 15 814 765       |
| d_metropolis.in           | 11 635 998       | 11 677 987       |
| e_high_bonus.in           | 21 395 951       | 21 435 975       |
| Total / Theoritical rank  | 48 680 120 # 268 | 49 105 614 # 170 | 


## Hashcode Qualification 2017

**Video caches against clients optimization.**

CPP solver that did not make it to Google Hashcode 2017 qualification contest

Dynamic solution with served request propagation/flagging

| Input                     |  SolverR       | SolverR + prop | SolverR + prop x2 |
|---------------------------|----------------|----------------|-------------------|
| trending_today.in         |  346999        | 499969         | 499989            |
| videos_worth_spreading.in | 533895         | 538999         | 539011            |
| kittens.in                | 664952         | 842618         | 843235            |
| me_at_the_zoo.in          | 454342         | 484496         | 485719            |
| Total / Theoritical rank  | 2000188 #~600 | 2366082 #~215  | 2367954 #~215      |


## Hashcode Qualification 2016

**Drone delivery travels optimization.**

Greedy solver that starts from de delivery order and looks for the nearest warehouse that has the product and the nearest available drone, delivery starts after each order.

| Input                      | Deliver on each load| Deliver on each order| 
|----------------------------|---------------------|----------------------|
| mother_of_all_warehouses.in|  58683              | 70192                |         
| busy_day.in | 533895       | 64162               | 86350                |
| redundancy.in              | 76971               | 87949                | 
| total score                | 199816              | 244491               | 
| Theoritical rank#cut-off   | 338/1054#58         | 203/1054#58          | 

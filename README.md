# Google Hashcode codes

## Hashcode Qualification 2020

**Book scanning problem.**

This year Google has invited us to plan books to get scanned from libraries. To maximize the total score of all scanned books, each library needs to be signed up before it can ship books. Our heartbeat rhythms reach the highest level there, towards the hall of fame. After 3 hours of organized work, we ran the program and successfully reached 26 M points. Satisfaction !  We've done it again with our euphoria excitement.

| Input                         |  Score         | 
|-------------------------------|----------------|
| a_example.txt                 | 21             |
| b_read_on.txt                 | 5 822 900      |
| c_incunabula.txt              | 5 690 246      |
| d_tough_choices.txt           | 5 040 490      | 
| e_so_many_books.txt           | 5 060 744      | 
| f_libraries_of_the_world.txt  | 5 308 034      | 
| Total / Theoritical rank      | 26 922 435 #218|  

## Hashcode Qualification 2019

**Photo album sorting problem.**

After 3 hours of intense work we finally break free. Our target is to make a simple Greedy algorithm to use the parallelism processing provided by a powerful AMD multi-cores Threadripper 1950X CPU. This year the Google problem is to sort photo album, and make each keyword of photo to match each other. Then, we make it, 2 versions will finally be done, one in Scala language and the other in C language. Run datasets for only 7 minutes on AMD machine. Hit the score board, having cross the 1M points bar. Great ! Classed after 200th. Satisfied but a bit disappointed since we have missed the top 200 world class team.

Algorithm : 
- Merge 2 vertical photos by choosing photos with the least common tags
- Greedy by looking through the whole dataset for photos that will score the higher (processing-heavy), in case of equality, take the photo that have least amount of tags (to save the "connecting" photos)  

| Input                     |  Score         | 
|---------------------------|----------------|
| a_example.txt             | 2              |
| b_lovely_landscapes.txt   | 224 874        |
| c_memorable_moments.txt   | 1 555          |
| d_pet_pictures.txt        | 434 796        | 
| e_shiny_selfies.txt       | 412 411        | 
| Total / Theoritical rank  | 1 073 638 # 36 |  

## Hashcode Qualification 2018

**Car rides scheduling problem.**

Multiples attempts. Starting from the rides is an ok and fast solution but best score was obtainted by looking for the best fit ride/vehicle at each step.
The best solver is "qhashcode2018_simple.cpp", albeit a long execution time up to 10 minutes on an AMD Threadripper 1950X. The codes had been executed at precisely 5:40 PM before deadline of 6:00 PM. We were confident because thanks to AMD we have the best desktop computer in the current state of the art. We expected as for each input data set, the best solution can be created by using specific tuning parameters.


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

Greedy solver that starts from received delivery orders and looks for the nearest warehouse that has the product and the nearest available drone, delivery starts after each order.

| Input                      | Deliver on each load| Deliver on each order| 
|----------------------------|---------------------|----------------------|
| mother_of_all_warehouses.in|  58683              | 70192                |         
| busy_day.in | 533895       | 64162               | 86350                |
| redundancy.in              | 76971               | 87949                | 
| total score                | 199816              | 244491               | 
| Theoritical rank#cut-off   | 338/1054#58         | 203/1054#58          | 


## Hashcode Qualification 2015

**Optimize a data center.**

Given a schema of a data center and a list of available servers, the task is to optimize the layout of the data center to maximize its availability. We have written the solver in js without having made accurately measurement of the code execution. The results arrived at a score of around 383, either at the 30th place of ladder.

# Google Hashcode codes

## Hashcode Qualification 2022

**Mentorship and Teamwork.**

Finding candidates to fulfill projects sounds simple, but the solution is a lot more complex than you might think because of several dimensions of a project and the skill levels of contributors. The first question we have during the competition is how can one know the date to start a project without knowing in advance the availability of each team member, and how to "upgrade" a developer's skill waiting for the next project. We need to sort the projects which have a shorter duration and especially which require lesser skills. All these dimensions should be explored by our algorithm, how and how much time needed to explorer all possibilities ? Moreover, reading the description of the Google Hashcodes problems in 15 minutes are really intensely difficult, the realization of the coding which spans not more than 3 hours. With our latest version of the algorithm, finally we've reached a very good score among the top teams of the world, it's just huge. We've done it again on our AMD Threadripper CPU. Job done, now feeling totally relaxed !

| Input                         |  Score         | 
|-------------------------------|----------------|
| a_an_example.in.txt           | 30             |
| b_better_start_small.in.txt   | 550 048        |
| c_collaboration.in.txt        | 199 497        |
| d_dense_schedule.in.txt       | 112 947        |
| e_exceptional_skills.in.txt   | 1 639 934      |
| f_find_great_mentors.in.txt   | 354 527        |
| Total  / Theoritical rank     | 2 856 983      |

## Hashcode Qualification 2021

**Traffic light scheduling.**

Let's review the famous scheduling method about the traffic light for cities. The system is made up of cyclic and periodic red/green lights in order to let cars run on streets and stop at each intersection. Some roads are empty, while the others are jammed. The cars run along a network of main roads connecting diffrent blocks of cities. They have to go from a start point to the end point. The traffic lights commute all around the city in same time. The purpose of our algorithm actually is to change switching cadences of the intersection lights, especially those which connect to the main axis of the city that higher traffic occurs. We made it just in time. Over 9 Millions points had been reached, run our program on the AMD Threadripper 1950X CPU. Hard work always brings satisfaction !

| Input                         |  Score         | 
|-------------------------------|----------------|
| a_example.txt                 |     2 000      |
| b_by_the_ocean.txt            | 4 566 349      |
| c_checkmate.txt               | 1 292 584      |
| d_daily_commute.txt           | 1 571 622      | 
| e_etoile.txt                  |   680 987      | 
| f_forever_jammed.txt          | 1 082 956      | 
| Total / Theoritical rank      | 9 196 498      | 

## Hashcode Qualification 2020

**Book scanning problem.**

This year Google has invited the best programmers of the world to plan the books to get scanned from libraries. To maximize the total score of all scanned books, each library needs to be signed up before it can ship books. Our heartbeat rhythms reach the highest level there, towards the hall of fame. After 3 hours of organized work, we ran the program and successfully reached 26 M points. Satisfaction !  We've done it again on the AMD Threadripper 1950X CPU with our euphoria excitement.

| Input                         |  Score         | 
|-------------------------------|----------------|
| a_example.txt                 |        21      |
| b_read_on.txt                 | 5 822 900      |
| c_incunabula.txt              | 5 690 246      |
| d_tough_choices.txt           | 5 040 490      | 
| e_so_many_books.txt           | 5 060 744      | 
| f_libraries_of_the_world.txt  | 5 308 034      | 
| Total / Theoritical rank      | 26 922 435 #218|  

## Hashcode Qualification 2019

**Photo album sorting problem.**

After 3 hours of intense work we finally break free. Our target is to make a simple Greedy algorithm to use the parallelism processing provided by a powerful AMD multi-cores Threadripper 1950X CPU. This year the Google problem is to sort photo album, and make each keyword of photo to match each other. Then, we make it, 2 versions will finally be done, one in Scala language and the other in C language. Run datasets for only 7 minutes on AMD 1950X CPU. Hit the score board, having cross the 1M points bar. Great ! Classed after 200th. Satisfied but a bit disappointed since we have missed the top 200 world class team.

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
The best solver is "qhashcode2018_simple.cpp", albeit a long execution time up to 10 minutes on an AMD Threadripper 1950X CPU. The codes had been executed at precisely 5:40 PM before deadline of 6:00 PM. We were confident because thanks to AMD we have the best desktop computer in the current state of the art. We expected as for each input data set, the best solution can be created by using specific tuning parameters.


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
| trending_today.in         | 346999         | 499969         | 499989            |
| videos_worth_spreading.in | 533895         | 538999         | 539011            |
| kittens.in                | 664952         | 842618         | 843235            |
| me_at_the_zoo.in          | 454342         | 484496         | 485719            |
| Total / Theoritical rank  | 2000188 #~600  | 2366082 #~215  | 2367954 #~215     |


## Hashcode Qualification 2016

**Drone delivery travels optimization.**

Greedy solver that starts from received delivery orders and looks for the nearest warehouse that has the product and the nearest available drone, delivery starts after each order.

| Input                      | Deliver on each load| Deliver on each order| 
|----------------------------|---------------------|----------------------|
| mother_of_all_warehouses.in| 58683               | 70192                |         
| busy_day.in | 533895       | 64162               | 86350                |
| redundancy.in              | 76971               | 87949                | 
| total score                | 199816              | 244491               | 
| Theoritical rank#cut-off   | 338/1054#58         | 203/1054#58          | 


## Hashcode Qualification 2015

**Optimize a data center.**

Given a schema of a data center and a list of available servers, the task is to optimize the layout of the data center to maximize its availability. We have written the solver in js without having made accurately measurement of the code execution. The results arrived at a score of around 383, either at the 30th place of ladder.

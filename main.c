#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM_TASKS 5
#define NUM_SYSTEMS 3
struct task
{
    int id;
    int runtime;
    int rank;
};
struct system
{
    int id;
    int current_time;
};
struct taskgraph
{
    int num_tasks;
    int** dependencies;
    struct task* tasks;
};

struct taskgraph createtaskgraph(){
    struct taskgraph graph;
    graph.num_tasks = NUM_TASKS;
    graph.dependencies = malloc(NUM_TASKS*sizeof(int*));
    graph.tasks = malloc(NUM_TASKS*sizeof(struct task));
    for(int i=0;i<NUM_TASKS;i++){
        graph.tasks[i].id=i;
        graph.tasks[i].runtime=(rand()%10)+1;
        graph.dependencies[i] = calloc(NUM_TASKS,sizeof(int));
    }
    graph.dependencies[0][1] = 1;
    graph.dependencies[1][2] = 1;
    graph.dependencies[0][3] = 1;
    graph.dependencies[3][4] = 1;
};


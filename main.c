#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM_TASKS 5
#define NUM_SYSTEMS 3
// user defined struct for tasks
struct task
{
    int id;
    int runtime;
    int rank;
};
//user defined struct for systems
struct system
{
    int id;
    int current_time;
};
//user defined taskgraph which is basically a directed acyclic graph
struct taskgraph
{
    int num_tasks;
    int** comm_cost;
    int** dependencies;
    struct task* tasks;
};
// making our task graph
struct taskgraph createtaskgraph(){
    struct taskgraph graph;
    graph.num_tasks = NUM_TASKS;
    graph.dependencies = malloc(NUM_TASKS*sizeof(int*));
    graph.tasks = malloc(NUM_TASKS*sizeof(struct task));
    graph.comm_cost =  malloc(NUM_TASKS*sizeof(int*));
    // we now randomly assign runtimes to each of the tasks between 1 and 10
    for(int i=0;i<NUM_TASKS;i++){
        graph.tasks[i].id=i;
        graph.tasks[i].runtime=(rand()%10)+1;
        graph.dependencies[i] = calloc(NUM_TASKS,sizeof(int));
        graph.comm_cost[i] = malloc(NUM_TASKS*sizeof(int));
        for(int j=0;j<NUM_TASKS;j++){
            graph.comm_cost[i][j] = (rand()%5)+1;
        }
    }
    // these are dependencies of the graph and the task scheduler essentially signifying which task will be executed when
    graph.dependencies[0][1]=1;
    graph.dependencies[1][2]=1;
    graph.dependencies[0][3]=1;
    graph.dependencies[3][4]=1;
    return graph;
};
void calculaterank(struct taskgraph *tg){
    for(int i=0;i<tg->num_tasks;i++){
        tg->tasks[i].rank=tg->tasks[i].runtime;
        for(int j=0;j<tg->num_tasks;j++){
            int rk_ck =tg->tasks[j].rank + tg->tasks[j].runtime + tg->comm_cost[i][j];
            if(tg->dependencies[i][j]>0 && tg->tasks[i].rank<rk_ck){
                tg->tasks[i].rank = rk_ck;
            }
        }
    }
  
}
void heftscheduler(struct taskgraph* graph, struct system systems[NUM_SYSTEMS]) {
    calculaterank(graph);
    for (int i = 0; i < graph->num_tasks; i++) {
        int min_time = -1;
        int min_proc = -1;
        for (int j = 0; j < NUM_SYSTEMS; j++) {
            int start_time = systems[j].current_time;
            int finish_time = start_time + graph->tasks[i].runtime;
            if (min_time == -1 || finish_time < min_time) {
                min_time = finish_time;
                min_proc = j;
            }
        }
        systems[min_proc].current_time = min_time;
        printf("Task %d scheduled on System %d\n", graph->tasks[i].id, systems[min_proc].id);
    }   
}
int main() {
    srand(time(NULL));
    struct taskgraph graph = createtaskgraph();
    struct system systems[NUM_SYSTEMS];
    for (int i = 0; i < NUM_SYSTEMS; i++) {
        systems[i].id = i;
        systems[i].current_time = 0;
    }
    printf("Task Scheduling:\n");
    heftscheduler(&graph, systems);
    for (int i = 0; i < NUM_TASKS; i++) {
        free(graph.dependencies[i]);
    }
    free(graph.dependencies);
    free(graph.tasks);
    return 0;
}



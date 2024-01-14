#include<stdio.h>
#include<stdlib.h>
#include <windows.h>
#include <psapi.h>
// here we initially declare our data structure task
typedef struct task{
	int* comp_cost;
	double rank;
	int id;
	int* EST;
	int* EFT;
	int* comm_cost; //graph of communication cost dependencies
	int* parent;
}task;
//function to generate task graph in dot format
void generatetaskgraph(task *tasks, int num_tasks) {
    FILE *dotFile = fopen("task_graph.dot", "w");
    fprintf(dotFile, "digraph TaskGraph {\n");
    // Create nodes for each task
    for (int i = 0; i < num_tasks; i++) {
        for (int j = 0; j < num_tasks; j++) {
            if (tasks[i].comm_cost[j] != 0) {
                fprintf(dotFile, "  %d -> %d [label=\"%d\"]\n", tasks[i].id, tasks[j].id, tasks[i].comm_cost[j]);
            }
        }
    }

    fprintf(dotFile, "}\n");

    fclose(dotFile);
}
//custom comparator used for sorting with qsort
int cmp_rank(const void *a, const void *b) {
		task* t1 = (task*)a;
		task* t2 = (task*)b;
		return (t2->rank) - (t1->rank);
}
int cmp_id(const void *a, const void *b) {
		task* t1 = (task*)a;
		task* t2 = (task*)b;
		return (t1->id) - (t2->id);
}

int find_min_index(int arr[],int n) {
	int temp = arr[0];
	int index = 0;
	// Find the minimum value in the array
	for(int i = 0; i < n; i++) {
		if(temp > arr[i]) {
			temp = arr[i];
			index = i;
		}
	}
	return index;
}
// Find the minimum value in the array
int find_min(int arr[],int n) {
	int temp = arr[0];
	for(int i = 0; i < n; i++) {
		if(temp > arr[i]) {
			temp = arr[i];
		}
	}
	return temp;
}
// Find the index of the task with the given id
int find_id(task arr[], int n,int id) {
	int index = 0;
	for(int i = 0; i < n; i++) {
		if(arr[i].id == id) {
			index = i;
		} 
	}
	return index;
}
// Initialize all the variables
void inittasks(task* tasks, int num_tasks, int num_procs) {
	for (int i = 0; i < num_tasks; i++) {
		tasks[i].id = i + 1;
		tasks[i].comp_cost = (int*)malloc(num_procs * sizeof(int));
		tasks[i].EST = (int*)malloc(num_procs * sizeof(int));
		tasks[i].EFT = (int*)malloc(num_procs * sizeof(int));
		tasks[i].comm_cost = (int*)malloc(num_tasks * sizeof(int));
		tasks[i].parent = (int*)malloc(num_tasks * sizeof(int));
	}
}
// Setup the parent array
void setuppred(task* tasks, int num_tasks) {
	for (int i = 0; i < num_tasks; i++) {
		for (int j = 0; j < num_tasks; j++) {
			if (tasks[i].comm_cost[j] != 0) {
				tasks[j].parent[i] = tasks[i].id;
			}
			else {
				tasks[j].parent[i] = -1;
			}

		}
	}
}
// Calculate rank
void calculaterank(task* tasks, int num_tasks, int num_procs) {
	for (int i = 0; i < num_tasks; i++) {
		tasks[i].rank = 0;

		for (int j = 0; j < num_procs; j++) {
			tasks[i].rank += tasks[i].comp_cost[j];
		}

		tasks[i].rank /= num_procs;
	}
	double temp, temp2 = 0;
    
    // add max of sum of previous ranks and communication cost
    for(int i = num_tasks-2 ; i >= 0; i--) {

    	for(int j = num_tasks-1 ; j >= 0; j--) {
    	
    		if(tasks[i].comm_cost[j] != 0) {
    		temp = tasks[i].rank + tasks[j].rank + tasks[i].comm_cost[j];
    			if(temp2 < temp) {
    				temp2 = temp;
				}
			}
    	}
    	tasks[i].rank = temp2;
    	temp2 = 0;
    }
}
void calculateESTEFT(task* tasks, int num_tasks, int num_procs, int* proc, int* pred_proc, int pred[][num_tasks]) {
	int min = find_min_index(tasks[0].EFT, num_procs);
	pred_proc[0] = min;
	int time = tasks[0].EFT[min];
	proc[min] = time;
	int max = 0;
	int temp_max = 0;
	int p;
	int id = 0;

	// Loop to find EFT and EST
	for (int i = 1; i < num_tasks; i++) {
		for (int j = 0; j < num_procs; j++) {
			for (int k = 0; k < num_tasks; k++) {
				// Find the parent of the task
				p = tasks[i].parent[k];
				// Find the index of the parent task
				id = find_id(tasks, num_tasks, p);
				if (p > 0) {
					// Find the processor on which the parent task is executed
					if (pred_proc[id] == j) {
						temp_max = proc[j];
					}
					else {
						// Find the max of the EFT of the parent task and the communication cost
						temp_max = find_min(tasks[id].EFT, num_procs) + pred[p - 1][tasks[i].id - 1];
					}
					if (temp_max > max) {
						// Find the max of the EFT of all the parent tasks
						max = temp_max;
						if (proc[j] > max) {
							max = proc[j];
						}
					}
				}
			}
			tasks[i].EST[j] = max;
			tasks[i].EFT[j] = tasks[i].EST[j] + tasks[i].comp_cost[j];
			max = 0;
		}
		min = find_min_index(tasks[i].EFT, num_procs);
		pred_proc[i] = min;
		proc[min] = tasks[i].EFT[min];
	}
	//// Loop ends
}
int calcmakespan(task* tasks, int num_tasks, int num_procs) {
	int makespan = 0;
	for (int i = 0; i < num_tasks; i++) {
		for (int j = 0; j < num_procs; j++) {
			if (makespan < tasks[i].EFT[j]) {
				makespan = tasks[i].EFT[j];
			}
		}
	}
	return makespan;
}
void generatetimeline(task *tasks, int num_tasks, int num_procs) {
    FILE *dotFile = fopen("timeline_graph.dot", "w");
    fprintf(dotFile, "digraph TimelineGraph {\n");
    fprintf(dotFile, "  rankdir=TB;\n"); // Make the graph vertical
    fprintf(dotFile, "  nodesep=1.0;\n"); // Increase the distance between nodes of the same rank
    fprintf(dotFile, "  ranksep=2.0;\n"); // Increase the distance between ranks

    // Create subgraphs for each processor
    for (int i = 0; i < num_procs; i++) {
        fprintf(dotFile, "  subgraph cluster_proc%d {\n", i);
        fprintf(dotFile, "    label=\"Processor %d\";\n", i + 1);

        // Nodes for each task on the processor
        for (int j = 0; j < num_tasks; j++) {
            fprintf(dotFile, "    Task%d_p%d [label=\"Task %d\\nEST: %d EFT: %d\"];\n",
                    tasks[j].id, i + 1, tasks[j].id, tasks[j].EST[i], tasks[j].EFT[i]);
        }

        fprintf(dotFile, "  }\n");
    }

    // Add invisible edges to force a vertical layout
    for (int i = 0; i < num_procs - 1; i++) {
        fprintf(dotFile, "  Task%d_p%d -> Task%d_p%d [style=invis];\n", tasks[0].id, i + 1, tasks[0].id, i + 2);
    }

    // Connect nodes based on task dependencies
    for (int i = 0; i < num_tasks; i++) {
        for (int j = 0; j < num_tasks; j++) {
            if (tasks[i].comm_cost[j] != 0) {
                fprintf(dotFile, "  Task%d_p%d -> Task%d_p%d [label=\"%d\"];\n",
                        tasks[i].id, i + 1, tasks[j].id, j + 1, tasks[i].comm_cost[j]);
            }
        }
    }

    fprintf(dotFile, "}\n");
    fclose(dotFile);
}
int main(int argc, char *argv[]) {
	LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
	FILE *input;
	int num_tasks;
	int num_procs;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Open the input file
    input = fopen(argv[1], "r");
    if (input == NULL) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }

    
    // Taking input
    fscanf(input,"%d",&num_tasks);
    fscanf(input,"%d",&num_procs);
    
    int proc[num_procs];
    int pred[num_tasks][num_tasks];
    int pred_proc[num_tasks];
    
    task* tasks;
    
    // Allocate memory to all variables
    tasks = (task*)malloc(num_tasks*sizeof(task));
    inittasks(tasks,num_tasks,num_procs);
    // Taking computation cost input
    for(int i = 0; i < num_procs; i++) {
    	for(int j = 0; j < num_tasks; j++) {
    		fscanf(input,"%d ",&tasks[j].comp_cost[i]);
    	}

    }
    // Taking communication cost input
    for(int i = 0; i < num_tasks; i++) {
    	
    	for(int j = 0; j < num_tasks; j++) {
    		fscanf(input,"%d",&tasks[i].comm_cost[j]);
    		pred[i][j] = tasks[i].comm_cost[j];
    	}
    }
    
    setuppred(tasks,num_tasks);
    // Inititalize processors
    for(int i = 0; i < num_procs; i++) {
    	proc[i] = 0;
    }
    // Calculate rank
	calculaterank(tasks,num_tasks,num_procs);
	// Sort tasks based on rank
	printf("The upward rank of the tasks:\n");
    for(int i = 0; i < num_tasks; i++) {
    	printf("Task %d: %lf\n",tasks[i].id,tasks[i].rank);
    }
    printf("\n");
	qsort(tasks, num_tasks, sizeof(task), cmp_rank);
    // Print tasks in order of execution
    printf("\nThe order of the tasks to be scheduled:\n");
    for(int i = 0; i < num_tasks; i++) {
    	printf("%d ",tasks[i].id);
    }
    printf("\n\n");
	printf("The makespan of the schedule is: %d\n",calcmakespan(tasks,num_tasks,num_procs));
    // Finding EST and EFT for entry task
    for(int i = 0; i < num_procs; i++) {
    	tasks[0].EST[i] = 0;
    	tasks[0].EFT[i] = tasks[0].EST[i] + tasks[0].comp_cost[i];
    }
    
    // Calculate EST and EFT for all tasks
    calculateESTEFT(tasks,num_tasks,num_procs,proc,pred_proc,pred);
    qsort(tasks, num_tasks, sizeof(task), cmp_id);
    FILE *output;
	output = fopen("output.txt","w");
	printf("The order of the tasks to be scheduled:\n");
    for(int i = 0; i < num_tasks; i++) {
    	int ind = find_min_index(tasks[i].EFT,num_procs);
    	printf("Task %d is executed on processor %d from time %d to %d\n",tasks[i].id,ind+1,tasks[i].EST[ind],tasks[i].EFT[ind]);
		fprintf(output,"%d %d\n",tasks[i].id,ind+1);
    }
	printf("\n");
    generatetaskgraph(tasks, num_tasks);
	generatetimeline(tasks, num_tasks, num_procs); 
    fclose(input);
	 QueryPerformanceCounter(&end);
    double interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    printf("Time taken: %f seconds\n", interval);
	printf("\n");
    // Get the memory usage of the current process
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    printf("Memory used: %llu bytes\n", virtualMemUsedByMe);

	return 0;
}
#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <iomanip>

using namespace std;

int sleepcount = 0;
static int index=0;
int n;
int x = 0;
float total_waiting_time = 0;
float total_turnaround_time = 0;
float avg_waiting_time = 0;
float avg_turnaround_time = 0;
int GANTTindex[100], k=0, l=0, GANTTpid[100];
int current_time = 0;
bool check = false;
int anothervar = -1;
int PID[100], PAT[100], PBT[100], PFT[100];
int TAT[100], WT[100];
int RT[100];
sem_t mutex;
sem_t completed;

struct Process{
    int pid;
    int arrival_time;
    int burst_time;
    int waiting_time;
    int turnaround_time;
    int completion_time;
    int remaining_time;
};
vector<Process> processes;
bool checkshortest(Process *p)
{
    bool shortest_remaining_time = true;
    for (int i = 0; i < processes.size(); i++) {
        if (processes[i].remaining_time > 0 && processes[i].arrival_time <= current_time && i != p->pid - 1) {
            if (processes[i].remaining_time < p->remaining_time)
            {
                shortest_remaining_time = false;
                break;
            }
            else if (processes[i].remaining_time == p->remaining_time && processes[i].arrival_time < p->arrival_time)
            {
                shortest_remaining_time = false;
                break;
            }
        }
    }
    return shortest_remaining_time;
}
bool check_process_arrival()
{
    bool arrived = false;
    for (int i = 0; i < processes.size(); i++)
	{
        if (processes[i].remaining_time > 0 && processes[i].arrival_time <= current_time)
        {
            arrived = true;
            break;
        }
    }
    return arrived;
}
void* srtf_algorithm(void* arg)
{
    Process* process = (Process*) arg;
    cout << "Process: P" << process->pid << " has arrived! thread created for P" << process->pid << "\n";
    process->remaining_time = process->burst_time;
    sleepcount--;
    sleep(sleepcount);
    
    
	while (process->remaining_time > 0)
    {	
        if(process->arrival_time <= current_time)
        {
            sem_wait(&mutex);
            bool flag = checkshortest(process);
            if(flag == true)
            {
                process->remaining_time--;
                current_time++;
				process->completion_time = current_time;
            }
            sem_post(&mutex);
        }
        else
		{
           if(!check_process_arrival())
		   {
           		sem_wait(&mutex);
                current_time++;
                sem_post(&mutex);
           }
        }
    }
	
	process->turnaround_time = process->completion_time - process->arrival_time;
    process->waiting_time = process->turnaround_time - process->burst_time;
    
	sem_post(&completed);
    	return NULL;
}
void SRTFGanttChart(){
	int Complete = 0, Current_Time = 0, minimum = 9999999;
    int Smallest = 0;
    bool Check = false;
	int Anothervar = -1;
	
	GANTTindex[k] = Current_Time;
	k++;
	while(Complete != n){
        for(int j=0 ; j<n ; j++){
            if((PAT[j]<=Current_Time) && (RT[j]<minimum) && (RT[j] > 0)){
                minimum = RT[j];
                Smallest = j;
                Check = true;
            }
        }
        if(Check == false){
            Current_Time++;				
        	if(GANTTpid[l-1] == -1){
            	GANTTindex[k-1] = Current_Time;
			}
			else{
				GANTTindex[k] = Current_Time;
		        k++;
		        GANTTpid[l] = -1;
		        l++;
			}
            continue;
        }
        RT[Smallest]--;
        minimum = RT[Smallest];
        if(minimum == 0){
            minimum = 9999999;
		}
        if(RT[Smallest] == 0){
            Complete++;
            Check = false;
            PFT[Smallest] = Current_Time + 1;
            GANTTindex[k] = PFT[Smallest];		
			k++;
    		GANTTpid[l] = PID[Smallest];
			l++;	
        }
        Current_Time++;
        if(Anothervar != Smallest && RT[Anothervar] != 0){
    		if(Anothervar != -1){
	    		GANTTindex[k] = Current_Time-1;
				k++;
    			GANTTpid[l] = PID[Anothervar];
				l++;	
			}
		}
        Anothervar = Smallest;
    }
}
int main(){
	cout << "\n---------------------------------------------------------\n";
	cout << "|\t\tSELECT PREEMPTIVE SCHEDULING ALGORITHM\t|\n";
	cout << "---------------------------------------------------------\n";
	cout << "\nEnter the Number of Processes: ";
	    cin >> n;
	    sleepcount = n;
	    sem_init(&mutex, 0, 1);
	    sem_init(&completed, 0, 0);
	
	    for (int i = 0; i < n; i++)
		{
	        Process p;
	        p.pid = i + 1;
	        PID[i] = p.pid;
	        cout << "Enter Arrival Time and Burst Time for Process " << i + 1 << ": ";
	        cin >> p.arrival_time >> p.burst_time;
	        p.remaining_time = p.burst_time;
	        PAT[i] = p.arrival_time;
	        PBT[i] = p.burst_time;
	        RT[i] = p.remaining_time;
	        processes.push_back(p);
	    }
	    
	    cout<<endl;
	    cout<<"-----------------------------"<<endl;
		cout<<"| Process | Arrival | Burst |"<<endl;
		cout<<"|   ID    |  Time   | Time  |"<<endl;
		cout<<"-----------------------------"<<endl;
		for(int i=0 ; i<n ; i++){
			if(processes[i].pid >= 10){
				cout<<"|   P"<<processes[i].pid<<"   |"<<setw(5)<<processes[i].arrival_time<<"    |"<<setw(4)<<processes[i].burst_time<<"   |"<<endl;
			}
			else{
	    		cout<<"|   P"<<processes[i].pid<<"    |"<<setw(5)<<processes[i].arrival_time<<"    |"<<setw(4)<<processes[i].burst_time<<"   |"<<endl;
			}
	    }
	    cout<<"-----------------------------"<<endl;
	    
	    pthread_t thread_id[n];
	    cout << endl;
	    for (int i = 0; i < n; i++)
		{
	        pthread_create(&thread_id[i], NULL, srtf_algorithm, (void*) &processes[i]);
	    }
	
	    int completed_processes = 0;
	    while (completed_processes < n)
		{
	        sem_wait(&completed);
	        completed_processes++;
	    }
	    
	    sem_destroy(&mutex);
	    sem_destroy(&completed);
	    
	    cout<<endl;
	    cout<<"----------------------------------------------------------------"<<endl;
		cout<<"| Process | Arrival | Burst | Finishing | Turnaround | Waiting |"<<endl;
		cout<<"|   ID    |  Time   | Time  |   Time    |    Time    |  Time   |"<<endl;
		cout<<"----------------------------------------------------------------"<<endl;
	    for (int i = 0; i < n; i++) {
			if(processes[i].pid >= 10){
				cout<<"|   P"<<processes[i].pid<<"   |"<<setw(5)<<processes[i].arrival_time<<"    |"<<setw(4)<<processes[i].burst_time<<"   |"<<setw(6)<<processes[i].completion_time<<"     |"<<setw(7)<<processes[i].turnaround_time<<"     |"<<setw(5)<<processes[i].waiting_time<<"    |"<<endl;
			}
			else{
				cout<<"|   P"<<processes[i].pid<<"    |"<<setw(5)<<processes[i].arrival_time<<"    |"<<setw(4)<<processes[i].burst_time<<"   |"<<setw(6)<<processes[i].completion_time<<"     |"<<setw(7)<<processes[i].turnaround_time<<"     |"<<setw(5)<<processes[i].waiting_time<<"    |"<<endl;
			}
	        total_waiting_time += processes[i].waiting_time;
	        total_turnaround_time += processes[i].turnaround_time;
	    }
		cout<<"----------------------------------------------------------------"<<endl;
		
	    avg_waiting_time = total_waiting_time / n;
	    avg_turnaround_time = total_turnaround_time / n;
	    
	    cout<<endl;
	    cout << "Average Waiting Time    : " << avg_waiting_time << endl;
	    cout << "Average Turnaround Time : " << avg_turnaround_time << endl;
	    
	    SRTFGanttChart();
	    
	    for(int i=0 ; i<n-1; i++){
	    	for(int j=i+1 ; j<n; j++){
				if(processes[j].arrival_time < processes[i].arrival_time){
					Process Temp = processes[i];
					processes[i] = processes[j];
					processes[j] = Temp;
				}
				else if(processes[j].arrival_time == processes[i].arrival_time){
					if(processes[j].pid < processes[i].pid){
						Process Temp = processes[i];
						processes[i] = processes[j];
						processes[j] = Temp;
					}
				}
			}
		}
		
	    cout<<endl;
		cout<<"\t\t--> GANTT Chart <--"<<endl;
		for(int i=0 ; i<l ; i++){
			if(GANTTpid[i] >= 10 || i==l-1){
				cout<<"------";
			}
			else{
				cout<<"-----";
			}
		}
		cout<<endl;
		cout<<" |";
		for(int i=0 ; i<l ; i++){
			if(GANTTpid[i] == -1){
				cout<<" ~~ |";
			}
			else{
				cout<<" P"<<GANTTpid[i]<<" |";
			}
		}
		cout<<endl;
		cout<<" ";
		for(int i=0 ; i<l ; i++){
			if(GANTTpid[i] >= 10 || i==l-1){
				cout<<"------";
			}
			else{
				cout<<"-----";
			}
		}
		cout<<endl;
		for(int i=0 ; i<k ; i++){
			if(i==0){
				cout<<" "<<GANTTindex[i];
			}
			else if(GANTTpid[i-1] >= 10){
				cout<<setw(6)<<GANTTindex[i];
			}
			else{
				cout<<setw(5)<<GANTTindex[i];
			}
		}
		cout<<endl;
	return 0;
}

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<malloc.h>
#include<time.h>
#include<pthread.h>
#include<semaphore.h>
struct Process {
	int Priority,Rtime,Atime,Btime,Qtime,ID,Priority_P;
	clock_t Arrival;
	int Preemtion_Flag,Arrival_Flag,Flag,Completed;
	sem_t se;
	struct Process *Next;
};
typedef struct Process node;
clock_t Start,Count;
pthread_t P[10];
node *Front_P=NULL,*Front_T=NULL,*Rear_T=NULL,*Front_F=NULL,*Rear_F=NULL,*Front_A=NULL,*Temp;
int i=0,n,TQ=10,l=1,P_Qtime_t=0,R_Qtime_t=0,F_Qtime_t=0;
float TAT=0,WT=0;
void Ppush(node *Temp);
void Tpush(node *Temp);
void Fpush(node *Temp);
void *Priority(node *S) {
	int Null_Flag=0;
	while(1) {
		sem_wait(&S->se);				
		if((S->Atime<=(clock()-Start)/CLOCKS_PER_SEC && S->Arrival_Flag==1) || S->Preemtion_Flag==1) {
			S->Arrival_Flag=0;
			S->Preemtion_Flag=0;
			Count=clock();
		} 
		if(S->Flag==1) {
			printf("\nProcess-%d Started \t\t\t\t\tTimer :%d",S->ID,(clock()-Start)/CLOCKS_PER_SEC);
			S->Flag=0;
			S->Arrival=clock();
		}
		if((clock()-Count)/CLOCKS_PER_SEC==1) {
			R_Qtime_t=0;
			F_Qtime_t=0;
			Count=clock();
			printf("\nProcess-%d Running\t\t\t\t\tTimer :%d",S->ID,(clock()-Start)/CLOCKS_PER_SEC);
			S->Rtime-=1;
			P_Qtime_t+=1;
			if(S->Rtime==0) {				
				TAT+=(clock()-Start)/CLOCKS_PER_SEC-S->Atime;	
				WT+=((clock()-Start)/CLOCKS_PER_SEC)-S->Atime-S->Btime;	
				if(Front_P->Next!=NULL) {
						Null_Flag=1;
					if(Front_P->Atime==(clock()-Start)/CLOCKS_PER_SEC){
						Front_P->Next=Front_P->Next->Next;
					}
					else {
						Front_P=Front_P->Next;
					}
					sem_post(&Front_P->se);		
				}
				S->Completed=1;
				printf("\nProcess-%d completed executing, ",S->ID);
				if(Front_P->Next==NULL && Null_Flag==0){
					Front_P=NULL;
					if(Front_T!=NULL) {
						printf("next Process is: %d\nLEVEL CHANGED TO 2",Front_T->ID);
						sem_post(&Front_T->se);
					}	
					else if(Front_F!=NULL) {
						printf("next Process is: %d \nLEVEL CHANGED TO 3",Front_F->ID);
						sem_post(&Front_F->se);
					}
					else {
						l=1;
					}
				}
				else{
					printf("next Process is: %d",Front_P->ID);
				}
			}
			else if(Front_P!=S) {
				printf("\nProcess-%d Context Switched to Process-%d",S->ID,Front_P->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_P->se);
				sem_wait(&S->se);	
			}
		}
		if(P_Qtime_t>=10) {
			if(Front_T!=NULL) {
				P_Qtime_t=0;
				printf("\nProcess-%d Context Switched to Process-%d\nLEVEL CHANGED TO 2",S->ID,Front_T->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_T->se);
				sem_wait(&S->se);
			}	
			else if(Front_F!=NULL) {
				P_Qtime_t=0;
				printf("\nProcess-%d Context Switched to Process-%d\nLEVEL CHANGED TO 3",S->ID,Front_F->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_F->se);
				sem_wait(&S->se);
			}
		}
		if(S->Completed==1) {
			break;
		}
		sem_post(&S->se);	
	}
}
void *RRobin(node *S) {
	int Null_Flag=0;
	while(1) {
		sem_wait(&S->se);	
		if((S->Atime<=(clock()-Start)/CLOCKS_PER_SEC && S->Arrival_Flag==1) || S->Preemtion_Flag==1) {
			S->Arrival_Flag=0;
			S->Preemtion_Flag=0;
			Count=clock();
		} 
		if(S->Flag==1) {
			printf("\nProcess-%d Started \t\t\t\t\tTimer :%d",S->ID,(clock()-Start)/CLOCKS_PER_SEC);
			S->Flag=0;
			S->Arrival=clock();
		}
		if((clock()-Count)/CLOCKS_PER_SEC==1) {	
			P_Qtime_t=0;
			F_Qtime_t=0;
			Count=clock();
			printf("\nProcess-%d Running\t\t\t\t\tTimer :%d",S->ID,(clock()-Start)/CLOCKS_PER_SEC);
			S->Rtime-=1;
			S->Qtime+=1;
			R_Qtime_t+=1;
			if(S->Rtime==0) {					
				TAT+=(clock()-Start)/CLOCKS_PER_SEC-S->Atime;		
				WT+=((clock()-Start)/CLOCKS_PER_SEC)-S->Atime-S->Btime;	
				if(Front_T!=Rear_T) {
					Null_Flag=1;
					Front_T=Front_T->Next;
					sem_post(&Front_T->se);
				}
				S->Completed=1;
				printf("\nProcess-%d completed executing, ",S->ID);
				if(Front_T==Rear_T && Null_Flag==0){				
					Rear_T=NULL;
					Front_T=NULL;
					if(Front_F!=NULL) {
						printf("next Process is: %d\nLEVEL CHANGED TO 3",Front_F->ID);
						sem_post(&Front_F->se);
					}	
					else if(Front_P!=NULL) {
						printf("next Process is: %d\nLEVEL CHANGED TO 1",Front_P->ID);
						sem_post(&Front_P->se);
					}
					else {
						l=1;
					}
				}
				else{
					printf("next Process is: %d",Front_T->ID);
				}
			}
			else if(S->Qtime>=4 && Front_T!=Rear_T) {				
				S->Qtime=0;
				Front_T=Front_T->Next;
				Tpush(S);
				printf("\nProcess-%d Context Switched to Process-%d",S->ID,Front_T->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_T->se);
				sem_wait(&S->se);	
			}
		}
		if(R_Qtime_t>=10) {
			if(Front_F!=NULL) {
				R_Qtime_t=0;
				printf("\nProcess-%d Context Switched to Process-%d\nLEVEL CHANGED TO 3",S->ID,Front_F->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_F->se);
				sem_wait(&S->se);
			}	
			else if(Front_P!=NULL) {
				R_Qtime_t=0;
				printf("\nProcess-%d Context Switched to Process-%d\nLEVEL CHANGED TO 1",S->ID,Front_P->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_P->se);
				sem_wait(&S->se);
			}
		}
		if(S->Completed==1) {			
			break;
		}
		sem_post(&S->se);				
	}
}
void *FCFS(node *S) {
	int Null_Flag=0;
	while(1) {
		sem_wait(&S->se);	
		if((S->Atime<=(clock()-Start)/CLOCKS_PER_SEC && S->Arrival_Flag==1) || S->Preemtion_Flag==1) {
			S->Arrival_Flag=0;
			S->Preemtion_Flag=0;
			Count=clock();
		} 
		if(S->Flag==1) {
			printf("\nProcess-%d Started \t\t\t\t\tTimer :%d",S->ID,(clock()-Start)/CLOCKS_PER_SEC);
			S->Flag=0;
			S->Arrival=clock();
		}
		if((clock()-Count)/CLOCKS_PER_SEC==1) {	
			R_Qtime_t=0;
			P_Qtime_t=0;
			Count=clock();
			printf("\nProcess-%d Running\t\t\t\t\tTimer :%d",S->ID,(clock()-Start)/CLOCKS_PER_SEC);
			S->Rtime-=1;
			F_Qtime_t+=1;
			if(S->Rtime==0) {					
				TAT+=(clock()-Start)/CLOCKS_PER_SEC-S->Atime;		
				WT+=((clock()-Start)/CLOCKS_PER_SEC)-S->Atime-S->Btime;	
				if(Front_F->Next!=NULL) {
				Null_Flag=1;
				if(Front_F->Atime==(clock()-Start)/CLOCKS_PER_SEC){
					Front_F->Next=Front_F->Next->Next;
				}
				else {
					Front_F=Front_F->Next;
				}
				sem_post(&Front_F->se);	
				}
				S->Completed=1;
				printf("\nProcess-%d completed executing, ",S->ID);
				if(Front_F->Next==NULL && Null_Flag==0){
					Front_F=NULL;
					Rear_F=NULL;
					if(Front_P!=NULL) {
						printf("next Process is: %d\nLEVEL CHANGED TO 1",Front_P->ID);
						sem_post(&Front_P->se);
					}	
					else if(Front_T!=NULL) {
						printf("next Process is: %d\nLEVEL CHANGED TO 2",Front_T->ID);
						sem_post(&Front_T->se);
					}
					else {
						l=1;
					}
				}
				else{
					printf("next Process is: %d",Front_F->ID);
				}
			}
		}
		if(F_Qtime_t>=10) {
			if(Front_P!=NULL) {
				F_Qtime_t=0;
				printf("\nProcess-%d Context Switched to Process-%d\nLEVEL CHANGED TO 1",S->ID,Front_P->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_P->se);
				sem_wait(&S->se);
			}	
			else if(Front_T!=NULL) {
				F_Qtime_t=0;
				printf("\nProcess-%d Context Switched to Process-%d\nLEVEL CHANGED TO 2",S->ID,Front_T->ID);
				S->Preemtion_Flag=1;
				sem_post(&Front_T->se);
				sem_wait(&S->se);
			}
		}
		if(S->Completed==1) {			
			break;
		}
		sem_post(&S->se);				
	}
}
void Ppush(node *Temp) {
	node *Start=Front_P;
	if(Front_P==NULL) {
		Front_P=Temp;
		Front_P->Next=NULL;
	}
	else{
		if (Start->Priority > Temp->Priority) {
        	Temp->Next = Front_P;
        	Front_P=Temp;
    	}
    	else {
        	while (Start->Next != NULL && Start->Next->Priority<Temp->Priority) {
            		Start = Start->Next;
        	}
        	Temp->Next = Start->Next;
        	Start->Next = Temp;
    	}	
	}
}
void Fpush(node *Temp) {
	if(Front_F==NULL) {
		Front_F=Temp;
		Rear_F=Temp;
		Temp->Next=NULL;
	}
	else {
		Temp->Next=NULL;
		Rear_F->Next=Temp;
		Rear_F=Temp;
	}
}
void Tpush(node *Temp) {
	if(Front_T==NULL) {
		Front_T=Temp;
		Rear_T=Temp;
		Temp->Next=NULL;
	}
	else {
		Temp->Next=NULL;
		Rear_T->Next=Temp;
		Rear_T=Temp;
	}
}
void push() {
	Temp=(node *)malloc(sizeof(node));
	Temp->ID=i+1;
	printf("\nProcess no: %d\n1.Priority or 2.RoundRobin or 3.FCFS?",Temp->ID);
	do{
		scanf("%d",&Temp->Priority_P);
		if(Temp->Priority_P!=1 && Temp->Priority_P!=2 && Temp->Priority_P!=3) {
			printf("\nWrong input select from above.\n1.Priority or 2.RoundRobin or 3.FCFS?");
		}
	}while(Temp->Priority_P!=1 && Temp->Priority_P!=3 && Temp->Priority_P!=2);
	printf("\nProcess Arrival Time :");
	scanf("%d",&Temp->Atime);
	printf("\nProcess Burst Time :");
	scanf("%d",&Temp->Btime);
	Temp->Rtime=Temp->Btime;
	if(Temp->Priority_P==1) {
		printf("\nProcess Priority :");
		scanf("%d",&Temp->Priority);
	}
	Temp->Qtime=0;
	sem_init(&Temp->se,0,0);
	Temp->Arrival_Flag=1;
	Temp->Preemtion_Flag=0;
	Temp->Flag=1;
	Temp->Completed=0;
	node *Start=Front_A;
	if(Front_A==NULL) {
		Front_A=Temp;
		Temp->Next=NULL;
	}
	else if (Start->Atime > Temp->Atime || (Start->Atime == Temp->Atime && Start->Priority > Temp->Priority)){
        	Temp->Next = Front_A;
        	Front_A=Temp;
    }
    else {
    	while (Start->Next != NULL && Start->Next->Atime <= Temp->Atime) {
       		if(Start->Next->Atime==Temp->Atime && Temp->Priority<Start->Next->Priority) {
           		break;
			}
			else
				Start = Start->Next;	
        	}
        Temp->Next = Start->Next;
        Start->Next = Temp;
    }
}
void main() {
	int l=1;
	printf("Enter Number of Processes :");
	scanf("%d",&n);
	while(i<n) {
		push();
		i+=1;
	}
	i=0;
	printf("\n\t\tPROCESS EXECUTION STARTED.");
	Start=clock();
	Count=clock();
	while(Front_A!=NULL) {
		if(Temp->Atime<0) {
			printf("Invalid Arrival Time for Process :%d",Temp->ID);
			Front_A=Front_A->Next;
			i++;
		}
		if((clock()-Start)/CLOCKS_PER_SEC>=Front_A->Atime) {
			if(l==1) {
				sem_post(&Front_A->se);
				l=0;
			}
			Temp=Front_A;
			Front_A=Front_A->Next;
			if(Temp->Priority_P==1) {
				pthread_create(&P[i],NULL,Priority,Temp);
				Ppush(Temp);
			}
			else if(Temp->Priority_P==2) {
				pthread_create(&P[i],NULL,RRobin,Temp);
				Tpush(Temp);
			}
			else {
				pthread_create(&P[i],NULL,FCFS,Temp);
				Fpush(Temp);
			}
			i+=1;
		}
		if((clock()-Count)/CLOCKS_PER_SEC==1 && (Front_P==NULL && Front_T==NULL  && Front_F==NULL)) { 
			Count=clock();
			l=1;
			printf("\nNo Process is Running\t\t\t\t\tTimer :%d",(clock()-Start)/CLOCKS_PER_SEC);
		}
	}
	int j;
	for(j=0;j<n;j++) {
		pthread_join(P[j],NULL);
	}
	printf("\nAverage Waiting Time :%f\nAverage Turn Around Time :%f",(float)WT/n,(float)TAT/n);
}

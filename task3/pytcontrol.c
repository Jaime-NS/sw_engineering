#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char msg[5]=" ";

int tcount = 0;
int T = 25, P =1013, Tref = 25, Pref = 1013, Tsen, Psen, var_T = 3, var_P = 20;
int run = 0, cs_init = 0, disp = 1;

void *stop (void* arg);
void *start (void* arg);
void *sense_control_P (void* arg);
void *sense_control_T (void* arg);
void *log_1 (void* arg);
void *show (void* arg);
void *change_setpoint (void* arg);

void err_abort (int status, char *message)
{
  printf("Error [%d] - %s - ABORTING !\n\n", status, message);
}

//MAXIMUM priority

void *stop (void* arg){
	
	do{
	} while (msg[0] != 'e' );
	
	run = 0;
	--tcount;
	return NULL;
}

//MEDIUM priority

void *start (void* arg){
	pthread_t thread1, thread2, thread3, thread4, thread5;
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	int status;
	
	printf ("Si quieres empezar, escribe 's' y dale al enter\n");
	printf ("Si quieres terminar el proceso, escribe 'e' y dale al enter\n");
	
	run = 1;
	
	do{
	} while (msg[0] != 's' && run == 1);
	
	if (run == 1){
		printf ("Hemos empezado, para poner un setpoint escribe 'd' y dale al enter\n");
	
		status = pthread_attr_init (&thread_attr);
		if (status!= 0) err_abort (status, "Init attr (start)");
		
		status= pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
		if (status != 0) printf ("Unable to set SCHED_FIFO policy (start).\n");
		
		status = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
		if (status != 0) err_abort (status, "Set inherit scheduling (start)");
		
		//Thread for MEDIUM priority 
		
		thread_param.sched_priority = 5;
		status = pthread_attr_setschedparam(&thread_attr, &thread_param);
		if (status!= 0) err_abort (status, "Set params");
		
			//Thread for SENSE CONTROL T:
			tcount++;
			status = pthread_create(&thread1, &thread_attr, sense_control_T, NULL);
			if (status != 0) err_abort (status, "Not able to create control");
			
			//Thread for SENSE CONTROL P:
			tcount++;
			status = pthread_create(&thread2, &thread_attr, sense_control_P, NULL);
			if (status != 0) err_abort (status, "Not able to create control");
			
		//Thread for MINIMUM priority 
		
		thread_param.sched_priority = 1;
		status = pthread_attr_setschedparam(&thread_attr, &thread_param);
		if (status!= 0) err_abort (status, "Set params");
		
			//Thread for SHOW:
			tcount++;
			status = pthread_create(&thread3, &thread_attr, show, NULL);
			if (status != 0) err_abort (status, "Not able to create control");
			
			//Thread for LOG:
			tcount++;
			status = pthread_create(&thread4, &thread_attr, log_1, NULL);
			if (status != 0) err_abort (status, "Not able to create control");
			
			//Thread for CHANGE SETPOINT:
			tcount++;
			status = pthread_create(&thread5, &thread_attr, change_setpoint, NULL);
			if (status != 0) err_abort (status, "Not able to create control");
		
		pthread_join (thread1, NULL);
		pthread_join (thread2, NULL);
		pthread_join (thread3, NULL);
		pthread_join (thread4, NULL);
		pthread_join (thread5, NULL);
	}
	--tcount;
	return NULL;
}


void *sense_control_T (void* arg){
		
	while (run != 0){
		srand(time(NULL));
	
		Tsen = T - var_T + rand()%(2*var_T+1);
		
		if (abs(Tsen - Tref) > var_T){
			if (Tsen - Tref > 0) --T;
			else ++T;
		}
		sleep(0.6);
	}
	
	--tcount;
	return NULL;
}
void *sense_control_P (void* arg){
	
	while (run != 0){
		srand(time(NULL));
	
		Psen = P - var_P + rand()%(2*var_P+1);
		
		if (abs(Psen - Pref) > var_P){
			if (Psen - Pref > 0) --P;
			else ++P;
		}
		sleep(0.25);
	}
	
	--tcount;
	return NULL;
}

//MEDIUM priority

void *show (void* arg){
	
	while (run != 0){
		if (disp == 1) printf ("Temperatura: %d	Presión: %d\n", Tsen, Psen);
		sleep(1);
	}
	
	--tcount;
	return NULL;
}

void *log_1 (void* arg){
	
	FILE* fp;
	fp = fopen("Process_state.txt", "w+");
	
	while (run != 0){
		fprintf (fp, "Temperatura: %d	Presion: %d\n", Tsen, Psen);
		sleep(1);
	}
	
	fclose(fp);
	
	--tcount;
	return NULL;
}

void *change_setpoint (void* arg){
	int valor;	
	
	while (run != 0){
		
		do{
		} while (msg[0] != 'd' && run == 1);
		
		cs_init = 1;
		
		if(run == 1){
			disp = 0;
		
			printf ("Introduzca T de referencia: \n");
			scanf("%s",msg);
			sscanf(msg, "%d", &valor);
			if (valor > 0) Tref = valor;
				
			if(run == 1){
				if (valor > 0){
					printf ("Introduzca P de referencia: \n");
					scanf("%s",msg);
					sscanf(msg, "%d", &valor);
					if (valor > 0) Pref = valor;
					else printf ("P no válida, debe ser un número mayor que 0\n");
				}
				else printf ("T no válida, debe ser un número mayor que 0\n");
			}
			
			disp = 1;
		}
		
		cs_init = 0;
	}	
		
	--tcount;
	return NULL;
}

int main (){
	
	pthread_t thread1, thread2;
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	int status;
	
	status = pthread_attr_init (&thread_attr);
	if (status!= 0) err_abort (status, "Init attr");
	
	status= pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
	if (status != 0) printf ("Unable to set SCHED_FIFO policy.\n");
	
	status = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
	if (status != 0) err_abort (status, "Set inherit scheduling");
	
	
	printf("Creating threads at FIFO\n");
	
	//Thread for MAXIMUM priority (STOP)
	
	thread_param.sched_priority = 10;
	status = pthread_attr_setschedparam(&thread_attr, &thread_param);
	if (status!= 0) err_abort (status, "Set params");
	
		//Thread for STOP:
		tcount++;
		status = pthread_create(&thread1, &thread_attr, stop, NULL);
		if (status != 0) err_abort (status, "Not able to create stop");
		
	
	//Thread for MEDIUM priority (START)
	
	thread_param.sched_priority = 5;
	status = pthread_attr_setschedparam(&thread_attr, &thread_param);
	if (status!= 0) err_abort (status, "Set params");
	
		//Thread for START:
		tcount++;
		status = pthread_create(&thread2, &thread_attr, start, NULL);
		if (status != 0) err_abort (status, "Not able to create start");
	
	do{
		if (cs_init == 0){
			scanf("%s",msg);
		}
	} while (msg[0] != 'e');
	
	pthread_join (thread1, NULL);
	pthread_join (thread2, NULL);
		
	printf("Tenemos %d hilos abiertos\n", tcount);
	
}

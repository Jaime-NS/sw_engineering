#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

struct position {
	int line_number;
	int column;
	int thread_id;
	int parent_id;
	int steps;
	char dir;
};

struct position *ini, *end;
struct position path[1024];

char lab[1024][1024];
int filas, columnas;
int id = 0, a = 0, active_threads = 0, fin = 0;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *go_horizontal(void *pos_rec);
void *go_vertical(void *pos_rec);

void show_lab(){
	pthread_mutex_lock(&mutex1);
	int i;
	printf("\n");
	for (i=0; i<=filas;++i){
		printf("%s", lab[i]);
	}
	printf("\n");
	pthread_mutex_unlock(&mutex1);
}

/*----------------------------------------------------------------------------------------------------------
		FUNCION PARA MOVERSE EN LA HORIZONTAL
-----------------------------------------------------------------------------------------------------------*/
void *go_horizontal(void *pos_rec){
	struct position *pos, *pos1, *pos2, *pos3;
	pthread_t thread1, thread2, thread3;
	int i, j, x, threads_created = 0;
	
	pos1 = malloc(sizeof(struct position));
	pos2 = malloc(sizeof(struct position));
	pos3 = malloc(sizeof(struct position));

	pos = pos_rec;
	pos->parent_id = pos->thread_id;
	pos->thread_id = ++id;
	pos->steps = 0;
	
	if(pos->dir == 'r'){x = 1;}
	else if(pos->dir == 'l'){x = -1;}
	
	++active_threads;
	
	printf ("Thread %d was created by %d and starts in (%d, %d)\n", pos->thread_id, pos->parent_id, pos->line_number, pos->column);

	i = pos->line_number;
	j = pos->column;
	
	for (; j >= 0 && j < columnas; j+=x){
		pos->line_number = i;
		pos->column = j;
		
		switch (lab[i][j]){
			case 'I' :
				break;
			case '#' : 
				printf ("Thread %d ends in (%d, %d) after %d steps\n",pos->thread_id, pos->line_number, pos->column, pos->steps);
				--active_threads;
				return NULL;
				break;
			case 'O' :
				++pos->steps;
				*end = *pos;
				path[a++] = *pos;
				fin = 1;
				printf ("Thread %d ends in (%d, %d) after %d steps\n",pos->thread_id, pos->line_number, pos->column, pos->steps);
				--active_threads;
				return NULL;
			case ' ' :
				lab[i][j] = '-';
				++pos->steps;
				path[a++] = *pos;
				break;
			default :
				printf("Labyrinth format not valid\n");
				break;		
		}
		
		switch (lab[i - 1][j]){
			case '#' : 
				break;
			case 'O' :
				printf("Out shouldn't be here\n");
			case ' ' :
				pos1->line_number = i - 1;
				pos1->column = j;
				pos1->thread_id = pos->thread_id;
				pos1->dir = 'u';
				pthread_create (&thread1, NULL, go_vertical, (void*) pos1);
				++threads_created;
				break;
			default :
				printf("Labyrinth format not valid\n");
				break;		
		}
		
		switch (lab[i + 1][j]){
			case '#' : 
				break;
			case 'O' :
				printf("Out shouldn't be here\n");
			case ' ' :
				pos2->line_number = i + 1;
				pos2->column = j;
				pos2->thread_id = pos->thread_id;
				pos2->dir = 'd';
				pthread_create (&thread2, NULL, go_vertical, (void*) pos2);
				++threads_created;
				break;
			default :
				printf("Labyrinth format not valid\n");
				break;		
		}
		
		if (threads_created == 2 && lab[i][j+x] == ' '){
			pos3->line_number = i;
			pos3->column = j + x;
			pos3->thread_id = pos->thread_id;
			pos3->dir = pos->dir;
			pthread_create (&thread3, NULL, go_horizontal, (void*) pos3);
			printf ("Thread %d ends in (%d, %d) after %d steps\n",pos->thread_id, pos->line_number, pos->column, pos->steps);
			--active_threads;
			return NULL;
		}
		
	}
}

/*----------------------------------------------------------------------------------------------------------
		FUNCION PARA MOVERSE EN LA VERTICAL
-----------------------------------------------------------------------------------------------------------*/
void *go_vertical(void *pos_rec){
	struct position *pos, *pos1, *pos2, *pos3;
	pthread_t thread1, thread2, thread3;
	int i, j, x, threads_created = 0;
	
	pos1 = malloc(sizeof(struct position));
	pos2 = malloc(sizeof(struct position));
	pos3 = malloc(sizeof(struct position));

	pos = pos_rec;
	
	pos->parent_id = pos->thread_id;
	pos->thread_id = ++id;
	pos->steps = 0;
	
	if(pos->dir == 'd'){x = 1;}
	else if(pos->dir == 'u'){x = -1;}
	
	++active_threads;
	
	printf ("Thread %d was created by %d and starts in (%d, %d)\n",pos->thread_id, pos->parent_id, pos->line_number, pos->column);

	i = pos->line_number;
	j = pos->column;
	
	for (; i >= 0 && i <= filas; i+=x){
		
		pos->line_number = i;
		pos->column = j;
		
		switch (lab[i][j]){
			case '#' : 
				printf ("Thread %d ends in (%d, %d) after %d steps\n",pos->thread_id, pos->line_number, pos->column, pos->steps);
				--active_threads;
				return NULL;
				break;
			case 'O' :
				printf("Out shouldn't be here\n");
			case ' ' :
				lab[i][j] = '|';
				++pos->steps;
				path[a++] = *pos;
				break;
			default :
				printf("Labyrinth format not valid\n");
				break;		
		}
		
		switch (lab[i][j-1]){
			case '#' : 
				break;
			case 'O' :
				printf("Out shouldn't be here\n");
			case ' ' :
				pos1->line_number = i;
				pos1->column = pos->column - 1;
				pos1->thread_id = pos->thread_id;
				pos1->dir = 'l';
				pthread_create (&thread1, NULL, go_horizontal, (void*) pos1);
				++threads_created;
				break;
			default :
				printf("Labyrinth format not valid\n");
				break;		
		}
		
		switch (lab[i][j+1]){
			case '#' : 
				break;
			case 'O' :
				printf("Out shouldn't be here\n");
			case ' ' :
				pos2->line_number = i;
				pos2->column = pos->column + 1;
				pos2->thread_id = pos->thread_id;
				pos2->dir = 'r';
				pthread_create (&thread2, NULL, go_horizontal, (void*) pos2);
				++threads_created;
				break;
			default :
				printf("Labyrinth format not valid\n");
				break;		
		}
		
		if (threads_created == 2 && lab[i+x][j] == ' '){
			printf("Tamos inside\n");
			pos3->line_number = i + x;
			pos3->column = j;
			pos3->thread_id = pos->thread_id;
			pos3->dir = pos->dir;
			pthread_create (&thread3, NULL, go_vertical, (void*) pos3);
			printf ("Thread %d ends in (%d, %d) after %d steps\n",pos->thread_id, pos->line_number, pos->column, pos->steps);
			--active_threads;
			return NULL;
		}
	}
}

/*----------------------------------------------------------------------------------------------------------
			MAIN 
-----------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int i, n, p, f, c, total_steps = 0;
	FILE* fp;
	char line[1024];
	//char *name = "labyrinth.txt"; //argv[1];
	pthread_t thread1;
		
	ini = malloc(sizeof(struct position));
	end = malloc(sizeof(struct position));
	
	fp = fopen(argv[1], "r");
	if (fp != NULL){
		while(fgets(line, 1024, fp) != NULL){
			strcpy(lab[i], line);
			++i;
		}
		filas = i-1;
		columnas = sizeof(lab[i-1]);
		i=0;
	}
	else {
		printf("Error opening file\n");
		goto final;
	}
	fclose(fp);
	
	while (lab[i][0] != 'I'){ i++;}
	ini->line_number = i;
	ini->column = 0;
	ini->thread_id = 0;
	ini->dir = 'r';
	
	*end = *ini;
	
	show_lab();
	
	pthread_create (&thread1, NULL, go_horizontal, (void*) end);
	pthread_join( thread1, NULL);
	
	while (active_threads != 0 || fin != 1){}
	
	show_lab();
	
	for(i = 1023; i >= 0; --i){
		if(path[i].line_number == end->line_number && path[i].column == end->column){
			n = path[i].thread_id;
			p = path[i].parent_id;
			f = path[i].line_number;
			c = path[i].column;
			total_steps = path[i].steps;
		}
		
		if((abs(path[i].line_number - f) + abs(path[i].column - c)) == 1){
			if(path[i].thread_id == p){
				total_steps += path[i].steps;
				lab[path[i].line_number][path[i].column] = 'X';
			}
			if(path[i].thread_id == n){
				lab[path[i].line_number][path[i].column] = 'X';
			}
			n = path[i].thread_id;
			p = path[i].parent_id;
			f = path[i].line_number;
			c = path[i].column;
		}
	}
	
	show_lab();
	
	printf ("The number of active threads is %d\n", active_threads);
	printf ("Start is in: line: %d        column: %d\n", ini->line_number, ini->column);
	printf ("Exit is in: line: %d        column: %d\n", end->line_number, end->column);
	printf ("It took %d steps to get out of here\n", total_steps);
	
	final: 
	
	return 0;
}
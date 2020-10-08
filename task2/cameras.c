#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#define QUEUE_NAME  "/rs.mq.01"
#define MAX_SIZE    1024
#define MSG_STOP    "Door closed"

int flag = 0;

/*---------------------------------------------------------------------------------------------------------------------------------
	Totalizer
---------------------------------------------------------------------------------------------------------------------------------*/
void * queue_totalizer(void * args) {
	
	mqd_t mq;
	ssize_t bytes_read;
	struct mq_attr attr;
	char buffer[MAX_SIZE + 1];
	char cmp[4];
	int stop_msg = 0, normal = 0, vips = 0, total = 0;

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_SIZE;
	attr.mq_curmsgs = 0;

	mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
	assert(mq>0);

	while(1) {		
		memset(buffer, 0x00, sizeof(buffer));

		bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
		
    	if (bytes_read >= 0) { 
			printf("TOTALIZER: Received: [%s]\n", buffer);
			
			snprintf(cmp, sizeof(cmp), buffer);
			
			if (strcmp(cmp,"[VM") == 0) ++vips;
			else if (strcmp(cmp,"[NM") == 0) ++normal;			
			
			total = normal + vips;
			printf("VIPs: %d 	Normal: %d	Total: %d\n", vips, normal, total);
			
			if (!strcmp(buffer,MSG_STOP)) {
				++stop_msg;
				if (stop_msg == 2){
					mq_close(mq); 
					mq_unlink(QUEUE_NAME);
					return NULL;
				}
			}
		} 
		
		flag = 1;
		
		fflush(stdout);
		usleep(100000);
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------
	Message of the VIP camera
---------------------------------------------------------------------------------------------------------------------------------*/
void * queue_vip_cam(void * args) {
	
	mqd_t mq;
	char buffer[MAX_SIZE];
	int count = 0;

	while (flag != 1){}

	mq = mq_open(QUEUE_NAME, O_WRONLY);

	assert(mq>0);

	printf("VIP DOOR OPEN\n");

	for (count=0; count<10; count++) {
		if (count<9) snprintf(buffer, sizeof(buffer), "[VM %d] VIP person entered the concert", count);
		else snprintf(buffer, sizeof(buffer), MSG_STOP);

		mq_send(mq, buffer, MAX_SIZE, 1);

		fflush(stdout);
		usleep(999999);
	}

	mq_close(mq);
	return NULL;
}

/*---------------------------------------------------------------------------------------------------------------------------------
	Message of the normal camera
---------------------------------------------------------------------------------------------------------------------------------*/
void * queue_normal_cam(void * args) {
	
	mqd_t mq;
	char buffer[MAX_SIZE];
	int count = 0;

	while (flag != 1){}

	mq = mq_open(QUEUE_NAME, O_WRONLY);

	assert(mq>0);

	printf("NORMAL DOOR OPEN\n");

	for (count=0; count<40; count++) {
		if (count<39) snprintf(buffer, sizeof(buffer), "[NM %d] Normal person entered the concert", count);
		else snprintf(buffer, sizeof(buffer), MSG_STOP);

		mq_send(mq, buffer, MAX_SIZE, 0);

		fflush(stdout);
		usleep(50000);
	}

	mq_close(mq);
	return NULL;
}

/*---------------------------------------------------------------------------------------------------------------------------------
	MAIN
---------------------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char** argv) {

	pthread_t normal_cam, vip_cam, totalizer;

	printf("MAIN: Start...\n");

	pthread_create(&totalizer, NULL, &queue_totalizer, NULL);
	pthread_create(&normal_cam, NULL, &queue_normal_cam, NULL);
	pthread_create(&vip_cam, NULL, &queue_vip_cam, NULL);

	printf("MAIN: Joining ... \n");
	pthread_join(normal_cam, NULL);
	pthread_join(vip_cam, NULL);
	pthread_join(totalizer, NULL);

	printf("MAIN: Done...\n");

	return (EXIT_SUCCESS);
}


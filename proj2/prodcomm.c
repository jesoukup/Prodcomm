/* Joseph Soukup jsoukup2
	Steven Mulvey smulvey2 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <ctype.h>

int buffer = 1024;
sem_t s;

typedef struct Queue {
    int length;
    int capacity;
    int front;
    int rear;
    char **elements;
	int enqueueCount;
	int dequeueCount;
	int enqueueBlockCount;
	int dequeueBlockCount;
} Queue;

Queue* CreateStringQueue(int size) {
	Queue *Q;
    Q = (Queue *)malloc(sizeof(Queue)); 
    Q->elements = (char**)malloc(sizeof(char*)*size); 
    Q->length = 0;
    Q->capacity = size;
    Q->front = 0;
    Q->rear = -1;
	Q->enqueueCount = 0;
	Q->dequeueCount = 0;
	Q->enqueueBlockCount = 0;
	Q->dequeueBlockCount = 0;
    return Q;
}

void EnqueueString(Queue *Q, char *string) {
	sem_wait(&s);
	if (Q->length == Q->capacity) {
		printf("Queue is Full\n");
    } else {
		Q->length++;
        Q->rear = Q->rear + 1;
        if(Q->rear == Q->capacity) {
                Q->rear = 0;
        }
		Q->elements[Q->rear] = (char *) malloc((sizeof string + 1)* sizeof(char));
		strcpy(Q->elements[Q->rear], string);
		Q->enqueueCount++;
    }
	sem_post(&s);
    return;
}

char* DequeueString(Queue *Q) {
	sem_wait(&s);
	if (Q->length == Q->capacity) {
		printf("Queue is Full\n");
	} else {
        Q->length--;
        Q->front++;
        if(Q->front == Q->capacity) {
			Q->front =0;
        }
		Q->dequeueCount++;
    }
	sem_post(&s);
    return Q->elements[Q->front-1];
}

void PrintQueueStats(Queue *Q) {
	printf("enqueueCount = ");
	printf("%d\n", Q->enqueueCount);
	printf("dequeueCount = ");
	printf("%d\n", Q->dequeueCount);
	printf("enqueueBlockCount = ");
	printf("%d\n", Q->enqueueBlockCount);
	printf("dequeueBlockCount = ");
	printf("%d\n", Q->dequeueBlockCount);
}

typedef struct __myarg_t {
	struct Queue* Q;
	struct Queue* R;
	struct Queue* S;
} myarg_t;

void* reader(void *arg) {
	myarg_t *m = (myarg_t *) arg;
	size_t buff = buffer;
	char *string = malloc(buffer * sizeof(char));
	if (getline(&string, &buff, stdin) == -1) {
		fprintf(stderr, "exceeds buffer");
	}
	EnqueueString(m->Q, string);
	return (void *) string;
}

void* munch1(void *arg) {
	myarg_t *m = (myarg_t *) arg;
	char *munch1 = malloc(buffer * sizeof(char));
	strcpy(munch1, DequeueString(m->Q));
	char ch = ' ';
	for (int i = 0; (unsigned) i < strlen(munch1); i++) {
		if (munch1[i] == ch) {
			munch1[i] = '*';
		} 
	}
	EnqueueString(m->R, munch1);
	return (void *) munch1;
}

void* munch2(void *arg) {
	myarg_t *m = (myarg_t *) arg;
	char *munch2 = malloc(buffer * sizeof(char));
	strcpy(munch2, DequeueString(m->R)); 
	for (int i = 0; (unsigned) i < strlen(munch2); i++) {
		munch2[i] = toupper(munch2[i]);
	}
	EnqueueString(m->S, munch2);
	return (void *) munch2;
}

void* writer(void *arg) {
	myarg_t *m = (myarg_t *) arg;
	char *writer = malloc(buffer * sizeof(char));
	strcpy(writer, DequeueString(m->S));
	printf("%s\n", writer);
	return (void *) writer;
}

int main() {
	sem_init(&s, 0, 1);
	struct Queue* Q = CreateStringQueue(10);
	struct Queue* R = CreateStringQueue(10);
	struct Queue* S = CreateStringQueue(10);
	pthread_t p;
	myarg_t args;
	args.Q = Q;
	args.R = R;
	args.S = S;
	if (pthread_create(&p, NULL, reader, &args) != 0) {
		perror("pthread_create() error");
		exit(1);
	}
	if (pthread_join(p, (void **) NULL) != 0) {
		perror("pthread_create() error");
		exit(3);
	}
	if (pthread_create(&p, NULL, munch1, &args) != 0) {
		perror("pthread_create() error");
		exit(1);
	}
	if (pthread_join(p, (void **) NULL) != 0) {
		perror("pthread_create() error");
		exit(3);
	}
	if (pthread_create(&p, NULL, munch2, &args) != 0) {
		perror("pthread_create() error");
		exit(1);
	}
	if (pthread_join(p, (void **) NULL) != 0) {
		perror("pthread_create() error");
		exit(3);
	}
	if (pthread_create(&p, NULL, writer, &args) != 0) {
		perror("pthread_create() error");
		exit(1);
	}
	if (pthread_join(p, (void **) NULL) != 0) {
		perror("pthread_create() error");
		exit(3);
	}
	PrintQueueStats(Q);
	PrintQueueStats(R);
	PrintQueueStats(S);
	return 0;
}
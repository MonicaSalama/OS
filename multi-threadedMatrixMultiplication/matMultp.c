/*
 ============================================================================
 Name        : matMultp.c
 Author      : Monica
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

int rowMultiply();    //for each row in MAT1 creates a thread
//that calculates the first row elements in MAT3
void* row_calculate(void* tid); //Function used by thread to calculate the elem. of MAT3[tid]
int elementMult();             //for each element in MAT3 creates a thread
void* element_calculate(void* tid);  //calculates the value of MAT3[ROW][COL]
char* copyString(char* d, char* append); //Set the file names from input
int ReadFile(char* f1, int first); //opens file
int scanFirst(FILE* fp); //save first matrix from file to MAT1
int scanSecond(FILE* fp); //save second matrix from file to MAT2
int print(char* name); // print the output to its corresponding file
int check(); // check for the correctness of dimensions for Matrix multplication

long long **mat1;
long long **mat2;
long long **mat3;
int r1, c1, r2, c2;

struct element {
	int r;
	int c;
};

char* copyString(char* d, char* append) {
	char*s = malloc(strlen(d));
	int i = 0;
	for (i = 0; d[i] != '\0'; i++) {
		s[i] = d[i];
	}
	if (append != NULL) {
		int j;
		for (j = 0; append[j] != '\0'; j++) {
			s[i] = append[j];
			i++;
		}
	}
	s[i] = '\0';
	return s;
}
void* row_calculate(void* tid) {
	int row = (intptr_t) tid;
	int i, j;
	//calculates matrix element
	for (i = 0; i < c2; i++) {
		mat3[row][i] = 0;
		for (j = 0; j < c1; j++) {
			mat3[row][i] += (mat1[row][j] * mat2[j][i]);
		}
	}
	pthread_exit(NULL);
}
int rowMultiply() {
	pthread_t threads[r1];
	int rc;
	int i;
	//creates thread for each row
	for (i = 0; i < r1; i++) {
		rc = pthread_create(&threads[i], NULL, row_calculate, (void *) (intptr_t) i);
		//if error occured print error msg
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d : %s\n", rc ,strerror(rc));
			return 1;
		}
	}
	for (i = 0; i < r1; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}
void* element_calculate(void* tid) {
	//element contains row and col values
	struct element *el = ((struct element *) tid);
	int j;
	mat3[(*el).r][(*el).c] = 0;
	for (j = 0; j < c1; j++) {
		mat3[(*el).r][(*el).c] += (mat1[(*el).r][j] * mat2[j][(*el).c]);
	}
	pthread_exit(NULL);
}
int elementMult() {
	pthread_t threads[r1 * c2];
	int rc;
	int i, j;
	//create thread for each element
	for (i = 0; i < r1; i++) {
		for (j = 0; j < c2; j++) {
			struct element *el;
			el = malloc(sizeof(struct element));
			(*el).r = i;
			(*el).c = j;
			rc = pthread_create(&threads[i * c2 + j], NULL, element_calculate,
					(void *) el);
			
			//rc > 0 error occured print error msg and return 
			if (rc) {
				printf("ERROR; return code from pthread_create() is %d : %s\n",
						rc, strerror(rc));
				return 1;
			}
		}
	}
	//wait for all threads to finish
	for (i = 0; i < r1; i++) {
		for (j = 0; j < c2; j++) {
			pthread_join(threads[i * c2 + j], NULL);
		}
	}
	return 0;

}

int scanFirst(FILE* fp) {
	//reads first array
	fscanf(fp, "row=%d col=%d", &r1, &c1);
	mat1 = malloc(r1 * sizeof(long long*));
	if(mat1 == NULL) {
		return 0;
	}
	int i, j = 0;
	for (i = 0; i < r1; i++) {
		mat1[i] = malloc(c1 * sizeof(long long));
		if(mat1[i] == NULL) {
			return 0;
		}
		for (j = 0; j < c1; j++) {
			fscanf(fp, "%lld", &mat1[i][j]);

		}
	}
	return 1;
}
int scanSecond(FILE* fp) {
	//reads second array
	fscanf(fp, "row=%d col=%d", &r2, &c2);
	mat2 = malloc(r2 * sizeof(long long*));
	if(mat2 == NULL) {
		return 0;
	}
	int i, j = 0;
	for (i = 0; i < r2; i++) {
		mat2[i] = malloc(c2 * sizeof(long long));
		if(mat2[i] == NULL) {
			return 0;
		}
		for (j = 0; j < c2; j++) {
			fscanf(fp, "%lld", &mat2[i][j]);
		}
	}
	return 1;
}

int alloc_Mat3() {
	//size of array mat3 = r1*c2
	mat3 = malloc(r1 * sizeof(long long*));
	if(mat3 == NULL) {
		return 0;
	}
	int i;
	for(i = 0; i < r1; i++) {
		mat3[i] = malloc(c2 * sizeof(long long));
     	if(mat3[i] == NULL) {
			return 0;
		}
	}
	return 1;
}

int ReadFile(char* f1, int first) {
	FILE*fp;
	fp = fopen(f1, "r");
	if (fp == NULL) {
		perror(f1);
		return 0;

	}
	int num;
	if (first) {
	    num = scanFirst(fp);
	    if(!num)
			perror("Memory Limit");
		return num;
	} else {
		num = scanSecond(fp);
	    if(!num)
			perror("Memory Limit");
		return num;
	}

	return 1;
}

int print(char* name) {
	FILE* fp;
	fp = fopen(name, "w");
	if (fp == NULL) {
		printf("ERROR openning %s File\n", name);
		return 1;
	}
	int i, j;
	for (i = 0; i < r1; i++) {
		for (j = 0; j < c2; j++) {
			fprintf(fp, "%lld\t", mat3[i][j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n\n");
	fclose(fp);
	return 0;
}

int check() {
	if (c1 != r2)
		return 1;
	if (r1 == 0 || c1 == 0 || r2 == 0 || c2 == 0)
		return 1;
	return 0;
}

int main(int argc, char *argv[]) {
	char* f1 = "a";
	char* f2 = "b";
	char* f3 = "c_1";
	char* f4 = "c_2";
	if (argc > 1) {
		f1 = copyString(argv[1], NULL);
	}
	if (argc > 2) {
		f2 = copyString(argv[2], NULL);
	}
	if (argc > 3) {
		f3 = copyString(argv[3], "_1");
		f4 = copyString(argv[3], "_2");
	}
	struct timeval stop, start;
	if (ReadFile(f1, 1)) {
		if (ReadFile(f2, 0)) {
			if (check()) {
				printf("Incorrect Matrix dimensions\n");
				return 0;
			}
			if(!alloc_Mat3()){puts("Memory Limit"); return 0;}
			gettimeofday(&start, NULL);
			if (rowMultiply())
				return 0;
			gettimeofday(&stop, NULL);
			printf("Number of Thread = %d\n", r1);
			printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
			printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
			puts("");
			if (print(f3))
				return 0;
			printf("Number of Thread = %d\n", (r1 * c2));
			gettimeofday(&start, NULL);
			if (elementMult())
				return 0;
			gettimeofday(&stop, NULL);
			printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
			printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
			if (print(f4))
				return 0;
		}
	}
	pthread_exit(NULL);
	return EXIT_SUCCESS;
}



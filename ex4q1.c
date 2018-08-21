#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX 510
int partition_arr[3]; // global arr to update low,high,level for the partition calls .
int *arr; // global arr for the numbers from the file
int division =0;

typedef struct { // a struct that will save the data that will be in the file given in argv.
	int num;
	int depth;
} File_data;

void* call_partition(void*);
File_data get_file_data(char *,File_data);
void printArray(int*,int);                                        // function decleration.
void error(char *);
void mergeSort(int arr[], int low, int mid, int high);
void partition();
void partition_update(int,int,int);
int main(int argc, char *argv[])
{
	if(argc != 2)
		error("Not enough arguments");
	File_data data;
	FILE *f;
	data = get_file_data(argv[1],data); // a private function to retrive file data 

	printf("Amount of numbers that sort: %d\nDegree of parallelism: %d\nBefore sort: ", data.num,data.depth);
	printArray(arr,data.num);
	division = log2(data.depth); // the number of threads  
	partition_update(0,data.num-1,0); // updating the global arr to the length of the arr for the first partition call
	partition(); // 
	printf("After sort: ");
	printArray(arr,data.num);

	free(arr);

	return 0;
}
void error(char *msg) // private function to print a given msg and exits
{
	perror(msg);
	printf("\n");
	exit(0);
}
File_data get_file_data(char* file_name,File_data data) /* private function to put all the file data into the struct i created  */
{
	FILE* f = fopen(file_name,"r");         // opening the file and 
	fscanf(f,"%d",&(data.num));             // taking each line to the correct variable 
	fscanf(f,"%d",&(data.depth));
	arr = (int*)malloc(data.num*sizeof(int));
	assert(arr!=NULL);
	char str[MAX];
	fscanf(f,"%s",str); // taking the last line as a string and conveting it to a number using ftok
	int i=0;
	char *temp  = strtok(str,",");
	while(temp!=NULL)
	{
		arr[i++] = atoi(temp); // updating the global arr
		temp =strtok(NULL,",");
	}
	fclose(f);
	return data;
}
void partition()
{	
	int low,high,mid,level,status;
	low = partition_arr[0];
	high = partition_arr[1];        // updating low,high,level from the global arr.
	level = partition_arr[2];												
	mid = 0;										
	pthread_t left,right,exp;
	if (low < high)
	{
		mid = (low + high) / 2;
		if(division==0)
		{
			division = -1; // division -1 for the next call
			partition_update(low,high,level +1 );
			status = pthread_create(&exp,NULL,call_partition,NULL); 
			pthread_join(exp,NULL); // waiting for the exp thread to finish 
			if(status != 0)
				error("pthread failed");
		}
		else if(level == division-1) // if we are in the correct depth we can start using threads to divide the tasks.
		{
				partition_update(low,mid,level+1);					// updating the global arr for the left part of a given arr. 
				status = pthread_create(&left,NULL,call_partition,NULL); // calling the left thread 
				sleep(0.5);
				if(status != 0)
					error("pthread failed");
			
				partition_update(mid+1,high,level+1); // updating the global arr for the right part of a given arr.
				status = pthread_create(&right,NULL,call_partition,NULL); // calling the right thread.
				sleep(0.5);
				if(status != 0)
					error("pthread failed");

				pthread_join(left,NULL); // waiting for both of the threads to finish and than calling mergeSort
				pthread_join(right,NULL);
				
				mergeSort(arr,low,mid,high);
		}
		else // if we are not in the correct depth using a normal merge sort calls 
		{
			partition_update(low,mid,level+1); // if were not in the right depth or we finished than updating before each call for the partition and than Mergin
			partition();
			partition_update(mid+1,high,level+1);
			partition();
			mergeSort(arr, low, mid, high);
			
		}
	}
	return ;
}

void mergeSort(int arr[], int low, int mid, int high) // a normal mergeSort function nothing more to explain.
{
	int i = 0;
	int m = 0;
	int k = 0;
	int l = 0;
	int temp[MAX];

	l = low;
	i = low;
	m = mid + 1;

	while ((l <= mid) && (m <= high))
	{
		if (arr[l] <= arr[m])
		{
			temp[i] = arr[l];
			l++;
		}
		else
		{
			temp[i] = arr[m];
			m++;
		}
		i++;
	}

	if (l > mid)
	{
		for (k = m; k <= high; k++)
		{
			temp[i] = arr[k];
			i++;
		}
	}
	else
	{
		for (k = l; k <= mid; k++)
		{
			temp[i] = arr[k];
			i++;
		}
	}

	for (k = low; k <= high; k++)
	{
		arr[k] = temp[k];
	}
}

void printArray(int *arr,int size) // a simple function to print an array of integers
{
	for (int i = 0; i < size; ++i)
	{
		printf("%d",arr[i]);
		if(i != size-1)
			printf(", ");
	}
	printf("\n");
}
void partition_update(int low,int high,int level) // a private function to update the arr for the partition function.
{
	partition_arr[0] = low;
	partition_arr[1] = high;
	partition_arr[2] = level;

}

void* call_partition(void* none) // this function is a transfer station before calling the partition after creating a thread
{ // meant to print the thread id and than calling partition.
	printf("Create a thread: %u\n",(unsigned)pthread_self());
	partition();

}
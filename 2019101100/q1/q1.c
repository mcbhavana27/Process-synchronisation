#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <time.h>
#include <wait.h>
#include <limits.h>
#include <pthread.h>
#include <inttypes.h>
#include <fcntl.h>
#include <math.h>

#define KNRM  "\x1B[1;0m"
#define KRED  "\x1B[1;31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[1;33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[1;36m"
#define KWHT  "\x1B[1;37m"
#define KGR "\x1B[1;32m"

typedef struct node node;
int temp[10004];
pthread_mutex_t mutex;
int shmid, *arr;
struct node
{
    int l,r;
};

void merge(int *arr,int start,int mid,int end)
{
    int left[10004],right[10004],ll=mid-start+1,rl=end-mid;
    for(int i=0;i<ll;i++)
        left[i]=arr[start+i];
    for(int i=0;i<rl;i++)
        right[i]=arr[mid+1+i];
    int a=0,b=0,c=0;
    while(a<ll && b<rl)
    {
        if(left[a]<right[b])
        {
            arr[c+start]=left[a++];
            c++;
        }
        else
        {
            arr[c+start]=right[b++];
            c++;
        }
    }
    while(a<ll)
    {
        arr[c+start]=left[a++];
        c++;
    }
    while(b<rl)
    {
        arr[c+start]=right[b++];
        c++;
    }
}

void selection_sort(int l,int r)
{
    for(int i=l;i<=r;i++)
        {
            int min=i;
            for(int j=i+1;j<=r;j++)
            {
                if(arr[j]<arr[min])
                    min=j;
            }
            int temp=arr[min];
            arr[min]=arr[i];
            arr[i]=temp;
        }

}

void proc_mergesort(int *arr,int start,int end)
{
	int len=end-start+1;
	if(len<=5)
	{
        selection_sort(start,end);
        return NULL;
	}
	else
	{
		pid_t left,right;
		left=fork();
		if(left==0)
		{
			proc_mergesort(arr,start,start+len/2-1);
			_exit(0);
		}
		else
		{
			right=fork();
			if(right==0)
			{
				proc_mergesort(arr,start+len/2,end);
				_exit(0);
			}
		}
		int status;
		waitpid(left,&status,0);
		waitpid(right,&status,0);
		merge(arr,start,start+len/2-1,end);
    }

}

void * thread_mergesort(void * temp)
{
    struct timespec ts;
    struct node * elem = (node *) temp;
    int l=elem->l;
    int r=elem->r;
    if(l>r)
    {
        return NULL;
    }
    if((r-l)<=5)
    {
        selection_sort(l,r);
        return NULL;
    }
    struct node a1,a2;
    int m=(l+r)/2; 
    pthread_t tid1,tid2;
    a1.l=l;
    a1.r=m;
    pthread_create(&tid1,NULL,thread_mergesort,&a1);
    a2.l=m+1;
    a2.r=r;
    pthread_create(&tid2,NULL,thread_mergesort,&a2);
    // joining threads after being sorted
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    pthread_mutex_lock(&mutex);
    merge(arr,l,m,r);
    pthread_mutex_unlock(&mutex);
}

void mergesort(int *brr,int start,int end)
{
	if(start>=end)
		return;
	int mid=start+(end-start)/2;
	mergesort(brr,start,mid);
	mergesort(brr,mid+1,end);
	merge(brr,start,mid,end);
}

int main()
{
	key_t key=IPC_PRIVATE;
    int n;
    printf("Enter number of elements:\n");
    scanf("%d",&n);
	shmid=shmget(key, sizeof(int)*n, IPC_CREAT|0666);
	arr=shmat(shmid,NULL,0);
    
    struct timespec ts;
    long double t1,t2,st,en,t3;
    printf("Enter the elements in the array :\n");
    for(int i=0;i<n;i++)
    {
        scanf("%d",&arr[i]); 
    }
    int *brr = (int*) malloc((n+5) * sizeof(int));
    int *crr = (int*) malloc((n+5) * sizeof(int));
    
    for(int i=0; i < n; i++)
    {
        brr[i] =arr[i];
        crr[i]=arr[i];
    }
    struct node temp;
    temp.l=0;
    temp.r=n-1;
    pthread_t root;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    pthread_create(&root,NULL,thread_mergesort,&temp);
    pthread_join(root,NULL);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    t1=en-st;
    printf("%s sorted array using threads\n",KRED);
    for(int i=0;i<n;i++)
    {
        printf("%d ",arr[i]);
    }
     printf("\n");
    printf("%s time taken by threaded_mergesort: %Lf\n",KYEL,t1); 

    printf("\n");
    //printf("time taken by threaded merge_sort: %Lf\n",t1); 

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    proc_mergesort(arr,0,n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    t2=en-st;
    printf("%s sorted array using process\n",KRED);
    for(int i=0;i<n;i++)
    {
        printf("%d ",arr[i]);
    }
     printf("\n");
    printf("%s time taken by concurrent_process_mergesort: %Lf\n",KYEL,t2); 

    printf("\n");
  //  printf("time taken by concurrent merge_sort: %Lf\n",t2); 

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    //printf("%Lf\n",en);
    mergesort(brr,0,n-1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
     //printf("%Lf\n",en);
    t3=en-st;
    //printf("time taken by normal_mergesort: %Lf\n",t3); 

    printf("%s sorted array using normal quicksort\n",KRED);
    for(int i=0;i<n;i++)
    {
        printf("%d ",brr[i]);
    }
     printf("\n");
    printf("%s time taken by normal_mergesort: %Lf\n",KYEL,t3); 

    printf("\n");
    return 0;
}
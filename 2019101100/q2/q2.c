#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define KNRM  "\x1B[1;0m"
#define KRED  "\x1B[1;31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[1;33m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[1;36m"
#define KGR "\x1B[1;32m"

typedef struct company company;
typedef struct zone zone;
typedef struct student student;

struct company
{
    int idx;
    int status;
    int vaccine;
    int capacity;
    pthread_mutex_t company_mutex;
    pthread_t company_thread_id;
    pthread_cond_t c_zone;
};

struct zone
{
    int idx;
    int slots;
    int occupancy;
    int capacity;
    pthread_mutex_t zone_mutex;
    pthread_t zone_thread_id;
    pthread_cond_t c_student;   
    
};

struct student
{
    int idx;
    pthread_t student_thread_id;
    
};

company companys[1000];
zone zones[1000];
student students[1000];

void *company_thread(void *inp);
void *zone_thread(void *inp);
void *student_thread(void *inp);
void vaccine_ready(company *company);
void ready_to_zone(zone *zone);
void wait_for_slot(int idx);
void student_in_slot(int i,int idx);

int no_companies;
int no_students;
int no_zones;

void *company_thread(void *inp)
{
    company *Company=(company*) inp;
    while(1)
    {
        int r = rand()%10 +1;
        printf("%s company %d preparing %d vaccines\n",KRED,Company->idx,r);
        int w=rand()%4+2;
        sleep(w);
        pthread_mutex_lock(&(Company -> company_mutex));
        Company->vaccine=r;
        Company->capacity=rand()%6+20;
        printf("%s Vaccination Zone %d is ready to vaccinate with %d\n slots where %d is the number of slots available in that zone for that particular round of vaccination\n",KMAG,Company->idx,Company->vaccine,Company->capacity);
        vaccine_ready(Company);
    }
    return NULL;
}

void vaccine_ready(company *company)
{
    while (1)
    {
        if(company->vaccine == 0)
        break;
        else
        {
            pthread_cond_wait(&(company->c_zone), &(company->company_mutex));
        }
        
    }
    printf("%s Vaccination Zone %d, has run out of vaccines\n",KNRM,company->idx);
    pthread_mutex_lock(&(company->company_mutex)); 

}

void * zone_thread(void *inp)
{
    zone *Zone=(zone*)inp;
    while(1)
    {
        int flag=0;
        for(int i=0;i<no_companies;i++)
        {
            pthread_mutex_lock(&(companys[i].company_mutex));
            if(companys[i].vaccine>0)
            {
                flag=1;
                Zone->capacity=companys->capacity;
                companys[i].vaccine--;
                printf("%s zone %d has received vaccine from company %d\n",KCYN,Zone->idx,i+1);
                pthread_cond_signal(&(companys[i].c_zone));
                pthread_mutex_unlock(&(companys[i].company_mutex));
                break;
            }
            pthread_cond_signal(&(companys[i].c_zone));
            pthread_mutex_unlock(&(companys[i].company_mutex));
            
        }
        while (flag)
        {
            pthread_mutex_lock(&(Zone-> zone_mutex));
            if(Zone->capacity == 0)
            {
                printf("%s Pharmaceutical Company %d has delivered vaccines to Vaccination zone %d, resuming vaccinations now/n",KRED,Zone->idx,Zone->idx);
                pthread_mutex_unlock(&(Zone-> zone_mutex));
                break;
            }
            Zone-> slots =rand()%10 +1;
            Zone->occupancy=0;
            if(Zone->slots > Zone->capacity)
                Zone->slots = Zone-> capacity;
            Zone-> capacity -= Zone->slots;
            printf("%s zone %d ready to vaccinate with %d slots\n",KMAG,Zone->idx,Zone->slots);
            ready_to_zone(Zone);
        }
        
    }
    return NULL;

}

void ready_to_zone(zone *zone)
{
    while (1)
    {
        if(zone->slots == zone->occupancy)
        {
            printf("%s When the Vaccines of a Vaccination Zone is refilled by a Pharmaceutical Company %d\n",KCYN,zone->idx);
        }
        else
        {
            pthread_cond_wait(&(zone->c_student),&(zone->zone_mutex));
        }
        pthread_mutex_unlock(&(zone->zone_mutex));
        
    }
    
}

void *student_thread(void *inp)
{
    student *Student = (student *) inp;
    int ti =rand()%10;
    sleep(ti);
    printf("%s Student %d is waiting to be allocated a slot on a Vaccination Zone\n",KYEL,Student->idx);
    wait_for_slot(Student->idx);
    return NULL;
}

void wait_for_slot(int idx)
{
    int got_zone=0;
    while(!got_zone)
    {
        for(int i=0;i<no_zones;i++)
        {
            pthread_mutex_lock(&(zones[i].zone_mutex));
            if(zones[i].slots - zones[i].occupancy > 0)
            {
                zones[i].occupancy++;
                got_zone=1;
                student_in_slot(i,idx);
                break;
            }

            pthread_cond_signal(&(zones[i].c_student));
            pthread_mutex_unlock(&(zones[i].zone_mutex));
        }
    }
    return;
}


void student_in_slot(int i,int idx)
{
    pthread_mutex_lock(&(zones[i].zone_mutex));
    printf("%s student %d waiting for slot at zone %d\n",KRED,idx,i+1);
    printf("%s Student %d on Vaccination Zone Y has been vaccinated which has success probability Xj Where Xj is the probability ​ of that vaccine successfully administrating antibodies. %d\n",KGRN,idx,i+1);
    //check the condition 3 times
    //while(1)
    pthread_cond_signal(&(zones[i].c_student));
    check_virus(idx);
    pthread_mutex_unlock(&(zones[i].zone_mutex));
    return;
}

void check_virus(int in)
{
    int r=rand()%2;
    //if(r==0)
    //printf()

}

int main()
{
    srand(time(0));
    printf("Enter no of companies,zones,and students:\n");
    scanf("%d %d %d",&no_companies,&no_zones,&no_students);

    for(int i=0;i<no_companies;i++)
    {
        companys[i].idx=i+1;
        pthread_mutex_init(&(companys[i].company_mutex),NULL);
    }

    for(int i=0;i<no_zones;i++)
    {
        zones[i].idx=i+1;
        pthread_mutex_init(&(companys[i].company_mutex),NULL);
    }

    for(int i=0;i<no_students;i++)
    {
        students[i].idx=i+1;
        pthread_mutex_init(&(companys[i].company_mutex),NULL);
    }
    printf("%s Beginning Simulation\n\n",KGR);
    for(int i=0;i<no_companies;i++)
    {
        pthread_create(&(companys[i].company_thread_id),NULL,company_thread,&companys[i]);
    }
    for(int i=0;i<no_zones;i++)
    {
        pthread_create(&(zones[i].zone_thread_id),NULL,company_thread,&zones[i]);
    }
    for(int i=0;i<no_students;i++)
    {
        pthread_create(&(students[i].student_thread_id),NULL,student_thread,&students[i]);
    }
    for(int i=0;i,no_students;i++)
    pthread_join(students[i].student_thread_id,0);

    printf("\n %s simulation over!\n",KGR);
    for(int i=0;i<no_companies;i++)
    pthread_mutex_destroy(&(companys[i].company_mutex));
    for(int i=0;i<no_zones;i++)
    pthread_mutex_destroy(&(zones[i].zone_mutex));

    return 0;

}





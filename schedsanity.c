#include "types.h"
#include "stat.h"
#include "user.h"

#define MED__CALC_ITERATIONS 1000
#define LARGE_CALC_ITERATIONS 1000000
#define MED__IO_ITERATIONS 100
#define LARGE_IO_ITERATIONS 1000

int
medCalc()
{
    int i;
    int x = 0;
    for (i=0; i<MED__CALC_ITERATIONS ; i++){
        x += i;
    }
    return x;
}

int
largeCalc()
{
    int i;
    int x = 0;
    for (i=0; i<LARGE_CALC_ITERATIONS ; i++){
        x += i;
    }
    return x;
}

void
medIo()
{
    int i;
    for (i=0; i<MED__IO_ITERATIONS ; i++){
        printf(1,"med size: %d\n",i);
    }
}

void
largeIo()
{
    int i;
    for (i=0; i<LARGE_IO_ITERATIONS ; i++){
        printf(1,"large size: %d\n",i);
    }
}

int
main(int argc, char *argv[])
{
    int x=0;
    int pid1,pid2,pid3,pid4;
    int wtime1,wtime2,wtime3,wtime4;
    int rtime1,rtime2,rtime3,rtime4;
    int iotime1,iotime2,iotime3,iotime4;
    
    
    pid1=fork();
    if (pid1==0){
        x=medCalc();
        printf(1,"med calc finished: %d\n",x);
        exit();
    }
    
    pid2=fork();
    if (pid2==0){
        x=largeCalc();
        printf(1,"large calc finished: %d\n",x);
        exit();
    }
    
    pid3=fork();
    if (pid3==0){
        medIo();
        exit();
    }
    
    pid4=fork();
    if (pid4==0){
        largeIo();
        exit();
    }
    
    wait2(pid1,&wtime1,&rtime1,&iotime1);
    wait2(pid2,&wtime2,&rtime2,&iotime2);
    wait2(pid3,&wtime3,&rtime3,&iotime3);
    wait2(pid4,&wtime4,&rtime4,&iotime4);
    
    printf(1,"med calc times: wtime: %d, rtime: %d, iotime: %d\n",wtime1,rtime1,iotime1);
    printf(1,"large calc times: wtime: %d, rtime: %d, iotime: %d\n",wtime2,rtime2,iotime2);
    printf(1,"med io times: wtime: %d, rtime: %d, iotime: %d\n",wtime3,rtime3,iotime3);
    printf(1,"large io times: wtime: %d, rtime: %d, iotime: %d\n",wtime4,rtime4,iotime4);
    
    exit();
}

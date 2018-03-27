#include "types.h"
#include "stat.h"
#include "user.h"

#define MED__CALC_ITERATIONS 1000
#define LARGE_CALC_ITERATIONS 1000000
#define MED__IO_ITERATIONS 100
#define LARGE_IO_ITERATIONS 1000
#define NUM_OF_ITERATIONS 10

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
    int pid[4* NUM_OF_ITERATIONS];
    int wtime1,wtime2,wtime3,wtime4;
    int rtime1,rtime2,rtime3,rtime4;
    int iotime1,iotime2,iotime3,iotime4;

    int avgwtime1=0,avgwtime2=0,avgwtime3=0,avgwtime4=0;
    int avgrtime1=0,avgrtime2=0,avgrtime3=0,avgrtime4=0;
    int avgiotime1=0,avgiotime2=0,avgiotime3=0,avgiotime4=0;

    
    int i;
    for(i = 0; i < NUM_OF_ITERATIONS ; i++){
        set_priority(3);
        pid[i*4]=fork();
        if (pid[i*4]==0){
            x=medCalc();
            printf(1,"med calc finished: %d\n",x);
            exit();
        }
        
        pid[i*4 + 1]=fork();
        if (pid[i*4 + 1]==0){
            x=largeCalc();
            printf(1,"large calc finished: %d\n",x);
            exit();
        }
        
        pid[i*4 + 2]=fork();
        if (pid[i*4 + 2]==0){
            medIo();
            exit();
        }
        
        set_priority(1);
        pid[i*4 + 3]=fork();
        if (pid[i*4 + 3]==0){
            largeIo();
            exit();
        }
    }

    for(i = 0; i < NUM_OF_ITERATIONS ; i++){
        wait2(pid[i*4],&wtime1,&rtime1,&iotime1);
        avgwtime1 += wtime1;
        avgrtime1 += rtime1;
        avgiotime1 += iotime1;

        wait2(pid[i*4 + 1],&wtime2,&rtime2,&iotime2);
        avgwtime2 += wtime2;
        avgrtime2 += rtime2;
        avgiotime2 += iotime2;

        wait2(pid[i*4 + 2],&wtime3,&rtime3,&iotime3);
        avgwtime3 += wtime3;
        avgrtime3 += rtime3;
        avgiotime3 += iotime3;

        wait2(pid[i*4 + 3],&wtime4,&rtime4,&iotime4);
        avgwtime4 += wtime4;
        avgrtime4 += rtime4;
        avgiotime4 += iotime4;
    }

    avgwtime1 /= NUM_OF_ITERATIONS;
    avgwtime2 /= NUM_OF_ITERATIONS;
    avgwtime3 /= NUM_OF_ITERATIONS;
    avgwtime4 /= NUM_OF_ITERATIONS;
    avgrtime1 /= NUM_OF_ITERATIONS;
    avgrtime2 /= NUM_OF_ITERATIONS;
    avgrtime3 /= NUM_OF_ITERATIONS;
    avgrtime4 /= NUM_OF_ITERATIONS;
    avgiotime1 /= NUM_OF_ITERATIONS;
    avgiotime2 /= NUM_OF_ITERATIONS;
    avgiotime3 /= NUM_OF_ITERATIONS;
    avgiotime4 /= NUM_OF_ITERATIONS;

    printf(1,"med calc times: wtime: %d, rtime: %d, iotime: %d\n",avgwtime1,avgrtime1,avgiotime1);
    printf(1,"large calc times: wtime: %d, rtime: %d, iotime: %d\n",avgwtime2,avgrtime2,avgiotime2);
    printf(1,"med io times: wtime: %d, rtime: %d, iotime: %d\n",avgwtime3,avgrtime3,avgiotime3);
    printf(1,"large io times: wtime: %d, rtime: %d, iotime: %d\n",avgwtime4,avgrtime4,avgiotime4);
    
    exit();
}

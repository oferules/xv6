#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"


int sys_yield(void)
{
  yield(); 
  return 0;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int
sys_setVariable(void)
{
  char * var;
  char * value;
  if(argptr(0, &var, 32) < 0 || argptr(1, &value, 100) < 0 )
    ///TODO check retrun value
    return -3;
  return setVariable(var,value);
}

int
sys_getVariable(void)
{
  char * var;
  char * value;
  if(argptr(0, &var, 32) < 0 || argptr(1, &value, 100) < 0 )
    return -3;
  return getVariable(var,value);
}

int
sys_remVariable(void)
{
  char * var;
  if(argptr(0, &var, 32) < 0)
    return -3;
  return remVariable(var);
}

int
sys_wait2(void)
{
  int pid;
  int* wtime;
  int* rtime;
  int* iotime;

  if(argint(0, &pid) < 0 || argptr(1, (char**) &wtime, 4) < 0 || argptr(2, (char**) &rtime, 4) < 0 || argptr(3, (char**) &iotime, 4) < 0)
    return -3;
  return wait2(pid, wtime, rtime, iotime);
}

int
sys_set_priority(void)
{
  int priority;
  if(argint(0, &priority) < 0)
    return -3;
  return set_priority(priority);
}

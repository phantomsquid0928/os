#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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

/**
 * mod
 * get time with cmostime(struct rtcdate *r);, returns it.
*/
int sys_date(void) {
  struct rtcdate *d;
  if (argptr(0, (void *)&d, sizeof(*d)) < 0) //important!!!! sysfile.c sys_fstat uses similar feature
    return -1;
  cmostime(d);
  return 0;
}
/**
 * mod
 * make alarm by ?
*/
int sys_alarm(void) {  //여ㅕ기서 틱증증가로 바꿔도 댈듯
  int time;
  // struct rtcdate d;
  if (argint(0, &time) < 0)
    return -1;
  if (time <= 0){
    cprintf("timer must bigger than 0\n");
    kill(myproc()->pid);
    return 0;
  }
  // cmostime(&d);
  acquire(&tickslock);
  //cprintf("ticks : %d\n", ticks);
  //release(&tickslock);
  struct proc * p = myproc();
  p->alarmticks = 0;
  p->alarm_timer.seconds = time;
  p->alarm_timer.ison = 1;
  
  //myproc()->state = RUNNABLE;
  
  release(&tickslock);
  return 0;
}

int sys_alert(void) {
  struct rtcdate r;
  cmostime(&r);
  cprintf("SSU_Alarm!\n");
  cprintf("Current time : %d-%d-%d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
  return 0;
}
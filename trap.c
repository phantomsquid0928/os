#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
// #include "date.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;
// struct trapframe * tf2 = 0;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)  
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    
    // cprintf("cpuid : %d, pid : %d, ticks : %d\n", cpuid(), myproc() ? myproc()->pid : -1, myproc() ?myproc()->proc_tick : -1);
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      struct proc *p = myproc();     //프로세스 가져오는 코드 -> trap 호출 대상이 프로세스면 반환되고, 아닐시 0
      if (p && p->alarm_timer.ison != 0) { //trap을 호출한 대상이 프로세스인지 검사. 프로세스일 때, 알람이 켜져 있다면 실행된다
        // struct trapframe * tf2 = (struct trapframe*)kalloc();
        // tf2->eax = 25;
        p->alarmticks++;
        if (p->alarm_timer.seconds * 100 <= p->alarmticks) {  //알람 울리는 시간이 됬다면
          p->killed = 1;       //프로세스 종료
          p->alarm_timer.ison = 0; //알람이 꺼졌음을 표시
          // tf2 = (struct trapframe*)kalloc();
          // tf2->eax = 25;
          //cprintf("ticks ; %d\n", ticks);      //deprecated 그냥 현 프로세스 tf에 25넣고 호출해도 돌아가긴 한다.
          myproc()->tf->eax = 25;        //시스템 콜 25번 강제 호출루틴
          syscall();                    //이거로 강제 호출 가능
        }
      }
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER) {
#ifdef DEFAULT
      // yield();
      myproc()->proc_tick++;
      myproc()->cpu_used++;
      if (myproc()->proc_tick == 30) {
        yield();
      }
#else
#ifdef CHANGED
      // yield();
      myproc()->proc_tick++;
      myproc()->cpu_used++;
      if (myproc()->proc_tick == 30) {
        yield();
      }
      if (myproc()->proc_tick == 60) {
        update_priority();
      }
#endif
#endif
  }
    

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
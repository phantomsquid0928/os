#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
// #define CHANGED
#ifdef DEFAULT
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;
void show_ptable_list() {
  // struct proc * p;
  // for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
  //   if (p->state != UNUSED) {
  //     cprintf("     pid : %d priority : %d par: %d--[%s]\n", p->pid, p-> priority, p->parent, states[p->state] );
  //   }
  // }
  // cprintf("\n\n\n");
  struct proc * p;
  int i=0;
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    cprintf("   %d  pid : %d priority : %d par: %d--[%s]\n", i, p->pid, p-> priority, p->parent, states[p->state] );
    i++;
  }
  cprintf("\n\n\n");
}
#else
#ifdef CHANGED
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  int priorities[100];
  run_queue *qproc[25];
} ptable;
// void init_queue() {
//   run_queue * q;
//   for (q = ptable.qproc; q = ptable.qproc[25]; q++) {
//     q->nodescnt = -1;
//     q->next = 0;
//     q->tail = 0;
//   }
// }
void show_ptable_list2() {
  struct proc * p;
  int i=0;
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    cprintf("   %d  pid : %d priority : %d par: %d--[%s]\n", i, p->pid, p-> priority, p->parent, states[p->state] );
    i++;
  }
  cprintf("\n\n\n");
}
void show_ptable_list() {
	for (int i =0; i < 25; i++) {
		run_queue * temp = 0;
    cprintf("qproc[%d]: ", i);
		if (ptable.qproc[i] == 0) {
			cprintf("\n");
			continue;
		}
		temp = ptable.qproc[i];
		cprintf("counts : %d ", temp->nodescnt);
		cprintf("tail : %d ", temp->tail->node->pid);
    cprintf("\n");
		while(temp) {
			cprintf("     pid : %d priority : %d par: %d--[%s]", temp->node->pid, temp -> node -> priority, temp->node->parent, states[temp->node->state] );
			temp = temp -> next;
		}
		cprintf("\n");
	}
}
int pushproc(struct proc *p, run_queue *q) {      //priority queue with linked list??????????  f for garbage
//maybe return priority?
  int priority = p->priority;
  int idx = priority / 4;
  run_queue * temp = q;
  temp= (run_queue *)kalloc();
  p->procnode = temp;
  // cprintf("noeeror");
  temp -> node = p;
  temp -> tail = 0;
  temp -> next = 0;
  temp -> nodescnt = 0;
  if (ptable.qproc[idx] == 0) {
  	ptable.qproc[idx] = temp;
  	ptable.qproc[idx]->tail = temp;
  	temp -> nodescnt = 1;
  }
  else {
  	ptable.qproc[idx]->tail->next = temp;
  	ptable.qproc[idx]->tail = temp;
  	ptable.qproc[idx]->nodescnt++;
  }
  return 1;
}
int pushnode(run_queue * target) {
	int priority = target->node->priority;
	int idx = priority / 4;
	
	target -> tail = 0;
	target -> next = 0;
	if (ptable.qproc[idx] == 0) {
	  	ptable.qproc[idx] = target;
	  	ptable.qproc[idx]->tail = target;
	  	target -> nodescnt = 1;
	}
	else {
	  	ptable.qproc[idx]->tail->next = target;
	  	ptable.qproc[idx]->tail = target;
	  	ptable.qproc[idx]->nodescnt++;
	}
  return 1;
}
/**
 * mod = 1 : pop for schedule
 * mod = 2 : pop for delete
*/
run_queue* pop(int idx, run_queue * p, int pid, int mod) { 
  if (ptable.qproc[idx] == 0) return 0;      //ptable.qproc[idx] == 0으로 꼮!!!!!!1 이러케 검사할것
  run_queue * temp = ptable.qproc[idx];
  run_queue * past = 0;
  run_queue * target = 0;
  run_queue * targetpast = 0;
  if (mod == 1) { //pop for schedule
    while(temp) {
      if (temp -> node ->priority < (target != 0 ? target->node->priority : 100) && temp->node->state == RUNNABLE) {
        target = temp;
        targetpast = past;
      }
      past = temp;
      temp = temp -> next;
    }
  }
  if (mod == 2) { //pop for del - process erased
    while(temp) {
      if (temp->node->pid == pid) {
        target = temp;
        targetpast = past;
      }
      past = temp;
      temp = temp -> next;
    }
  }
  if (target == 0) return 0; 
  if (targetpast == 0) {
    if (ptable.qproc[idx]->nodescnt == 1) {
      ptable.qproc[idx] = 0;
      return target;  //*p = *target;
		}
		int temp = ptable.qproc[idx]->nodescnt;
		run_queue * temptail = ptable.qproc[idx]->tail;
	//			cout << temp << "fsfsf ";
		ptable.qproc[idx] = target->next;
		ptable.qproc[idx]->nodescnt = temp - 1;
		ptable.qproc[idx]->tail = temptail;
	//			cout << "changed" << x << "th arr with p["<< ptable.qproc[x]->node->pid << "]";
	}
	else {
		if (target->next == 0){ //tail
			ptable.qproc[idx]->tail = targetpast;
			targetpast->next = 0;
			ptable.qproc[idx]->nodescnt--;
		}
		else {
			ptable.qproc[idx]->nodescnt--;
			targetpast -> next = target -> next;
		}
	}
  return target;
  
}//////////////////// change priority func needed???????????
run_queue* popforsched(int idx, run_queue *q) {
  if ((q = pop(idx, q, 0, 1)) == 0) return 0;
  ptable.priorities[q->node->priority]++;
  if (pushnode(q) == 0) return 0;
  return q;
}
run_queue * popfordel(int pid, run_queue *q) {
  for (int i =0 ; i<25; i++) {
    if ((q = pop(i, q, pid, 2)) == 0) {
      return 0;
    }
  }
  return q;
}
int update_priority() {
	int ilist[100];       //processes cannot exceed 64
	run_queue * clist[100];
	run_queue * plist[100];
	int cnt =0 ;
  int i;
	for (i = 0; i < 25; i++) {
		if (ptable.qproc[i] == 0) continue;
		run_queue * past = 0;
		run_queue * temp;
		temp = ptable.qproc[i];
		while(temp) {
      int old = temp->node->priority;
			temp->node->priority = temp->node->priority + (int)(temp->node->proc_tick / 10);
      if (temp->node->priority > 99) temp->node->priority = 99; //priority max < 100
      ptable.priorities[old]--;
      ptable.priorities[temp->node->priority]++;
			if ((int)temp->node->priority / 4 != i) { //needs update this run_queue node location
				clist[cnt] = temp;
				plist[cnt] = past;
				ilist[cnt] = i;
				cnt++;
			}
			past = temp;
			temp = temp -> next;
		}
	}
	
	for (i = 0; i< cnt; i++) {     //del queuenode
		int x = ilist[i];
		if (plist[i] == 0) {
			int temp = ptable.qproc[x]->nodescnt;
			run_queue * temptail = ptable.qproc[x]->tail;
//			cout << temp << "fsfsf ";
			ptable.qproc[x] = clist[i]->next;
			ptable.qproc[x]->nodescnt = temp -1;
			ptable.qproc[x]->tail = temptail;
//			cout << "changed" << x << "th arr with p["<< ptable.qproc[x]->node->pid << "]";
		}
		else {
			if (clist[i]->next == 0){ //tail
				ptable.qproc[x]->tail = plist[i];
				plist[i]->next = 0;
				ptable.qproc[x]->nodescnt--;
			}
			else {
				ptable.qproc[x]->nodescnt--;
				plist[i] -> next = clist[i] -> next;
			}
		}
	}
	
	for (i = 0; i< cnt; i++) {       //repush
    clist[i]->node->proc_tick = 0; //reset proctick that calced.
		pushnode(clist[i]);
	}
	return 0;
}
#endif
#endif

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  // #ifdef DEFAULT

  #ifdef CHANGED
  run_queue *q = 0;

  p->proc_tick = 0;
  p->cpu_used = 0;
  int i;
  for (i = 0; i < 100; i++) {       //give priority that min of priorities
    if (ptable.priorities[i] != 0) break;
  }
  p->priority = i;
  if (i == 100) p->priority = 99;
  cprintf("p->priority: %d", p->priority);

  ptable.priorities[p->priority]++;
  // show_ptable_list();
  #endif

  release(&ptable.lock);
  // #endif

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    // #ifdef CHANGED
    // release(&ptable.lock);
    // #endif
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  #ifdef CHANGED
  pushproc(p, q);
  #endif
  // cprintf("fffff");
  //mod
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  // show_ptable_list();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;
  #ifdef CHANGED
  ptable.priorities[p->priority]--; //give userinit priority as 0
  p->priority = 0;
  ptable.priorities[0] = 1; //mod
  #endif

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();
  // cprintf("fork : sttt");
  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }
  // cprintf("ttttttfttttttttt");
  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  // cprintf("sssssss");
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);
  cprintf("fork suces : %d\n", pid);
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;
  // cprintf("exiting... %d, par : %d", curproc->pid, curproc->parent->pid);
  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock); //?

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  // #ifdef DEFAULT
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }
  // #else
  // #ifdef CHANGED
  // int i;
  // for (i = 0; i < 25; i++) {
  //   if (ptable.qproc[i] == 0) continue;
  //   run_queue * q = ptable.qproc[i];
  //   while(q) {
  //     p = q -> node;
  //     if (p->parent == curproc) {
  //       p->parent = initproc;
  //       if (p->state == ZOMBIE)
  //       wakeup1(initproc);
  //     }
  //     q = q -> next;
  //   }
  // }
  // #endif
  // #endif
  
  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  // show_ptable_list();
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    // #ifdef DEFAULT
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1; 
      if(p->state == ZOMBIE){
        // Found one.
        #ifdef CHANGED
        run_queue * temp = 0;
        if ((temp = popfordel(p->pid, temp)) != 0) {
          kfree((char*)temp);
        }
        #endif
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }
    // #else
    // #ifdef CHANGED
    // int i;
    // int cnt = 0;
    // int ilist[100];       //processes cannot exceed 64
	  // run_queue * clist[100];
    // for (i = 0; i < 25; i++) {
    //   if (ptable.qproc[i] == 0) continue;
    //   run_queue * temp = ptable.qproc[i];
    //   while(temp) {
    //     p = temp->node;
    //     if (p -> parent == curproc)
    //       break;
    //     temp = temp -> next;
    //   }
    //   havekids = 1;
    //   if (p->state == ZOMBIE) {
    //     ilist[cnt] = i;
    //     clist[cnt] = temp;
    //     cnt++;
    //   }
    // }
    // for (i = 0; i < cnt; i++) {
    //   int idx = ilist[i];
    //   p = clist[i]->node;
    //   run_queue * temp = clist[i];
    //   run_queue * s;
    //   if ((s = popfordel(idx, p->pid, temp)) != 0) {
    //       kfree((char*)temp);
    //   }
      
    //   pid = p->pid;
    //   kfree(p->kstack);
    //   p->kstack = 0;
    //   freevm(p->pgdir);
    //   p->pid = 0;
    //   p->parent = 0;
    //   p->name[0] = 0;
    //   p->killed = 0;
    //   p->state = UNUSED;
    //   release(&ptable.lock);
    //   return pid;
    // }
    // #endif
    // #endif

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  #ifdef DEFAULT
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
      // cprintf("\n scheduler] : seleced: %d\n", p->pid);
      
      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    // show_ptable_list();
    release(&ptable.lock);

  }
  #else
  #ifdef CHANGED
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for (;;) {
    sti();
    // cprintf("ss");
    acquire(&ptable.lock);
    int i = 0;
    for (i = 0; i < 25; i++) {
      if (ptable.qproc[i] == 0) continue;
      run_queue * target = 0;
      // cprintf("selecting...\n");
      if ((target = popforsched(i, target)) == 0) continue; //o(n)
      p = target->node;
      // cprintf("\n scheduler] : seleced: %d\n", p->pid);
      c->proc = p;
      // cprintf("pid : %d, priro: %d,name : %s", p->pid, p->priority, p->name);
      // cprintf("kstack::::; %s\n", p->kstack);
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      c->proc = 0;
    }
    // show_ptable_list2();
    release(&ptable.lock);
  }
  #endif
  #endif
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  #ifdef DEFAULT
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
  #else
  #ifdef CHANGED
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan) {
      int i = 0;
      for (i = 0; i < 100; i++) {       //give priority that min of priorities for wake
        if (ptable.priorities[i] != 0) break;
      }
      p->priority = i;
      if (i == 100) p->priority = 99;
      ptable.priorities[p->priority]++;
      p->state = RUNNABLE;
    }
  #endif
  #endif
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
#ifdef LAB_TRAPS
  backtrace();
#endif
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 start_addr;
  argaddr(0, &start_addr);
  int num;
  argint(1, &num);
  uint64 address;
  argaddr(2, &address);

  unsigned int mask = 0x0;
  struct proc * my_proc = myproc();
  pagetable_t * pagetable = &(my_proc -> pagetable);
  for(int i = 0; i < num; i++)
  {
    pte_t * pte = walk(*pagetable, start_addr + i * PGSIZE, 0);
    //use walk to find the page table entrys
    if(pte == 0)
    {
      return -1;
    }
    if(*pte & PTE_A)
    {
      mask |= (1 << i);
      //This means this page is accessed
    }
    *(pte) &= ~PTE_A;
    //erase the PTE_A bit(I just check, this is not access)
  }
  copyout(*pagetable, address, (char *)&mask, sizeof(mask));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// click the sys call number in p->tracemask
// so as to tracing its calling afterwards
uint64 
sys_trace(void) {
  int trace_sys_mask;
  argint(0,&trace_sys_mask);
  myproc()->tracemask = trace_sys_mask;
  return 0;
}

uint64
sys_sysinfo(void) {
  uint64 Addr;
  argaddr(0, &Addr);

  struct sysinfo info;
  info.freemem = kfreemem();
  info.nproc = count_free_proc();

  if(copyout(myproc()->pagetable, Addr, (char *)&info, sizeof(info)) < 0) {
    return -1;
  }
  return 0;
}

uint64
sys_sigalarm(void) {
  int interval;
  uint64 handler;
  argint(0, &interval);
  argaddr(1, &handler);
  myproc()->passed_ticks = 0;
  myproc()->alarm_interval = interval;
  myproc()->alarm_handler = handler;
  return 0;
}

uint64
sys_sigreturn(void){
    struct proc *p = myproc();
    *p->trapframe = *p->alarmframe;
    p->trap_in = 0;
    p->passed_ticks = 0;
    return (*p->trapframe).a0;
}
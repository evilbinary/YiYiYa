#include "cpu.h"
#include "acpi.h"
#include "../io.h"
#include "mm.h"
#include "../interrupt.h"

u32 lapic_cpus_id[MAX_CPU];

static inline void ia32_msr_read(u32 msr, u32 *lo, u32 *hi){
   __asm__ __volatile__("rdmsr":"=a"(*lo),"=d"(*hi):"c"(msr));
}

static inline void ia32_msr_write(u32 msr, u32 lo, u32 hi){
   __asm__ __volatile__("wrmsr"::"a"(lo),"d"(hi),"c"(msr));
}

u32 acpi_get_id(){
    //xAPIC Mode
    //P6 family and Pentium processors
    //Pentium 4 processors, Xeon processors, and later processors
    return mem_read32(APIC_BASE+LAPIC_ID) >> 24;
}


INTERRUPT_SERVICE
void lapic_do_timer() {
  interrupt_entering_code(0x60,0);
  kprintf("lapic_do_timer\n");
  //interrupt_process(do_schedule);
//   interrupt_exit_context();
    cpu_halt();
}

void lapic_timer_init(){
    u32 value;

    kprintf("init local timer of cpu %d\n",cpu_get_id());

    //Divide Configuration Register 011: Divide by 16,111 divede by 1
    mem_write32(APIC_BASE+LAPIC_TDCR,0xb);

    //init count lvt timer
    mem_write32(APIC_BASE+LAPIC_TICR,0xFFFFFFFF);

    // //mask 
    // u32 value=mem_read32(APIC_BASE+LAPIC_TIMER);
    // mem_write32(APIC_BASE+LAPIC_TIMER,value|APIC_LVT_MASKED);
    
    //read count Current Count Registers
    u32 count = mem_read32(LAPIC_TCCR);
    u32 ticks = 0xFFFFFFFF - count ;

    kprintf("time ticks %d\n",ticks);

    ticks = 1000;

    //TSC-Deadline mode,vector 0x20, program target value in IA32_TSC_DEADLINE MSR.
    value=mem_read32(APIC_BASE+LAPIC_TIMER);
    value &= (~APIC_LVT_MASKED);//unmask
    value |=0x60 | APIC_TIMER_MODE_PERIODIC;
    mem_write32(APIC_BASE+LAPIC_TIMER,value) ;
    mem_write32(APIC_BASE+LAPIC_TDCR,3);
    mem_write32(APIC_BASE+LAPIC_TICR, ticks)  ;

	// mem_write32(APIC_BASE+LAPIC_SVR, mem_read32(APIC_BASE+LAPIC_SVR) | 0x60);
    // lapic_eoi();

    if ((mem_read32(APIC_BASE+LAPIC_TIMER) & APIC_LVT_MASKED) == 0)	{
		kprintf("init local timer success\n");
        interrutp_regist(0x60, lapic_do_timer);
        
        // mem_write32(APIC_BASE+LAPIC_ESR,0);

        // value = mem_read32(APIC_BASE+LAPIC_SVR);	// Spurious Interrupt Vector reg
        // value |= APIC_SPIV_APIC_ENABLED; // set enable bit explicitly
        // mem_write32(APIC_BASE+LAPIC_SVR, value);

        // lapic_eoi();        
	} else {
		for(;;){
            cpu_halt();
        }
	}
}

int lapic_init(){

    // mask all interrupts ([16] = 1)
	mem_write32(APIC_BASE+LAPIC_TIMER, 0x10000);	// local timer
	mem_write32(APIC_BASE+LAPIC_THERMAL, 0x10000);	// Thermal monitor 
	mem_write32(APIC_BASE+LAPIC_PERF, 0x10000);	// Performance monitor
	mem_write32(APIC_BASE+LAPIC_LINT1, 0x10000);	// Local INT #1
	//mem_write32(APIC_BASE+LAPIC_LINT0, 0x10000);	// Local INT #0	
	mem_write32(APIC_BASE+LAPIC_ERROR, 0x10000);	// Error

    //clear interrupt
    mem_write32(APIC_BASE+LAPIC_TPR,0);  

    // logical destination mode
    mem_write32(APIC_BASE+LAPIC_DFR, 0xffffffff);   // Flat mode
    mem_write32(APIC_BASE+LAPIC_LDR, 0x01000000);   // All cpus use logical id 1
  
    //特权指令
    int msrh,msrl;
    ia32_msr_read(APIC_BASE,&msrl,&msrh);
    msrl |= (1 << IA32_APIC_BASE_ENABLE_BIT);
    ia32_msr_write(APIC_BASE,&msrl,&msrh);

    //8 Local-APIC is Enabled  (1=yes, 0=no)
    u32 val=mem_read32(APIC_BASE+LAPIC_SVR);
    val|=0x100;//Spurious Interrupt Vector Register bit 8 to start receiving interrupts
    mem_write32(APIC_BASE+LAPIC_SVR,val);

  

    return 0;
}


/**
 * @description: 初始化cpu
 * @param {u32} id
 * @return {*}
 */
void lapic_send_init(u32 id){
    //参考 10.6.1 Interrupt Command Register (ICR)
    
    //56-63 Destination Field=id
    mem_write32(APIC_BASE+LAPIC_ICRHI, id << ICR_DESTINATION_SHIFT);
    //0-7 Vector
    //8-10 Delivery Mode = b101 init
    //11 Destination Mode=0 Physical
    //14 Level =1 Assert
    //15 Trigger Mode= 0 Edge
    //18-19 Destination Shorthand= 00 No Shorthand
    mem_write32(APIC_BASE+LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL
        | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    cpu_delay(200);

    //12 Delivery Status 0: Idle 1: Send Pending
    while (mem_read32(APIC_BASE+LAPIC_ICRLO) & ICR_SEND_PENDING)
        ;

}

/**
 * @description: 启动cpu
 * @param {u32} id  cpu的id
 * @param {u32} vec 发送的中断号
 * @return {*}
 */
void lapic_send_start(u32 id,u32 vec){
    //参考 10.6.1 Interrupt Command Register (ICR)

    //触发两次
    for(int i=0;i<2;i++){
        mem_write32(APIC_BASE+LAPIC_ICRHI, id << ICR_DESTINATION_SHIFT);
        //8-10 Delivery Mode = b110: Start Up
        mem_write32(APIC_BASE+LAPIC_ICRLO, vec | ICR_STARTUP
            | ICR_PHYSICAL | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

        cpu_delay(200);
    }

    //12 Delivery Status 0: Idle 1: Send Pending
    while (mem_read32(APIC_BASE+LAPIC_ICRLO) & ICR_SEND_PENDING)
        ;
}

u32 lapic_get_index(int idx){
    if(idx<0||idx>cpu_get_number()){
        kprintf("out of bound get cpu idx\n");
        return 0;
    }
    return lapic_cpus_id[idx];
}

void lapic_eoi(void){
	mem_write32(APIC_BASE+LAPIC_EOI, 0);
}



void acpi_init(){
    //todo acpi 解析
    u32 address=APIC_BASE;
    map_page(address, address, PAGE_P | PAGE_USU | PAGE_RWW);
    for(int i=0;i<MAX_CPU;i++){
        lapic_cpus_id[i]=i;
    }
   

    //使用local acpi初始化
    lapic_init();
    //使用 apic timer
    // lapic_timer_init();
}

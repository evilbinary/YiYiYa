#include "cpu.h"
#include "acpi.h"
#include "../io.h"

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

int lapic_init(){

    //clear interrupt
    mem_write32(APIC_BASE+LAPIC_TPR,0);

    //特权指令
    int msrh,msrl;
    ia32_msr_read(APIC_BASE,&msrl,&msrh);
    msrl |= (1 << IA32_APIC_BASE_ENABLE_BIT);
    ia32_msr_write(APIC_BASE,&msrl,&msrh);

    // logical destination mode
    mem_write32(APIC_BASE+LAPIC_DFR, 0xffffffff);   // Flat mode
    mem_write32(APIC_BASE+LAPIC_LDR, 0x01000000);   // All cpus use logical id 1

    u32 val=mem_read32(APIC_BASE+LAPIC_SVR);
    val|=0x100;//Spurious Interrupt Vector Register bit 8 to start receiving interrupts
    mem_write32(APIC_BASE+LAPIC_SVR,val);

    return 0;
}


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

    //12 Delivery Status 0: Idle 1: Send Pending
    while (mem_read32(APIC_BASE+LAPIC_ICRLO) & ICR_SEND_PENDING)
        ;

}

void lapic_send_start(u32 id,u32 vec){
    //参考 10.6.1 Interrupt Command Register (ICR)

    mem_write32(APIC_BASE+LAPIC_ICRHI, id << ICR_DESTINATION_SHIFT);
    //8-10 Delivery Mode = b110: Start Up
    mem_write32(APIC_BASE+LAPIC_ICRLO, vec | ICR_STARTUP
        | ICR_PHYSICAL | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

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


void acpi_init(){
    //todo acpi 解析

    for(int i=0;i<MAX_CPU;i++){
        lapic_cpus_id[i]=i;
    }
    //使用local acpi初始化
    lapic_init();
}
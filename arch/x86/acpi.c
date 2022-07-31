#include "cpu.h"
#include "acpi.h"
#include "../io.h"


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
    return io_read32(APIC_BASE+LAPIC_ID) >> 24;
}


int lapic_init(){

    //clear interrupt
    io_write32(APIC_BASE+LAPIC_TPR,0);

    //特权指令
    int msrh,msrl;
    ia32_msr_read(APIC_BASE,&msrl,&msrh);
    msrl |= (1 << IA32_APIC_BASE_ENABLE_BIT);
    ia32_msr_write(APIC_BASE,&msrl,&msrh);

    // logical destination mode
    io_write32(APIC_BASE+LAPIC_DFR, 0xffffffff);   // Flat mode
    io_write32(APIC_BASE+LAPIC_LDR, 0x01000000);   // All cpus use logical id 1

    u32 val=io_read32(APIC_BASE+LAPIC_SVR);
    val|=0x100;//Spurious Interrupt Vector Register bit 8 to start receiving interrupts
    io_write32(APIC_BASE+LAPIC_SVR,val);

    return 0;
}

void lapic_send_init(u32 id){
    io_write32(APIC_BASE+LAPIC_ICRHI, id << ICR_DESTINATION_SHIFT);
    io_write32(APIC_BASE+LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL
        | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    while (io_read32(APIC_BASE+LAPIC_ICRLO) & ICR_SEND_PENDING)
        ;

}

void lapic_send_start(u32 id,u32 vec){
    io_write32(APIC_BASE+LAPIC_ICRHI, id << ICR_DESTINATION_SHIFT);
    io_write32(APIC_BASE+LAPIC_ICRLO, vec | ICR_STARTUP
        | ICR_PHYSICAL | ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    while (io_read32(APIC_BASE+LAPIC_ICRLO) & ICR_SEND_PENDING)
        ;
}
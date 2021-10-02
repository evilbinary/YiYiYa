static int afe;

static unsigned int pa2va(unsigned int pa)
{
    return pa;
    // unsigned int va;
    // unsigned int vaddr;
    // unsigned int paddr;
    // unsigned int i;

    // va = 0;
    // for (i = 0; i < 0x100000; i++)
    // {
    //     vaddr = i << 12;
    //     __asm__("mcr p15,0,%1,c7,c8,0\n\t"
    //             "mrc p15,0,%0,c7,c4,0\n\t" : "=r" (paddr) : "r" (vaddr));
    //     if ((pa & 0xFFFFF000) == (paddr & 0xFFFFF000))
    //     {
    //         va = vaddr + (pa & 0xFFF);
    //         break;
    //     }
    // }
    // return va;
}

static void mmu_get_perms(int ap2, int ap1, int *ur, int *uw, int *pr, int *pw)
{
    /* AFE enabled, use simple permissions */
    if (afe)
    {
        *ur = ap1 > 1;
        *uw = !ap2 && ap1 > 1;
        *pr = 1;
        *pw = !ap2 && ap1 < 2;
    }
    else
    {
        *pw = (!ap2 && ap1);
        *pr = *pw || ap1;
        *ur = ap1 > 1;
        *uw = !ap2 && ap1 == 3;
    }
}

static void mmu_dump_pages(unsigned int vaddr, unsigned int entry)
{
    int xn;
    int ng;
    int s;
    int ap2;
    int ap1;
    int pr;
    int pw;
    int ur;
    int uw;
    unsigned int paddr;

    if ((entry & 0x3) == 0x1) /* large page */
    {
        xn = entry & 0x8000;
        ng = entry & 0x800;
        s = entry & 0x400;
        ap2 = entry & 0x200;
        ap1 = (entry >> 4) & 3;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = entry & 0xFFFF0000;
        kprintf("-[0x%x] %s PA:0x%x NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Lg Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
    }
    else if ((entry & 0x2)) /* small page */
    {
        xn = entry & 1;
        ng = entry & 0x800;
        s = entry & 0x400;
        ap2 = entry & 0x200;
        ap1 = (entry >> 4) & 3;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = entry & 0xFFFFF000;
        kprintf("-[0x%x] %s PA:0x%x NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
    }
    else
    {
        kprintf("-[0x%x] %s\n", vaddr, "Unmapped ");
    }
}

static void mmu_dump_sections(unsigned int vaddr, unsigned int entry)
{
    int ns;
    int ss;
    int ng;
    int s;
    int ap1;
    int ap2;
    int domain;
    int xn;
    int pr;
    int pw;
    int ur;
    int uw;
    unsigned int paddr;
    unsigned int i;
    unsigned int test;
    unsigned int *tbl;
    unsigned int tblentry;

    // if(vaddr!=0x01C2A000 ) return; 


    if ((entry & 0x3) == 2) /* section or supersection */
    {
        ns = entry & 0x80000;
        ss = entry & 0x40000;
        ng = entry & 0x20000;
        s = entry & 0x10000;
        ap2 = entry & 0x8000;
        ap1 = (entry >> 10) & 3;
        domain = (entry >> 5) & 15;
        xn = entry & 0x10;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = ss ? entry & 0xFF000000 : entry & 0xFFF00000;

        kprintf("[0x%x] %s PA:0x%x NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d NS:%d DOM:%x\n", vaddr, ss ? "S-Section " : "Section   ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn, !!ns, domain);
    }
    else if ((entry & 0x3) == 1) /* page table */
    {
        domain = (entry >> 5) & 15;
        ns = entry & 8;
        paddr = entry & 0xFFFFFC00 ;
        tbl = (unsigned int *)pa2va(paddr);
        kprintf("[0x%x] %s PA:0x%x VA:0x%x NS:%d DOM:%x\n", vaddr, "Page TBL  ", paddr, tbl, !!ns, domain);
        for (i = 0; i < 0x100; i++)
        {
            mmu_dump_pages(vaddr+(i<<12), tbl[i]);
        }
    }
    else if ((entry & 0x3) == 0) /* not mapped */
    {
        kprintf("[0x%x] %s\n", vaddr, "Unmapped  ");
    }
    else
    {
        kprintf("[0x%x] %s\n", vaddr, "Invalid   ");
    }
}

int mmu_dump(void)
{
    unsigned int ttbr[2];
    int ttbcr;
    int n;
    unsigned int i;
    int paddr;
    unsigned int *ttb_vaddr[2];
    unsigned int entry;

    __asm__("mrc p15,0,%0,c2,c0,0" : "=r" (ttbr[0]));
    __asm__("mrc p15,0,%0,c2,c0,1" : "=r" (ttbr[1]));
    __asm__("mrc p15,0,%0,c2,c0,2" : "=r" (ttbcr));
    kprintf("TTBR0: 0x%x, TTBR1: 0x%x, TTBCR: 0x%x\n", ttbr[0], ttbr[1], ttbcr);

    n = ttbcr & 0x7;
    ttbr[0] &= (unsigned int)((int)0x80000000 >> (31 - 14 + 1 - n));
    ttbr[1] &= 0xFFFFC000;

    ttb_vaddr[0] = (unsigned int *)pa2va(ttbr[0]);
    ttb_vaddr[1] = (unsigned int *)pa2va(ttbr[1]);

    kprintf("TBBR0 (physical): 0x%x, (virtual): 0x%x\n", ttbr[0], i<<12);
    kprintf("TBBR1 (physical): 0x%x, (virtual): 0x%x\n", ttbr[1], i<<12);

    unsigned int* p=(unsigned int *) (ttbr[0]);
    kprintf("Dumping TTBR0...\n");
    for (i = 0; i < (1 << 12 - n); i++)
    {
        entry = ttb_vaddr[0][i];
        mmu_dump_sections(i<<20, entry);
    }

    if (n)
    {
        kprintf("Dumping TTBR1...\n");
        for (i = ((~0xEFFF & 0xFFFF) >> n); i < 0x1000; i++)
        {
            entry = ttb_vaddr[1][i];
            mmu_dump_sections(i<<20, entry);
        }
    }
}


int mmu_dump_page(unsigned int ttbr0,unsigned int ttbr1,unsigned int ttbcr){
    int n;
    unsigned int i;
    int paddr;
    unsigned int *ttb_vaddr[2];
    unsigned int entry;
    unsigned int ttbr[2];

    ttbr[0]=ttbr0;
    ttbr[1]=ttbr1;

    kprintf("TTBR0: 0x%x, TTBR1: 0x%x, TTBCR: 0x%x\n", ttbr[0], ttbr[1], ttbcr);

    n = ttbcr & 0x7;
    ttbr[0] &= (unsigned int)((int)0x80000000 >> (31 - 14 + 1 - n));
    ttbr[1] &= 0xFFFFC000;

    ttb_vaddr[0] = (unsigned int *)pa2va(ttbr[0]);
    ttb_vaddr[1] = (unsigned int *)pa2va(ttbr[1]);

    kprintf("TBBR0 (physical): 0x%x, (virtual): 0x%x\n", ttbr[0], i<<12);
    kprintf("TBBR1 (physical): 0x%x, (virtual): 0x%x\n", ttbr[1], i<<12);

    unsigned int* p=(unsigned int *) (ttbr[0]);
    kprintf("Dumping TTBR0...\n");
    for (i = 0; i < (1 << 12 - n); i++)
    {
        entry = ttb_vaddr[0][i];
        mmu_dump_sections(i<<20, entry);
    }

    if (n)
    {
        kprintf("Dumping TTBR1...\n");
        for (i = ((~0xEFFF & 0xFFFF) >> n); i < 0x1000; i++)
        {
            entry = ttb_vaddr[1][i];
            mmu_dump_sections(i<<20, entry);
        }
    }
}

int start_dump(void)
{
    unsigned int sctlr;

    __asm__("mrc p15,0,%0,c1,c0,0" : "=r" (sctlr));
    afe = sctlr & 0x20000000;
    mmu_dump();

    return 0;
}
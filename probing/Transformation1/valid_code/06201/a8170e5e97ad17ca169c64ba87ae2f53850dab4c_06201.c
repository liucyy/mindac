target_phys_addr_t memory_region_section_get_iotlb(CPUArchState *env,

                                                   MemoryRegionSection *section,

                                                   target_ulong vaddr,

                                                   target_phys_addr_t paddr,

                                                   int prot,

                                                   target_ulong *address)

{

    target_phys_addr_t iotlb;

    CPUWatchpoint *wp;



    if (memory_region_is_ram(section->mr)) {

        

        iotlb = (memory_region_get_ram_addr(section->mr) & TARGET_PAGE_MASK)

            + memory_region_section_addr(section, paddr);

        if (!section->readonly) {

            iotlb |= phys_section_notdirty;

        } else {

            iotlb |= phys_section_rom;

        }

    } else {

        

        iotlb = section - phys_sections;

        iotlb += memory_region_section_addr(section, paddr);

    }



    

    QTAILQ_FOREACH(wp, &env->watchpoints, entry) {

        if (vaddr == (wp->vaddr & TARGET_PAGE_MASK)) {

            

            if ((prot & PAGE_WRITE) || (wp->flags & BP_MEM_READ)) {

                iotlb = phys_section_watch + paddr;

                *address |= TLB_MMIO;

                break;

            }

        }

    }



    return iotlb;

}
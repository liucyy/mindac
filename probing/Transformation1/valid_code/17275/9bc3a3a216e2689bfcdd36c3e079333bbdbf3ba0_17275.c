static void ehci_advance_periodic_state(EHCIState *ehci)

{

    uint32_t entry;

    uint32_t list;

    const int async = 0;



    



    switch(ehci_get_state(ehci, async)) {

    case EST_INACTIVE:

        if (!(ehci->frindex & 7) && ehci_periodic_enabled(ehci)) {

            ehci_set_state(ehci, async, EST_ACTIVE);

            

        } else

            break;



    case EST_ACTIVE:

        if (!(ehci->frindex & 7) && !ehci_periodic_enabled(ehci)) {

            ehci_queues_rip_all(ehci, async);

            ehci_set_state(ehci, async, EST_INACTIVE);

            break;

        }



        list = ehci->periodiclistbase & 0xfffff000;

        

        if (list == 0) {

            break;

        }

        list |= ((ehci->frindex & 0x1ff8) >> 1);



        pci_dma_read(&ehci->dev, list, &entry, sizeof entry);

        entry = le32_to_cpu(entry);



        DPRINTF("PERIODIC state adv fr=%d.  [%08X] -> %08X\n",

                ehci->frindex / 8, list, entry);

        ehci_set_fetch_addr(ehci, async,entry);

        ehci_set_state(ehci, async, EST_FETCHENTRY);

        ehci_advance_state(ehci, async);

        ehci_queues_rip_unused(ehci, async, 0);

        break;



    default:

        

        fprintf(stderr, "ehci: Bad periodic state %d. "

                "Resetting to active\n", ehci->pstate);

        assert(0);

    }

}
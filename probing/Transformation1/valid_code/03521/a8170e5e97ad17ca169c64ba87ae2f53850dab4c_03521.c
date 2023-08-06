static void gem_write(void *opaque, target_phys_addr_t offset, uint64_t val,

        unsigned size)

{

    GemState *s = (GemState *)opaque;

    uint32_t readonly;



    DB_PRINT("offset: 0x%04x write: 0x%08x ", offset, (unsigned)val);

    offset >>= 2;



    

    val &= ~(s->regs_ro[offset]);

    

    readonly = s->regs[offset];

    readonly &= s->regs_ro[offset];



    

    val &= ~(s->regs_w1c[offset] & val);



    

    s->regs[offset] = val | readonly;



    

    switch (offset) {

    case GEM_NWCTRL:

        if (val & GEM_NWCTRL_TXSTART) {

            gem_transmit(s);

        }

        if (!(val & GEM_NWCTRL_TXENA)) {

            

            s->tx_desc_addr = s->regs[GEM_TXQBASE];

        }

        if (!(val & GEM_NWCTRL_RXENA)) {

            

            s->rx_desc_addr = s->regs[GEM_RXQBASE];

        }

        break;



    case GEM_TXSTATUS:

        gem_update_int_status(s);

        break;

    case GEM_RXQBASE:

        s->rx_desc_addr = val;

        break;

    case GEM_TXQBASE:

        s->tx_desc_addr = val;

        break;

    case GEM_RXSTATUS:

        gem_update_int_status(s);

        break;

    case GEM_IER:

        s->regs[GEM_IMR] &= ~val;

        gem_update_int_status(s);

        break;

    case GEM_IDR:

        s->regs[GEM_IMR] |= val;

        gem_update_int_status(s);

        break;

    case GEM_PHYMNTNC:

        if (val & GEM_PHYMNTNC_OP_W) {

            uint32_t phy_addr, reg_num;



            phy_addr = (val & GEM_PHYMNTNC_ADDR) >> GEM_PHYMNTNC_ADDR_SHFT;

            if (phy_addr == BOARD_PHY_ADDRESS) {

                reg_num = (val & GEM_PHYMNTNC_REG) >> GEM_PHYMNTNC_REG_SHIFT;

                gem_phy_write(s, reg_num, val);

            }

        }

        break;

    }



    DB_PRINT("newval: 0x%08x\n", s->regs[offset]);

}
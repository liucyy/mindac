static void mipsnet_ioport_write(void *opaque, target_phys_addr_t addr,

                                 uint64_t val, unsigned int size)

{

    MIPSnetState *s = opaque;



    addr &= 0x3f;

    trace_mipsnet_write(addr, val);

    switch (addr) {

    case MIPSNET_TX_DATA_COUNT:

	s->tx_count = (val <= MAX_ETH_FRAME_SIZE) ? val : 0;

        s->tx_written = 0;

        break;

    case MIPSNET_INT_CTL:

        if (val & MIPSNET_INTCTL_TXDONE) {

            s->intctl &= ~MIPSNET_INTCTL_TXDONE;

        } else if (val & MIPSNET_INTCTL_RXDONE) {

            s->intctl &= ~MIPSNET_INTCTL_RXDONE;

        } else if (val & MIPSNET_INTCTL_TESTBIT) {

            mipsnet_reset(s);

            s->intctl |= MIPSNET_INTCTL_TESTBIT;

        } else if (!val) {

            

        }

        s->busy = !!s->intctl;

        mipsnet_update_irq(s);

        break;

    case MIPSNET_TX_DATA_BUFFER:

        s->tx_buffer[s->tx_written++] = val;

        if (s->tx_written == s->tx_count) {

            

            trace_mipsnet_send(s->tx_count);

            qemu_send_packet(&s->nic->nc, s->tx_buffer, s->tx_count);

            s->tx_count = s->tx_written = 0;

            s->intctl |= MIPSNET_INTCTL_TXDONE;

            s->busy = 1;

            mipsnet_update_irq(s);

        }

        break;

    

    case MIPSNET_DEV_ID:

    case MIPSNET_BUSY:

    case MIPSNET_RX_DATA_COUNT:

    case MIPSNET_INTERRUPT_INFO:

    case MIPSNET_RX_DATA_BUFFER:

    default:

        break;

    }

}
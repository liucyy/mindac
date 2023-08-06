static void sdhci_do_adma(SDHCIState *s)

{

    unsigned int n, begin, length;

    const uint16_t block_size = s->blksize & 0x0fff;

    ADMADescr dscr;

    int i;



    for (i = 0; i < SDHC_ADMA_DESCS_PER_DELAY; ++i) {

        s->admaerr &= ~SDHC_ADMAERR_LENGTH_MISMATCH;



        get_adma_description(s, &dscr);

        DPRINT_L2("ADMA loop: addr=" TARGET_FMT_plx ", len=%d, attr=%x\n",

                dscr.addr, dscr.length, dscr.attr);



        if ((dscr.attr & SDHC_ADMA_ATTR_VALID) == 0) {

            

            s->admaerr &= ~SDHC_ADMAERR_STATE_MASK;

            s->admaerr |= SDHC_ADMAERR_STATE_ST_FDS;



            

            if (s->errintstsen & SDHC_EISEN_ADMAERR) {

                s->errintsts |= SDHC_EIS_ADMAERR;

                s->norintsts |= SDHC_NIS_ERR;

            }



            sdhci_update_irq(s);

            return;

        }



        length = dscr.length ? dscr.length : 65536;



        switch (dscr.attr & SDHC_ADMA_ATTR_ACT_MASK) {

        case SDHC_ADMA_ATTR_ACT_TRAN:  



            if (s->trnmod & SDHC_TRNS_READ) {

                while (length) {

                    if (s->data_count == 0) {

                        for (n = 0; n < block_size; n++) {

                            s->fifo_buffer[n] = sd_read_data(s->card);

                        }

                    }

                    begin = s->data_count;

                    if ((length + begin) < block_size) {

                        s->data_count = length + begin;

                        length = 0;

                     } else {

                        s->data_count = block_size;

                        length -= block_size - begin;

                    }

                    dma_memory_write(&address_space_memory, dscr.addr,

                                     &s->fifo_buffer[begin],

                                     s->data_count - begin);

                    dscr.addr += s->data_count - begin;

                    if (s->data_count == block_size) {

                        s->data_count = 0;

                        if (s->trnmod & SDHC_TRNS_BLK_CNT_EN) {

                            s->blkcnt--;

                            if (s->blkcnt == 0) {

                                break;

                            }

                        }

                    }

                }

            } else {

                while (length) {

                    begin = s->data_count;

                    if ((length + begin) < block_size) {

                        s->data_count = length + begin;

                        length = 0;

                     } else {

                        s->data_count = block_size;

                        length -= block_size - begin;

                    }

                    dma_memory_read(&address_space_memory, dscr.addr,

                                    &s->fifo_buffer[begin],

                                    s->data_count - begin);

                    dscr.addr += s->data_count - begin;

                    if (s->data_count == block_size) {

                        for (n = 0; n < block_size; n++) {

                            sd_write_data(s->card, s->fifo_buffer[n]);

                        }

                        s->data_count = 0;

                        if (s->trnmod & SDHC_TRNS_BLK_CNT_EN) {

                            s->blkcnt--;

                            if (s->blkcnt == 0) {

                                break;

                            }

                        }

                    }

                }

            }

            s->admasysaddr += dscr.incr;

            break;

        case SDHC_ADMA_ATTR_ACT_LINK:   

            s->admasysaddr = dscr.addr;

            DPRINT_L1("ADMA link: admasysaddr=0x%lx\n", s->admasysaddr);

            break;

        default:

            s->admasysaddr += dscr.incr;

            break;

        }



        if (dscr.attr & SDHC_ADMA_ATTR_INT) {

            DPRINT_L1("ADMA interrupt: admasysaddr=0x%lx\n", s->admasysaddr);

            if (s->norintstsen & SDHC_NISEN_DMA) {

                s->norintsts |= SDHC_NIS_DMA;

            }



            sdhci_update_irq(s);

        }



        

        if (((s->trnmod & SDHC_TRNS_BLK_CNT_EN) &&

                    (s->blkcnt == 0)) || (dscr.attr & SDHC_ADMA_ATTR_END)) {

            DPRINT_L2("ADMA transfer completed\n");

            if (length || ((dscr.attr & SDHC_ADMA_ATTR_END) &&

                (s->trnmod & SDHC_TRNS_BLK_CNT_EN) &&

                s->blkcnt != 0)) {

                ERRPRINT("SD/MMC host ADMA length mismatch\n");

                s->admaerr |= SDHC_ADMAERR_LENGTH_MISMATCH |

                        SDHC_ADMAERR_STATE_ST_TFR;

                if (s->errintstsen & SDHC_EISEN_ADMAERR) {

                    ERRPRINT("Set ADMA error flag\n");

                    s->errintsts |= SDHC_EIS_ADMAERR;

                    s->norintsts |= SDHC_NIS_ERR;

                }



                sdhci_update_irq(s);

            }

            SDHCI_GET_CLASS(s)->end_data_transfer(s);

            return;

        }



    }



    

    timer_mod(s->transfer_timer,

                   qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + SDHC_TRANSFER_DELAY);

}
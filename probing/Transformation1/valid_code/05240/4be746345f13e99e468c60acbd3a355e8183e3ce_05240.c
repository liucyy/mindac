static void ide_sector_write_cb(void *opaque, int ret)

{

    IDEState *s = opaque;

    int n;



    if (ret == -ECANCELED) {

        return;

    }

    block_acct_done(bdrv_get_stats(s->bs), &s->acct);



    s->pio_aiocb = NULL;

    s->status &= ~BUSY_STAT;



    if (ret != 0) {

        if (ide_handle_rw_error(s, -ret, IDE_RETRY_PIO)) {

            return;

        }

    }



    n = s->nsector;

    if (n > s->req_nb_sectors) {

        n = s->req_nb_sectors;

    }

    s->nsector -= n;

    if (s->nsector == 0) {

        

        ide_transfer_stop(s);

    } else {

        int n1 = s->nsector;

        if (n1 > s->req_nb_sectors) {

            n1 = s->req_nb_sectors;

        }

        ide_transfer_start(s, s->io_buffer, n1 * BDRV_SECTOR_SIZE,

                           ide_sector_write);

    }

    ide_set_sector(s, ide_get_sector(s) + n);



    if (win2k_install_hack && ((++s->irq_count % 16) == 0)) {

        

        timer_mod(s->sector_write_timer,

                       qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + (get_ticks_per_sec() / 1000));

    } else {

        ide_set_irq(s->bus);

    }

}
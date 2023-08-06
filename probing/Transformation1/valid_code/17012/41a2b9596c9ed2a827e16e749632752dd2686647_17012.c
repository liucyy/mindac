static void ide_atapi_cmd_reply_end(IDEState *s)

{

    int byte_count_limit, size, ret;

#ifdef DEBUG_IDE_ATAPI

    printf("reply: tx_size=%d elem_tx_size=%d index=%d\n",

           s->packet_transfer_size,

           s->elementary_transfer_size,

           s->io_buffer_index);

#endif

    if (s->packet_transfer_size <= 0) {

        

        ide_transfer_stop(s);

        s->status = READY_STAT;

        s->nsector = (s->nsector & ~7) | ATAPI_INT_REASON_IO | ATAPI_INT_REASON_CD;

        ide_set_irq(s);

#ifdef DEBUG_IDE_ATAPI

        printf("status=0x%x\n", s->status);

#endif

    } else {

        

        if (s->lba != -1 && s->io_buffer_index >= s->cd_sector_size) {

            ret = cd_read_sector(s->bs, s->lba, s->io_buffer, s->cd_sector_size);

            if (ret < 0) {

                ide_transfer_stop(s);

                ide_atapi_io_error(s, ret);

                return;

            }

            s->lba++;

            s->io_buffer_index = 0;

        }

        if (s->elementary_transfer_size > 0) {

            

            size = s->cd_sector_size - s->io_buffer_index;

            if (size > s->elementary_transfer_size)

                size = s->elementary_transfer_size;

            ide_transfer_start(s, s->io_buffer + s->io_buffer_index,

                               size, ide_atapi_cmd_reply_end);

            s->packet_transfer_size -= size;

            s->elementary_transfer_size -= size;

            s->io_buffer_index += size;

        } else {

            

            s->nsector = (s->nsector & ~7) | ATAPI_INT_REASON_IO;

            byte_count_limit = s->lcyl | (s->hcyl << 8);

#ifdef DEBUG_IDE_ATAPI

            printf("byte_count_limit=%d\n", byte_count_limit);

#endif

            if (byte_count_limit == 0xffff)

                byte_count_limit--;

            size = s->packet_transfer_size;

            if (size > byte_count_limit) {

                

                if (byte_count_limit & 1)

                    byte_count_limit--;

                size = byte_count_limit;

            }

            s->lcyl = size;

            s->hcyl = size >> 8;

            s->elementary_transfer_size = size;

            

            if (s->lba != -1) {

                if (size > (s->cd_sector_size - s->io_buffer_index))

                    size = (s->cd_sector_size - s->io_buffer_index);

            }

            ide_transfer_start(s, s->io_buffer + s->io_buffer_index,

                               size, ide_atapi_cmd_reply_end);

            s->packet_transfer_size -= size;

            s->elementary_transfer_size -= size;

            s->io_buffer_index += size;

            ide_set_irq(s);

#ifdef DEBUG_IDE_ATAPI

            printf("status=0x%x\n", s->status);

#endif

        }

    }

}
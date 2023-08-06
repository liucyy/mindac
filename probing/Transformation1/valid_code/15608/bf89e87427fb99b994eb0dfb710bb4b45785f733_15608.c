int vhdx_parse_log(BlockDriverState *bs, BDRVVHDXState *s, bool *flushed,

                   Error **errp)

{

    int ret = 0;

    VHDXHeader *hdr;

    VHDXLogSequence logs = { 0 };



    hdr = s->headers[s->curr_header];



    *flushed = false;



    

    if (s->log.hdr == NULL) {

        s->log.hdr = qemu_blockalign(bs, sizeof(VHDXLogEntryHeader));

    }



    s->log.offset = hdr->log_offset;

    s->log.length = hdr->log_length;



    if (s->log.offset < VHDX_LOG_MIN_SIZE ||

        s->log.offset % VHDX_LOG_MIN_SIZE) {

        ret = -EINVAL;

        goto exit;

    }



    

    if (hdr->log_version != 0) {

        ret = -EINVAL;

        goto exit;

    }



    

    if (guid_eq(hdr->log_guid, zero_guid)) {

        goto exit;

    }



    if (hdr->log_length == 0) {

        goto exit;

    }



    if (hdr->log_length % VHDX_LOG_MIN_SIZE) {

        ret = -EINVAL;

        goto exit;

    }





    



    ret = vhdx_log_search(bs, s, &logs);

    if (ret < 0) {

        goto exit;

    }



    if (logs.valid) {

        if (bs->read_only) {

            ret = -EPERM;

            error_setg_errno(errp, EPERM,

                             "VHDX image file '%s' opened read-only, but "

                             "contains a log that needs to be replayed.  To "

                             "replay the log, execute:\n qemu-img check -r "

                             "all '%s'",

                             bs->filename, bs->filename);

            goto exit;

        }

        

        ret = vhdx_log_flush(bs, s, &logs);

        if (ret < 0) {

            goto exit;

        }

        *flushed = true;

    }





exit:

    return ret;

}
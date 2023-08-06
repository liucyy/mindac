static int nsv_read_header(AVFormatContext *s)

{

    NSVContext *nsv = s->priv_data;

    int i, err;



    av_dlog(s, "%s()\n", __FUNCTION__);

    av_dlog(s, "filename '%s'\n", s->filename);



    nsv->state = NSV_UNSYNC;

    nsv->ahead[0].data = nsv->ahead[1].data = NULL;



    for (i = 0; i < NSV_MAX_RESYNC_TRIES; i++) {

        if (nsv_resync(s) < 0)

            return -1;

        if (nsv->state == NSV_FOUND_NSVF)

            err = nsv_parse_NSVf_header(s);

            

        if (nsv->state == NSV_FOUND_NSVS) {

            err = nsv_parse_NSVs_header(s);

            break; 

        }

    }

    if (s->nb_streams < 1) 

        return -1;

    

    err = nsv_read_chunk(s, 1);



    av_dlog(s, "parsed header\n");

    return err;

}
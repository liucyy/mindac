int ff_h264_field_end(H264Context *h, H264SliceContext *sl, int in_setup)

{

    AVCodecContext *const avctx = h->avctx;

    int err = 0;

    h->mb_y = 0;



    if (!in_setup && !h->droppable)

        ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX,

                                  h->picture_structure == PICT_BOTTOM_FIELD);



    if (in_setup || !(avctx->active_thread_type & FF_THREAD_FRAME)) {

        if (!h->droppable) {

            err = ff_h264_execute_ref_pic_marking(h, h->mmco, h->mmco_index);

            h->prev_poc_msb = h->poc_msb;

            h->prev_poc_lsb = h->poc_lsb;

        }

        h->prev_frame_num_offset = h->frame_num_offset;

        h->prev_frame_num        = h->frame_num;

    }



    if (avctx->hwaccel) {

        if (avctx->hwaccel->end_frame(avctx) < 0)

            av_log(avctx, AV_LOG_ERROR,

                   "hardware accelerator failed to decode picture\n");

    }



#if CONFIG_ERROR_RESILIENCE

    

    if (!FIELD_PICTURE(h) && h->enable_er) {

        h264_set_erpic(&sl->er.cur_pic, h->cur_pic_ptr);

        h264_set_erpic(&sl->er.last_pic,

                       sl->ref_count[0] ? sl->ref_list[0][0].parent : NULL);

        h264_set_erpic(&sl->er.next_pic,

                       sl->ref_count[1] ? sl->ref_list[1][0].parent : NULL);

        ff_er_frame_end(&sl->er);

    }

#endif 



    emms_c();



    h->current_slice = 0;



    return err;

}
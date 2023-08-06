static int decode_frame_packing_arrangement(H264Context *h)

{

    h->sei_fpa.frame_packing_arrangement_id          = get_ue_golomb(&h->gb);

    h->sei_fpa.frame_packing_arrangement_cancel_flag = get_bits1(&h->gb);

    h->sei_frame_packing_present = !h->sei_fpa.frame_packing_arrangement_cancel_flag;



    if (h->sei_frame_packing_present) {

        h->sei_fpa.frame_packing_arrangement_type =

        h->frame_packing_arrangement_type = get_bits(&h->gb, 7);

        h->sei_fpa.quincunx_sampling_flag         =

        h->quincunx_subsampling           = get_bits1(&h->gb);

        h->sei_fpa.content_interpretation_type    =

        h->content_interpretation_type    = get_bits(&h->gb, 6);



        

        

        

        skip_bits(&h->gb, 6);



        if (!h->quincunx_subsampling && h->frame_packing_arrangement_type != 5)

            skip_bits(&h->gb, 16);      

        skip_bits(&h->gb, 8);           

        h->sei_fpa.frame_packing_arrangement_repetition_period = get_ue_golomb(&h->gb) ;

    }

    skip_bits1(&h->gb);                 



    if (h->avctx->debug & FF_DEBUG_PICT_INFO)

        av_log(h->avctx, AV_LOG_DEBUG, "SEI FPA %d %d %d %d %d %d\n",

                                       h->sei_fpa.frame_packing_arrangement_id,

                                       h->sei_fpa.frame_packing_arrangement_cancel_flag,

                                       h->sei_fpa.frame_packing_arrangement_type,

                                       h->sei_fpa.quincunx_sampling_flag,

                                       h->sei_fpa.content_interpretation_type,

                                       h->sei_fpa.frame_packing_arrangement_repetition_period);



    return 0;

}
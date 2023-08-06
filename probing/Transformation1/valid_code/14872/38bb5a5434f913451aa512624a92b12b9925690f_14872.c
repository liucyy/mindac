void mpeg4_encode_picture_header(MpegEncContext * s, int picture_number)

{

    int time_incr;

    int time_div, time_mod;



    if(s->pict_type==AV_PICTURE_TYPE_I){

        if(!(s->flags&CODEC_FLAG_GLOBAL_HEADER)){

            if(s->strict_std_compliance < FF_COMPLIANCE_VERY_STRICT) 

                mpeg4_encode_visual_object_header(s);

            if(s->strict_std_compliance < FF_COMPLIANCE_VERY_STRICT || picture_number==0) 

                mpeg4_encode_vol_header(s, 0, 0);

        }

        if(!(s->workaround_bugs & FF_BUG_MS))

            mpeg4_encode_gop_header(s);

    }



    s->partitioned_frame= s->data_partitioning && s->pict_type!=AV_PICTURE_TYPE_B;



    put_bits(&s->pb, 16, 0);                

    put_bits(&s->pb, 16, VOP_STARTCODE);    

    put_bits(&s->pb, 2, s->pict_type - 1);  



    assert(s->time>=0);

    time_div= s->time/s->avctx->time_base.den;

    time_mod= s->time%s->avctx->time_base.den;

    time_incr= time_div - s->last_time_base;

    assert(time_incr >= 0);

    while(time_incr--)

        put_bits(&s->pb, 1, 1);



    put_bits(&s->pb, 1, 0);



    put_bits(&s->pb, 1, 1);                             

    put_bits(&s->pb, s->time_increment_bits, time_mod); 

    put_bits(&s->pb, 1, 1);                             

    put_bits(&s->pb, 1, 1);                             

    if (    s->pict_type == AV_PICTURE_TYPE_P

        || (s->pict_type == AV_PICTURE_TYPE_S && s->vol_sprite_usage==GMC_SPRITE)) {

        put_bits(&s->pb, 1, s->no_rounding);    

    }

    put_bits(&s->pb, 3, 0);     

    if(!s->progressive_sequence){

         put_bits(&s->pb, 1, s->current_picture_ptr->top_field_first);

         put_bits(&s->pb, 1, s->alternate_scan);

    }

    



    put_bits(&s->pb, 5, s->qscale);



    if (s->pict_type != AV_PICTURE_TYPE_I)

        put_bits(&s->pb, 3, s->f_code); 

    if (s->pict_type == AV_PICTURE_TYPE_B)

        put_bits(&s->pb, 3, s->b_code); 

}
static int vc1_decode_frame(AVCodecContext *avctx,

                            void *data, int *data_size,

                            AVPacket *avpkt)

{

    const uint8_t *buf = avpkt->data;

    int buf_size = avpkt->size;

    VC1Context *v = avctx->priv_data;

    MpegEncContext *s = &v->s;

    AVFrame *pict = data;

    uint8_t *buf2 = NULL;

    const uint8_t *buf_start = buf;



    

    if (buf_size == 0) {

        

        if (s->low_delay==0 && s->next_picture_ptr) {

            *pict= *(AVFrame*)s->next_picture_ptr;

            s->next_picture_ptr= NULL;



            *data_size = sizeof(AVFrame);

        }



        return 0;

    }



    

    if(s->current_picture_ptr==NULL || s->current_picture_ptr->data[0]){

        int i= ff_find_unused_picture(s, 0);

        s->current_picture_ptr= &s->picture[i];

    }



    if (s->avctx->codec->capabilities&CODEC_CAP_HWACCEL_VDPAU){

        if (v->profile < PROFILE_ADVANCED)

            avctx->pix_fmt = PIX_FMT_VDPAU_WMV3;

        else

            avctx->pix_fmt = PIX_FMT_VDPAU_VC1;

    }



    

    if (avctx->codec_id == CODEC_ID_VC1) {

        int buf_size2 = 0;

        buf2 = av_mallocz(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);



        if(IS_MARKER(AV_RB32(buf))){ 

            const uint8_t *start, *end, *next;

            int size;



            next = buf;

            for(start = buf, end = buf + buf_size; next < end; start = next){

                next = find_next_marker(start + 4, end);

                size = next - start - 4;

                if(size <= 0) continue;

                switch(AV_RB32(start)){

                case VC1_CODE_FRAME:

                    if (avctx->hwaccel ||

                        s->avctx->codec->capabilities&CODEC_CAP_HWACCEL_VDPAU)

                        buf_start = start;

                    buf_size2 = vc1_unescape_buffer(start + 4, size, buf2);

                    break;

                case VC1_CODE_ENTRYPOINT: 

                    buf_size2 = vc1_unescape_buffer(start + 4, size, buf2);

                    init_get_bits(&s->gb, buf2, buf_size2*8);

                    vc1_decode_entry_point(avctx, v, &s->gb);

                    break;

                case VC1_CODE_SLICE:

                    av_log(avctx, AV_LOG_ERROR, "Sliced decoding is not implemented (yet)\n");

                    av_free(buf2);

                    return -1;

                }

            }

        }else if(v->interlace && ((buf[0] & 0xC0) == 0xC0)){ 

            const uint8_t *divider;



            divider = find_next_marker(buf, buf + buf_size);

            if((divider == (buf + buf_size)) || AV_RB32(divider) != VC1_CODE_FIELD){

                av_log(avctx, AV_LOG_ERROR, "Error in WVC1 interlaced frame\n");

                av_free(buf2);

                return -1;

            }



            buf_size2 = vc1_unescape_buffer(buf, divider - buf, buf2);

            

            av_free(buf2);return -1;

        }else{

            buf_size2 = vc1_unescape_buffer(buf, buf_size, buf2);

        }

        init_get_bits(&s->gb, buf2, buf_size2*8);

    } else

        init_get_bits(&s->gb, buf, buf_size*8);

    

    if(v->profile < PROFILE_ADVANCED) {

        if(vc1_parse_frame_header(v, &s->gb) == -1) {

            av_free(buf2);

            return -1;

        }

    } else {

        if(vc1_parse_frame_header_adv(v, &s->gb) == -1) {

            av_free(buf2);

            return -1;

        }

    }



    if(s->pict_type != FF_I_TYPE && !v->res_rtm_flag){

        av_free(buf2);

        return -1;

    }



    

    s->current_picture.pict_type= s->pict_type;

    s->current_picture.key_frame= s->pict_type == FF_I_TYPE;



    

    if(s->last_picture_ptr==NULL && (s->pict_type==FF_B_TYPE || s->dropable)){

        av_free(buf2);

        return -1;

    }

    

    if(avctx->hurry_up && s->pict_type==FF_B_TYPE) return -1;

    if(   (avctx->skip_frame >= AVDISCARD_NONREF && s->pict_type==FF_B_TYPE)

       || (avctx->skip_frame >= AVDISCARD_NONKEY && s->pict_type!=FF_I_TYPE)

       ||  avctx->skip_frame >= AVDISCARD_ALL) {

        av_free(buf2);

        return buf_size;

    }

    

    if(avctx->hurry_up>=5) {

        av_free(buf2);

        return -1;

    }



    if(s->next_p_frame_damaged){

        if(s->pict_type==FF_B_TYPE)

            return buf_size;

        else

            s->next_p_frame_damaged=0;

    }



    if(MPV_frame_start(s, avctx) < 0) {

        av_free(buf2);

        return -1;

    }



    s->me.qpel_put= s->dsp.put_qpel_pixels_tab;

    s->me.qpel_avg= s->dsp.avg_qpel_pixels_tab;



    if ((CONFIG_VC1_VDPAU_DECODER || CONFIG_WMV3_VDPAU_DECODER)

        &&s->avctx->codec->capabilities&CODEC_CAP_HWACCEL_VDPAU)

        ff_vdpau_vc1_decode_picture(s, buf_start, (buf + buf_size) - buf_start);

    else if (avctx->hwaccel) {

        if (avctx->hwaccel->start_frame(avctx, buf, buf_size) < 0)

            return -1;

        if (avctx->hwaccel->decode_slice(avctx, buf_start, (buf + buf_size) - buf_start) < 0)

            return -1;

        if (avctx->hwaccel->end_frame(avctx) < 0)

            return -1;

    } else {

        ff_er_frame_start(s);



        v->bits = buf_size * 8;

        vc1_decode_blocks(v);







        ff_er_frame_end(s);

    }



    MPV_frame_end(s);



assert(s->current_picture.pict_type == s->current_picture_ptr->pict_type);

assert(s->current_picture.pict_type == s->pict_type);

    if (s->pict_type == FF_B_TYPE || s->low_delay) {

        *pict= *(AVFrame*)s->current_picture_ptr;

    } else if (s->last_picture_ptr != NULL) {

        *pict= *(AVFrame*)s->last_picture_ptr;

    }



    if(s->last_picture_ptr || s->low_delay){

        *data_size = sizeof(AVFrame);

        ff_print_debug_info(s, pict);

    }



    av_free(buf2);

    return buf_size;

}
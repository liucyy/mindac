static int read_header(FFV1Context *f){

    uint8_t state[CONTEXT_SIZE];

    int i, j, context_count;

    RangeCoder * const c= &f->slice_context[0]->c;



    memset(state, 128, sizeof(state));



    if(f->version < 2){

        f->version= get_symbol(c, state, 0);

        f->ac= f->avctx->coder_type= get_symbol(c, state, 0);

        if(f->ac>1){

            for(i=1; i<256; i++){

                f->state_transition[i]= get_symbol(c, state, 1) + c->one_state[i];

            }

        }

        f->colorspace= get_symbol(c, state, 0); 

        if(f->version>0)

            f->avctx->bits_per_raw_sample= get_symbol(c, state, 0);

        f->chroma_planes= get_rac(c, state);

        f->chroma_h_shift= get_symbol(c, state, 0);

        f->chroma_v_shift= get_symbol(c, state, 0);

        f->transparency= get_rac(c, state);

        f->plane_count= 2 + f->transparency;

    }



    if(f->colorspace==0){

        if(!f->transparency && !f->chroma_planes){

            if (f->avctx->bits_per_raw_sample<=8)

                f->avctx->pix_fmt= PIX_FMT_GRAY8;

            else

                f->avctx->pix_fmt= PIX_FMT_GRAY16;

        }else if(f->avctx->bits_per_raw_sample<=8 && !f->transparency){

            switch(16*f->chroma_h_shift + f->chroma_v_shift){

            case 0x00: f->avctx->pix_fmt= PIX_FMT_YUV444P; break;

            case 0x01: f->avctx->pix_fmt= PIX_FMT_YUV440P; break;

            case 0x10: f->avctx->pix_fmt= PIX_FMT_YUV422P; break;

            case 0x11: f->avctx->pix_fmt= PIX_FMT_YUV420P; break;

            case 0x20: f->avctx->pix_fmt= PIX_FMT_YUV411P; break;

            case 0x22: f->avctx->pix_fmt= PIX_FMT_YUV410P; break;

            default:

                av_log(f->avctx, AV_LOG_ERROR, "format not supported\n");

                return -1;

            }

        }else if(f->avctx->bits_per_raw_sample<=8 && f->transparency){

            switch(16*f->chroma_h_shift + f->chroma_v_shift){

            case 0x00: f->avctx->pix_fmt= PIX_FMT_YUVA444P; break;

            case 0x11: f->avctx->pix_fmt= PIX_FMT_YUVA420P; break;

            default:

                av_log(f->avctx, AV_LOG_ERROR, "format not supported\n");

                return -1;

            }

        }else if(f->avctx->bits_per_raw_sample==9) {

            f->packed_at_lsb=1;

            switch(16*f->chroma_h_shift + f->chroma_v_shift){

            case 0x00: f->avctx->pix_fmt= PIX_FMT_YUV444P9; break;

            case 0x10: f->avctx->pix_fmt= PIX_FMT_YUV422P9; break;

            case 0x11: f->avctx->pix_fmt= PIX_FMT_YUV420P9; break;

            default:

                av_log(f->avctx, AV_LOG_ERROR, "format not supported\n");

                return -1;

            }

        }else if(f->avctx->bits_per_raw_sample==10) {

            f->packed_at_lsb=1;

            switch(16*f->chroma_h_shift + f->chroma_v_shift){

            case 0x00: f->avctx->pix_fmt= PIX_FMT_YUV444P10; break;

            case 0x10: f->avctx->pix_fmt= PIX_FMT_YUV422P10; break;

            case 0x11: f->avctx->pix_fmt= PIX_FMT_YUV420P10; break;

            default:

                av_log(f->avctx, AV_LOG_ERROR, "format not supported\n");

                return -1;

            }

        }else {

            switch(16*f->chroma_h_shift + f->chroma_v_shift){

            case 0x00: f->avctx->pix_fmt= PIX_FMT_YUV444P16; break;

            case 0x10: f->avctx->pix_fmt= PIX_FMT_YUV422P16; break;

            case 0x11: f->avctx->pix_fmt= PIX_FMT_YUV420P16; break;

            default:

                av_log(f->avctx, AV_LOG_ERROR, "format not supported\n");

                return -1;

            }

        }

    }else if(f->colorspace==1){

        if(f->chroma_h_shift || f->chroma_v_shift){

            av_log(f->avctx, AV_LOG_ERROR, "chroma subsampling not supported in this colorspace\n");

            return -1;

        }

        if(f->transparency) f->avctx->pix_fmt= PIX_FMT_RGB32;

        else                f->avctx->pix_fmt= PIX_FMT_0RGB32;

    }else{

        av_log(f->avctx, AV_LOG_ERROR, "colorspace not supported\n");

        return -1;

    }





    if(f->version < 2){

        context_count= read_quant_tables(c, f->quant_table);

        if(context_count < 0){

                av_log(f->avctx, AV_LOG_ERROR, "read_quant_table error\n");

                return -1;

        }

    }else{

        f->slice_count= get_symbol(c, state, 0);

        if(f->slice_count > (unsigned)MAX_SLICES)

            return -1;

    }



    for(j=0; j<f->slice_count; j++){

        FFV1Context *fs= f->slice_context[j];

        fs->ac= f->ac;

        fs->packed_at_lsb= f->packed_at_lsb;



        if(f->version >= 2){

            fs->slice_x     = get_symbol(c, state, 0)   *f->width ;

            fs->slice_y     = get_symbol(c, state, 0)   *f->height;

            fs->slice_width =(get_symbol(c, state, 0)+1)*f->width  + fs->slice_x;

            fs->slice_height=(get_symbol(c, state, 0)+1)*f->height + fs->slice_y;



            fs->slice_x /= f->num_h_slices;

            fs->slice_y /= f->num_v_slices;

            fs->slice_width  = fs->slice_width /f->num_h_slices - fs->slice_x;

            fs->slice_height = fs->slice_height/f->num_v_slices - fs->slice_y;

            if((unsigned)fs->slice_width > f->width || (unsigned)fs->slice_height > f->height)

                return -1;

            if(    (unsigned)fs->slice_x + (uint64_t)fs->slice_width  > f->width

                || (unsigned)fs->slice_y + (uint64_t)fs->slice_height > f->height)

                return -1;

        }



        for(i=0; i<f->plane_count; i++){

            PlaneContext * const p= &fs->plane[i];



            if(f->version >= 2){

                int idx=get_symbol(c, state, 0);

                if(idx > (unsigned)f->quant_table_count){

                    av_log(f->avctx, AV_LOG_ERROR, "quant_table_index out of range\n");

                    return -1;

                }

                p->quant_table_index= idx;

                memcpy(p->quant_table, f->quant_tables[idx], sizeof(p->quant_table));

                context_count= f->context_count[idx];

            }else{

                memcpy(p->quant_table, f->quant_table, sizeof(p->quant_table));

            }



            if(p->context_count < context_count){

                av_freep(&p->state);

                av_freep(&p->vlc_state);

            }

            p->context_count= context_count;

        }

    }



    return 0;

}
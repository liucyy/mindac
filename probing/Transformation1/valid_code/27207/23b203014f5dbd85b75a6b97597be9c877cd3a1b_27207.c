static int decode_pic_hdr(IVI45DecContext *ctx, AVCodecContext *avctx)

{

    int             pic_size_indx, i, p;

    IVIPicConfig    pic_conf;



    if (get_bits(&ctx->gb, 18) != 0x3FFF8) {

        av_log(avctx, AV_LOG_ERROR, "Invalid picture start code!\n");

        return AVERROR_INVALIDDATA;

    }



    ctx->prev_frame_type = ctx->frame_type;

    ctx->frame_type      = get_bits(&ctx->gb, 3);

    if (ctx->frame_type == 7) {

        av_log(avctx, AV_LOG_ERROR, "Invalid frame type: %d\n", ctx->frame_type);

        return AVERROR_INVALIDDATA;

    }



#if IVI4_STREAM_ANALYSER

    if (ctx->frame_type == FRAMETYPE_BIDIR)

        ctx->has_b_frames = 1;

#endif



    ctx->transp_status = get_bits1(&ctx->gb);

#if IVI4_STREAM_ANALYSER

    if (ctx->transp_status) {

        ctx->has_transp = 1;

    }

#endif



    

    if (get_bits1(&ctx->gb)) {

        av_log(avctx, AV_LOG_ERROR, "Sync bit is set!\n");

        return AVERROR_INVALIDDATA;

    }



    ctx->data_size = get_bits1(&ctx->gb) ? get_bits(&ctx->gb, 24) : 0;



    

    if (ctx->frame_type >= FRAMETYPE_NULL_FIRST) {

        av_dlog(avctx, "Null frame encountered!\n");

        return 0;

    }



    

    

    

    if (get_bits1(&ctx->gb)) {

        skip_bits_long(&ctx->gb, 32);

        av_dlog(avctx, "Password-protected clip!\n");

    }



    pic_size_indx = get_bits(&ctx->gb, 3);

    if (pic_size_indx == IVI4_PIC_SIZE_ESC) {

        pic_conf.pic_height = get_bits(&ctx->gb, 16);

        pic_conf.pic_width  = get_bits(&ctx->gb, 16);

    } else {

        pic_conf.pic_height = ivi4_common_pic_sizes[pic_size_indx * 2 + 1];

        pic_conf.pic_width  = ivi4_common_pic_sizes[pic_size_indx * 2    ];

    }



    

    if (get_bits1(&ctx->gb)) {

        pic_conf.tile_height = scale_tile_size(pic_conf.pic_height, get_bits(&ctx->gb, 4));

        pic_conf.tile_width  = scale_tile_size(pic_conf.pic_width,  get_bits(&ctx->gb, 4));

#if IVI4_STREAM_ANALYSER

        ctx->uses_tiling = 1;

#endif

    } else {

        pic_conf.tile_height = pic_conf.pic_height;

        pic_conf.tile_width  = pic_conf.pic_width;

    }



    

    if (get_bits(&ctx->gb, 2)) {

        av_log(avctx, AV_LOG_ERROR, "Only YVU9 picture format is supported!\n");

        return AVERROR_INVALIDDATA;

    }

    pic_conf.chroma_height = (pic_conf.pic_height + 3) >> 2;

    pic_conf.chroma_width  = (pic_conf.pic_width  + 3) >> 2;



    

    pic_conf.luma_bands = decode_plane_subdivision(&ctx->gb);


    if (pic_conf.luma_bands)

        pic_conf.chroma_bands = decode_plane_subdivision(&ctx->gb);

    ctx->is_scalable = pic_conf.luma_bands != 1 || pic_conf.chroma_bands != 1;

    if (ctx->is_scalable && (pic_conf.luma_bands != 4 || pic_conf.chroma_bands != 1)) {

        av_log(avctx, AV_LOG_ERROR, "Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d\n",

               pic_conf.luma_bands, pic_conf.chroma_bands);

        return AVERROR_INVALIDDATA;

    }



    

    if (ivi_pic_config_cmp(&pic_conf, &ctx->pic_conf)) {

        if (ff_ivi_init_planes(ctx->planes, &pic_conf)) {

            av_log(avctx, AV_LOG_ERROR, "Couldn't reallocate color planes!\n");

            return AVERROR(ENOMEM);

        }



        ctx->pic_conf = pic_conf;



        

        for (p = 0; p <= 2; p++) {

            for (i = 0; i < (!p ? pic_conf.luma_bands : pic_conf.chroma_bands); i++) {

                ctx->planes[p].bands[i].mb_size  = !p ? (!ctx->is_scalable ? 16 : 8) : 4;

                ctx->planes[p].bands[i].blk_size = !p ? 8 : 4;

            }

        }



        if (ff_ivi_init_tiles(ctx->planes, ctx->pic_conf.tile_width,

                              ctx->pic_conf.tile_height)) {

            av_log(avctx, AV_LOG_ERROR,

                   "Couldn't reallocate internal structures!\n");

            return AVERROR(ENOMEM);

        }

    }



    ctx->frame_num = get_bits1(&ctx->gb) ? get_bits(&ctx->gb, 20) : 0;



    

    if (get_bits1(&ctx->gb))

        skip_bits(&ctx->gb, 8);



    

    if (ff_ivi_dec_huff_desc(&ctx->gb, get_bits1(&ctx->gb), IVI_MB_HUFF,  &ctx->mb_vlc,  avctx) ||

        ff_ivi_dec_huff_desc(&ctx->gb, get_bits1(&ctx->gb), IVI_BLK_HUFF, &ctx->blk_vlc, avctx))

        return AVERROR_INVALIDDATA;



    ctx->rvmap_sel = get_bits1(&ctx->gb) ? get_bits(&ctx->gb, 3) : 8;



    ctx->in_imf = get_bits1(&ctx->gb);

    ctx->in_q   = get_bits1(&ctx->gb);



    ctx->pic_glob_quant = get_bits(&ctx->gb, 5);



    

    ctx->unknown1 = get_bits1(&ctx->gb) ? get_bits(&ctx->gb, 3) : 0;



    ctx->checksum = get_bits1(&ctx->gb) ? get_bits(&ctx->gb, 16) : 0;



    

    while (get_bits1(&ctx->gb)) {

        av_dlog(avctx, "Pic hdr extension encountered!\n");

        skip_bits(&ctx->gb, 8);

    }



    if (get_bits1(&ctx->gb)) {

        av_log(avctx, AV_LOG_ERROR, "Bad blocks bits encountered!\n");

    }



    align_get_bits(&ctx->gb);



    return 0;

}
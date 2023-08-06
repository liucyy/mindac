static av_cold int oggvorbis_init_encoder(vorbis_info *vi,

                                          AVCodecContext *avctx)

{

    OggVorbisContext *s = avctx->priv_data;

    double cfreq;

    int ret;



    if (avctx->flags & CODEC_FLAG_QSCALE) {

        

        float q = avctx->global_quality / (float)FF_QP2LAMBDA;

        if ((ret = vorbis_encode_setup_vbr(vi, avctx->channels,

                                           avctx->sample_rate,

                                           q / 10.0)))

            goto error;

    } else {

        int minrate = avctx->rc_min_rate > 0 ? avctx->rc_min_rate : -1;

        int maxrate = avctx->rc_min_rate > 0 ? avctx->rc_max_rate : -1;



        

        if ((ret = vorbis_encode_setup_managed(vi, avctx->channels,

                                               avctx->sample_rate, minrate,

                                               avctx->bit_rate, maxrate)))

            goto error;



        

        if (minrate == -1 && maxrate == -1)

            if ((ret = vorbis_encode_ctl(vi, OV_ECTL_RATEMANAGE2_SET, NULL)))

                goto error;

    }



    

    if (avctx->cutoff > 0) {

        cfreq = avctx->cutoff / 1000.0;

        if ((ret = vorbis_encode_ctl(vi, OV_ECTL_LOWPASS_SET, &cfreq)))

            goto error;

    }



    

    if (s->iblock) {

        if ((ret = vorbis_encode_ctl(vi, OV_ECTL_IBLOCK_SET, &s->iblock)))

            goto error;

    }



    if ((ret = vorbis_encode_setup_init(vi)))

        goto error;



    return 0;

error:

    return vorbis_error_to_averror(ret);

}
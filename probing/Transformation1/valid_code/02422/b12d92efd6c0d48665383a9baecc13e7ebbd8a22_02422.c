static av_cold int decode_init(AVCodecContext * avctx)

{

    KmvcContext *const c = avctx->priv_data;

    int i;



    c->avctx = avctx;



    if (avctx->width > 320 || avctx->height > 200) {

        av_log(avctx, AV_LOG_ERROR, "KMVC supports frames <= 320x200\n");

        return -1;

    }



    c->frm0 = av_mallocz(320 * 200);

    c->frm1 = av_mallocz(320 * 200);

    c->cur = c->frm0;

    c->prev = c->frm1;



    for (i = 0; i < 256; i++) {

        c->pal[i] = 0xFF << 24 | i * 0x10101;

    }



    if (avctx->extradata_size < 12) {

        av_log(avctx, AV_LOG_WARNING,

               "Extradata missing, decoding may not work properly...\n");

        c->palsize = 127;

    } else {

        c->palsize = AV_RL16(avctx->extradata + 10);

        if (c->palsize >= (unsigned)MAX_PALSIZE) {

            c->palsize = 127;

            av_log(avctx, AV_LOG_ERROR, "KMVC palette too large\n");

            return AVERROR_INVALIDDATA;

        }

    }



    if (avctx->extradata_size == 1036) {        

        uint8_t *src = avctx->extradata + 12;

        for (i = 0; i < 256; i++) {

            c->pal[i] = AV_RL32(src);

            src += 4;

        }

        c->setpal = 1;

    }



    avcodec_get_frame_defaults(&c->pic);

    avctx->pix_fmt = AV_PIX_FMT_PAL8;



    return 0;

}
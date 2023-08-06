static av_cold int Faac_encode_init(AVCodecContext *avctx)

{

    FaacAudioContext *s = avctx->priv_data;

    faacEncConfigurationPtr faac_cfg;

    unsigned long samples_input, max_bytes_output;



    

    if (avctx->channels < 1 || avctx->channels > 6)

        return -1;



    s->faac_handle = faacEncOpen(avctx->sample_rate,

                                 avctx->channels,

                                 &samples_input, &max_bytes_output);



    

    faac_cfg = faacEncGetCurrentConfiguration(s->faac_handle);

    if (faac_cfg->version != FAAC_CFG_VERSION) {

        av_log(avctx, AV_LOG_ERROR, "wrong libfaac version (compiled for: %d, using %d)\n", FAAC_CFG_VERSION, faac_cfg->version);

        faacEncClose(s->faac_handle);

        return -1;

    }



    

    switch(avctx->profile) {

        case FF_PROFILE_AAC_MAIN:

            faac_cfg->aacObjectType = MAIN;

            break;

        case FF_PROFILE_UNKNOWN:

        case FF_PROFILE_AAC_LOW:

            faac_cfg->aacObjectType = LOW;

            break;

        case FF_PROFILE_AAC_SSR:

            faac_cfg->aacObjectType = SSR;

            break;

        case FF_PROFILE_AAC_LTP:

            faac_cfg->aacObjectType = LTP;

            break;

        default:

            av_log(avctx, AV_LOG_ERROR, "invalid AAC profile\n");

            faacEncClose(s->faac_handle);

            return -1;

    }

    faac_cfg->mpegVersion = MPEG4;

    faac_cfg->useTns = 0;

    faac_cfg->allowMidside = 1;

    faac_cfg->bitRate = avctx->bit_rate / avctx->channels;

    faac_cfg->bandWidth = avctx->cutoff;

    if(avctx->flags & CODEC_FLAG_QSCALE) {

        faac_cfg->bitRate = 0;

        faac_cfg->quantqual = avctx->global_quality / FF_QP2LAMBDA;

    }

    faac_cfg->outputFormat = 1;

    faac_cfg->inputFormat = FAAC_INPUT_16BIT;



    avctx->frame_size = samples_input / avctx->channels;



    avctx->coded_frame= avcodec_alloc_frame();

    avctx->coded_frame->key_frame= 1;



    

    avctx->extradata_size = 0;

    if (avctx->flags & CODEC_FLAG_GLOBAL_HEADER) {



        unsigned char *buffer = NULL;

        unsigned long decoder_specific_info_size;



        if (!faacEncGetDecoderSpecificInfo(s->faac_handle, &buffer,

                                           &decoder_specific_info_size)) {

            avctx->extradata = av_malloc(decoder_specific_info_size + FF_INPUT_BUFFER_PADDING_SIZE);

            avctx->extradata_size = decoder_specific_info_size;

            memcpy(avctx->extradata, buffer, avctx->extradata_size);

            faac_cfg->outputFormat = 0;

        }

#undef free

        free(buffer);

#define free please_use_av_free

    }



    if (!faacEncSetConfiguration(s->faac_handle, faac_cfg)) {

        av_log(avctx, AV_LOG_ERROR, "libfaac doesn't support this output format!\n");

        return -1;

    }



    return 0;

}
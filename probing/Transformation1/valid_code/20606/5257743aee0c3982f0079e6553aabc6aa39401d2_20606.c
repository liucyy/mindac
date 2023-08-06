static int ws_snd_decode_frame(AVCodecContext *avctx, void *data,

                               int *got_frame_ptr, AVPacket *avpkt)

{

    WSSndContext *s = avctx->priv_data;

    const uint8_t *buf = avpkt->data;

    int buf_size       = avpkt->size;



    int in_size, out_size, ret;

    int sample = 128;

    uint8_t *samples;

    uint8_t *samples_end;



    if (!buf_size)

        return 0;



    if (buf_size < 4) {

        av_log(avctx, AV_LOG_ERROR, "packet is too small\n");

        return AVERROR(EINVAL);

    }



    out_size = AV_RL16(&buf[0]);

    in_size  = AV_RL16(&buf[2]);

    buf += 4;



    if (in_size > buf_size) {

        av_log(avctx, AV_LOG_ERROR, "Frame data is larger than input buffer\n");

        return -1;

    }



    

    s->frame.nb_samples = out_size;

    if ((ret = avctx->get_buffer(avctx, &s->frame)) < 0) {

        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");

        return ret;

    }

    samples     = s->frame.data[0];

    samples_end = samples + out_size;



    if (in_size == out_size) {

        memcpy(samples, buf, out_size);

        *got_frame_ptr   = 1;

        *(AVFrame *)data = s->frame;

        return buf_size;

    }



    while (samples < samples_end && buf - avpkt->data < buf_size) {

        int code, smp, size;

        uint8_t count;

        code  = *buf >> 6;

        count = *buf & 0x3F;

        buf++;



        

        switch (code) {

        case 0:  smp = 4;                              break;

        case 1:  smp = 2;                              break;

        case 2:  smp = (count & 0x20) ? 1 : count + 1; break;

        default: smp = count + 1;                      break;

        }

        if (samples_end - samples < smp)

            break;



        

        size = ((code == 2 && (count & 0x20)) || code == 3) ? 0 : count + 1;

        if ((buf - avpkt->data) + size > buf_size)

            break;



        switch (code) {

        case 0: 

            for (count++; count > 0; count--) {

                code = *buf++;

                sample += ( code       & 0x3) - 2;

                sample = av_clip_uint8(sample);

                *samples++ = sample;

                sample += ((code >> 2) & 0x3) - 2;

                sample = av_clip_uint8(sample);

                *samples++ = sample;

                sample += ((code >> 4) & 0x3) - 2;

                sample = av_clip_uint8(sample);

                *samples++ = sample;

                sample +=  (code >> 6)        - 2;

                sample = av_clip_uint8(sample);

                *samples++ = sample;

            }

            break;

        case 1: 

            for (count++; count > 0; count--) {

                code = *buf++;

                sample += ws_adpcm_4bit[code & 0xF];

                sample = av_clip_uint8(sample);

                *samples++ = sample;

                sample += ws_adpcm_4bit[code >> 4];

                sample = av_clip_uint8(sample);

                *samples++ = sample;

            }

            break;

        case 2: 

            if (count & 0x20) { 

                int8_t t;

                t = count;

                t <<= 3;

                sample += t >> 3;

                sample = av_clip_uint8(sample);

                *samples++ = sample;

            } else { 

                memcpy(samples, buf, smp);

                samples += smp;

                buf     += smp;

                sample = buf[-1];

            }

            break;

        default: 

            memset(samples, sample, smp);

            samples += smp;

        }

    }



    s->frame.nb_samples = samples - s->frame.data[0];

    *got_frame_ptr   = 1;

    *(AVFrame *)data = s->frame;



    return buf_size;

}
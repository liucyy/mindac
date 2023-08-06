static int gxf_write_header(AVFormatContext *s)

{

    AVIOContext *pb = s->pb;

    GXFContext *gxf = s->priv_data;

    GXFStreamContext *vsc = NULL;

    uint8_t tracks[255] = {0};

    int i, media_info = 0;



    if (!pb->seekable) {

        av_log(s, AV_LOG_ERROR, "gxf muxer does not support streamed output, patch welcome");

        return -1;

    }



    gxf->flags |= 0x00080000; 

    for (i = 0; i < s->nb_streams; ++i) {

        AVStream *st = s->streams[i];

        GXFStreamContext *sc = av_mallocz(sizeof(*sc));

        if (!sc)

            return AVERROR(ENOMEM);

        st->priv_data = sc;



        sc->media_type = ff_codec_get_tag(gxf_media_types, st->codecpar->codec_id);

        if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {

            if (st->codecpar->codec_id != AV_CODEC_ID_PCM_S16LE) {

                av_log(s, AV_LOG_ERROR, "only 16 BIT PCM LE allowed for now\n");

                return -1;

            }

            if (st->codecpar->sample_rate != 48000) {

                av_log(s, AV_LOG_ERROR, "only 48000hz sampling rate is allowed\n");

                return -1;

            }

            if (st->codecpar->channels != 1) {

                av_log(s, AV_LOG_ERROR, "only mono tracks are allowed\n");

                return -1;

            }

            sc->track_type = 2;

            sc->sample_rate = st->codecpar->sample_rate;

            avpriv_set_pts_info(st, 64, 1, sc->sample_rate);

            sc->sample_size = 16;

            sc->frame_rate_index = -2;

            sc->lines_index = -2;

            sc->fields = -2;

            gxf->audio_tracks++;

            gxf->flags |= 0x04000000; 

            media_info = 'A';

        } else if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

            if (i != 0) {

                av_log(s, AV_LOG_ERROR, "video stream must be the first track\n");

                return -1;

            }

            

            if (st->codecpar->height == 480 || st->codecpar->height == 512) { 

                sc->frame_rate_index = 5;

                sc->sample_rate = 60;

                gxf->flags |= 0x00000080;

                gxf->time_base = (AVRational){ 1001, 60000 };

            } else if (st->codecpar->height == 576 || st->codecpar->height == 608) { 

                sc->frame_rate_index = 6;

                sc->media_type++;

                sc->sample_rate = 50;

                gxf->flags |= 0x00000040;

                gxf->time_base = (AVRational){ 1, 50 };

            } else {

                av_log(s, AV_LOG_ERROR, "unsupported video resolution, "

                       "gxf muxer only accepts PAL or NTSC resolutions currently\n");

                return -1;

            }

            avpriv_set_pts_info(st, 64, gxf->time_base.num, gxf->time_base.den);

            if (gxf_find_lines_index(st) < 0)

                sc->lines_index = -1;

            sc->sample_size = st->codecpar->bit_rate;

            sc->fields = 2; 



            vsc = sc;



            switch (st->codecpar->codec_id) {

            case AV_CODEC_ID_MJPEG:

                sc->track_type = 1;

                gxf->flags |= 0x00004000;

                media_info = 'J';

                break;

            case AV_CODEC_ID_MPEG1VIDEO:

                sc->track_type = 9;

                gxf->mpeg_tracks++;

                media_info = 'L';

                break;

            case AV_CODEC_ID_MPEG2VIDEO:

                sc->first_gop_closed = -1;

                sc->track_type = 4;

                gxf->mpeg_tracks++;

                gxf->flags |= 0x00008000;

                media_info = 'M';

                break;

            case AV_CODEC_ID_DVVIDEO:

                if (st->codecpar->format == AV_PIX_FMT_YUV422P) {

                    sc->media_type += 2;

                    sc->track_type = 6;

                    gxf->flags |= 0x00002000;

                    media_info = 'E';

                } else {

                    sc->track_type = 5;

                    gxf->flags |= 0x00001000;

                    media_info = 'D';

                }

                break;

            default:

                av_log(s, AV_LOG_ERROR, "video codec not supported\n");

                return -1;

            }

        }

        

        sc->media_info = media_info<<8 | ('0'+tracks[media_info]++);

        sc->order = s->nb_streams - st->index;

    }



    if (ff_audio_interleave_init(s, GXF_samples_per_frame, (AVRational){ 1, 48000 }) < 0)

        return -1;



    gxf_init_timecode_track(&gxf->timecode_track, vsc);

    gxf->flags |= 0x200000; 



    gxf_write_map_packet(s, 0);

    gxf_write_flt_packet(s);

    gxf_write_umf_packet(s);



    gxf->packet_count = 3;



    avio_flush(pb);

    return 0;

}
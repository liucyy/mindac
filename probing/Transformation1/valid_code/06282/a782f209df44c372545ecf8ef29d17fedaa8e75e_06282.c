static int http_prepare_data(HTTPContext *c)

{

    int i, len, ret;

    AVFormatContext *ctx;



    switch(c->state) {

    case HTTPSTATE_SEND_DATA_HEADER:

        memset(&c->fmt_ctx, 0, sizeof(c->fmt_ctx));

        pstrcpy(c->fmt_ctx.author, sizeof(c->fmt_ctx.author), 

                c->stream->author);

        pstrcpy(c->fmt_ctx.comment, sizeof(c->fmt_ctx.comment), 

                c->stream->comment);

        pstrcpy(c->fmt_ctx.copyright, sizeof(c->fmt_ctx.copyright), 

                c->stream->copyright);

        pstrcpy(c->fmt_ctx.title, sizeof(c->fmt_ctx.title), 

                c->stream->title);



        

        c->fmt_ctx.oformat = c->stream->fmt;

        c->fmt_ctx.nb_streams = c->stream->nb_streams;

        for(i=0;i<c->fmt_ctx.nb_streams;i++) {

            AVStream *st;

            st = av_mallocz(sizeof(AVStream));

            c->fmt_ctx.streams[i] = st;

            

            if (!c->stream->feed || 

                c->stream->feed == c->stream)

                memcpy(st, c->stream->streams[i], sizeof(AVStream));

            else

                memcpy(st, c->stream->feed->streams[c->stream->feed_streams[i]],

                           sizeof(AVStream));

            st->codec.frame_number = 0; 

        }

        c->got_key_frame = 0;



        

        if (url_open_dyn_buf(&c->fmt_ctx.pb) < 0) {

            

            return -1;

        }

        c->fmt_ctx.pb.is_streamed = 1;



        av_write_header(&c->fmt_ctx);



        len = url_close_dyn_buf(&c->fmt_ctx.pb, &c->pb_buffer);

        c->buffer_ptr = c->pb_buffer;

        c->buffer_end = c->pb_buffer + len;



        c->state = HTTPSTATE_SEND_DATA;

        c->last_packet_sent = 0;

        break;

    case HTTPSTATE_SEND_DATA:

        

        {

            AVPacket pkt;

            

            

            if (c->stream->feed) {

                ffm_set_write_index(c->fmt_in, 

                                    c->stream->feed->feed_write_index,

                                    c->stream->feed->feed_size);

            }



            if (c->stream->max_time && 

                c->stream->max_time + c->start_time - cur_time < 0) {

                

                c->state = HTTPSTATE_SEND_DATA_TRAILER;

            } else {

                if (c->is_packetized) {

                    if (compute_send_delay(c) > 0) {

                        c->state = HTTPSTATE_WAIT;

                        return 1; 

                    }

                }

                if (av_read_frame(c->fmt_in, &pkt) < 0) {

                    if (c->stream->feed && c->stream->feed->feed_opened) {

                        

                        c->state = HTTPSTATE_WAIT_FEED;

                        return 1; 

                    } else {

                        

                        c->state = HTTPSTATE_SEND_DATA_TRAILER;

                    }

                } else {

                    

                    if (c->first_pts == AV_NOPTS_VALUE)

                        c->first_pts = pkt.pts;

                    

                    

                    if (c->stream->feed) {

                        

                        if (c->switch_pending) {

                            c->switch_pending = 0;

                            for(i=0;i<c->stream->nb_streams;i++) {

                                if (c->switch_feed_streams[i] == pkt.stream_index) {

                                    if (pkt.flags & PKT_FLAG_KEY) {

                                        do_switch_stream(c, i);

                                    }

                                }

                                if (c->switch_feed_streams[i] >= 0) {

                                    c->switch_pending = 1;

                                }

                            }

                        }

                        for(i=0;i<c->stream->nb_streams;i++) {

                            if (c->feed_streams[i] == pkt.stream_index) {

                                pkt.stream_index = i;

                                if (pkt.flags & PKT_FLAG_KEY) {

                                    c->got_key_frame |= 1 << i;

                                }

                                

                                if (!c->stream->send_on_key || 

                                    ((c->got_key_frame + 1) >> c->stream->nb_streams)) {

                                    goto send_it;

                                }

                            }

                        }

                    } else {

                        AVCodecContext *codec;

                        

                    send_it:

                        

                        if (c->is_packetized) {

                            c->packet_stream_index = pkt.stream_index;

                            ctx = c->rtp_ctx[c->packet_stream_index];

                            codec = &ctx->streams[0]->codec;

                        } else {

                            ctx = &c->fmt_ctx;

                            

                            codec = &ctx->streams[pkt.stream_index]->codec;

                        }

                        

                        codec->key_frame = ((pkt.flags & PKT_FLAG_KEY) != 0);

                        

#ifdef PJSG

                        if (codec->codec_type == CODEC_TYPE_AUDIO) {

                            codec->frame_size = (codec->sample_rate * pkt.duration + 500000) / 1000000;

                            

                        }

#endif

                        

                        if (c->is_packetized) {

                            ret = url_open_dyn_packet_buf(&ctx->pb, 

                                                          url_get_max_packet_size(c->rtp_handles[c->packet_stream_index]));

                            c->packet_byte_count = 0;

                            c->packet_start_time_us = av_gettime();

                        } else {

                            ret = url_open_dyn_buf(&ctx->pb);

                        }

                        if (ret < 0) {

                            

                            return -1;

                        }

                        if (av_write_packet(ctx, &pkt, pkt.pts)) {

                            c->state = HTTPSTATE_SEND_DATA_TRAILER;

                        }

                        

                        len = url_close_dyn_buf(&ctx->pb, &c->pb_buffer);

                        c->buffer_ptr = c->pb_buffer;

                        c->buffer_end = c->pb_buffer + len;

                        

                        codec->frame_number++;

                    }

#ifndef AV_READ_FRAME

                    av_free_packet(&pkt);

#endif

                }

            }

        }

        break;

    default:

    case HTTPSTATE_SEND_DATA_TRAILER:

        

        if (c->last_packet_sent || c->is_packetized)

            return -1;

        ctx = &c->fmt_ctx;

        

        if (url_open_dyn_buf(&ctx->pb) < 0) {

            

            return -1;

        }

        av_write_trailer(ctx);

        len = url_close_dyn_buf(&ctx->pb, &c->pb_buffer);

        c->buffer_ptr = c->pb_buffer;

        c->buffer_end = c->pb_buffer + len;



        c->last_packet_sent = 1;

        break;

    }

    return 0;

}
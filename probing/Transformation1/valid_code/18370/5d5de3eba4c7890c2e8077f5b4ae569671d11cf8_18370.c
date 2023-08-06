int ff_v4l2_context_dequeue_frame(V4L2Context* ctx, AVFrame* frame)

{

    V4L2Buffer* avbuf = NULL;



    

    avbuf = v4l2_dequeue_v4l2buf(ctx, ctx_to_m2mctx(ctx)->draining ? 200 : -1);

    if (!avbuf) {

        if (ctx->done)

            return AVERROR_EOF;



        return AVERROR(EAGAIN);

    }



    return ff_v4l2_buffer_buf_to_avframe(frame, avbuf);

}
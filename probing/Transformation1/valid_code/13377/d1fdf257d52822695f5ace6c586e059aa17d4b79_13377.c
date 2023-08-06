static void nbd_co_receive_reply(NBDClientSession *s,

                                 NBDRequest *request,

                                 NBDReply *reply,

                                 QEMUIOVector *qiov)

{

    int ret;



    

    qemu_coroutine_yield();

    *reply = s->reply;

    if (reply->handle != request->handle ||

        !s->ioc) {

        reply->error = EIO;

    } else {

        if (qiov && reply->error == 0) {

            ret = nbd_wr_syncv(s->ioc, qiov->iov, qiov->niov, request->len,

                               true, NULL);

            if (ret != request->len) {

                reply->error = EIO;

            }

        }



        

        s->reply.handle = 0;

    }

}
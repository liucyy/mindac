void aio_set_event_notifier(AioContext *ctx,

                            EventNotifier *e,

                            EventNotifierHandler *io_notify,

                            AioFlushEventNotifierHandler *io_flush)

{

    AioHandler *node;



    QLIST_FOREACH(node, &ctx->aio_handlers, node) {

        if (node->e == e && !node->deleted) {

            break;

        }

    }



    

    if (!io_notify) {

        if (node) {

            g_source_remove_poll(&ctx->source, &node->pfd);



            

            if (ctx->walking_handlers) {

                node->deleted = 1;

                node->pfd.revents = 0;

            } else {

                

                QLIST_REMOVE(node, node);

                g_free(node);

            }

        }

    } else {

        if (node == NULL) {

            

            node = g_malloc0(sizeof(AioHandler));

            node->e = e;

            node->pfd.fd = (uintptr_t)event_notifier_get_handle(e);

            node->pfd.events = G_IO_IN;

            QLIST_INSERT_HEAD(&ctx->aio_handlers, node, node);



            g_source_add_poll(&ctx->source, &node->pfd);

        }

        

        node->io_notify = io_notify;

        node->io_flush = io_flush;

    }



    aio_notify(ctx);

}
static BlockAIOCB *raw_aio_submit(BlockDriverState *bs,

        int64_t sector_num, QEMUIOVector *qiov, int nb_sectors,

        BlockCompletionFunc *cb, void *opaque, int type)

{

    BDRVRawState *s = bs->opaque;



    if (fd_open(bs) < 0)

        return NULL;



    

    if (s->needs_alignment) {

        if (!bdrv_qiov_is_aligned(bs, qiov)) {

            type |= QEMU_AIO_MISALIGNED;

#ifdef CONFIG_LINUX_AIO

        } else if (s->use_aio) {

            return laio_submit(bs, s->aio_ctx, s->fd, sector_num, qiov,

                               nb_sectors, cb, opaque, type);

#endif

        }

    }



    return paio_submit(bs, s->fd, sector_num, qiov, nb_sectors,

                       cb, opaque, type);

}
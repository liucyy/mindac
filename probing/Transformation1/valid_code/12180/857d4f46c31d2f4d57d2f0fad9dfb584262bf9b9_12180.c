static int coroutine_fn bdrv_co_do_write_zeroes(BlockDriverState *bs,

    int64_t sector_num, int nb_sectors, BdrvRequestFlags flags)

{

    BlockDriver *drv = bs->drv;

    QEMUIOVector qiov;

    struct iovec iov = {0};

    int ret = 0;



    int max_write_zeroes = bs->bl.max_write_zeroes ?

                           bs->bl.max_write_zeroes : MAX_WRITE_ZEROES_DEFAULT;



    while (nb_sectors > 0 && !ret) {

        int num = nb_sectors;



        

        if (bs->bl.write_zeroes_alignment

            && num > bs->bl.write_zeroes_alignment) {

            if (sector_num % bs->bl.write_zeroes_alignment != 0) {

                

                num = bs->bl.write_zeroes_alignment;

                num -= sector_num % bs->bl.write_zeroes_alignment;

            } else if ((sector_num + num) % bs->bl.write_zeroes_alignment != 0) {

                

                num -= (sector_num + num) % bs->bl.write_zeroes_alignment;

            }

        }



        

        if (num > max_write_zeroes) {

            num = max_write_zeroes;

        }



        ret = -ENOTSUP;

        

        if (drv->bdrv_co_write_zeroes) {

            ret = drv->bdrv_co_write_zeroes(bs, sector_num, num, flags);

        }



        if (ret == -ENOTSUP) {

            

            iov.iov_len = num * BDRV_SECTOR_SIZE;

            if (iov.iov_base == NULL) {

                iov.iov_base = qemu_blockalign(bs, num * BDRV_SECTOR_SIZE);

                memset(iov.iov_base, 0, num * BDRV_SECTOR_SIZE);

            }

            qemu_iovec_init_external(&qiov, &iov, 1);



            ret = drv->bdrv_co_writev(bs, sector_num, num, &qiov);



            

            if (num < max_write_zeroes) {

                qemu_vfree(iov.iov_base);

                iov.iov_base = NULL;

            }

        }



        sector_num += num;

        nb_sectors -= num;

    }



    qemu_vfree(iov.iov_base);

    return ret;

}
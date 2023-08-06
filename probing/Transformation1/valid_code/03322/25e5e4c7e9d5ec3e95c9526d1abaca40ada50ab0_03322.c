qemu_co_sendv_recvv(int sockfd, struct iovec *iov, unsigned iov_cnt,

                    size_t offset, size_t bytes, bool do_send)

{

    size_t done = 0;

    ssize_t ret;

    while (done < bytes) {

        ret = iov_send_recv(sockfd, iov,

                            offset + done, bytes - done, do_send);

        if (ret > 0) {

            done += ret;

        } else if (ret < 0) {

            if (errno == EAGAIN) {

                qemu_coroutine_yield();

            } else if (done == 0) {

                return -1;

            } else {

                break;

            }

        } else if (ret == 0 && !do_send) {

            

            break;

        }

    }

    return done;

}
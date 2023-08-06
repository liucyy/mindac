int virtqueue_pop(VirtQueue *vq, VirtQueueElement *elem)

{

    unsigned int i, head, max;

    hwaddr desc_pa = vq->vring.desc;

    VirtIODevice *vdev = vq->vdev;



    if (!virtqueue_num_heads(vq, vq->last_avail_idx))

        return 0;



    

    elem->out_num = elem->in_num = 0;



    max = vq->vring.num;



    i = head = virtqueue_get_head(vq, vq->last_avail_idx++);

    if (virtio_vdev_has_feature(vdev, VIRTIO_RING_F_EVENT_IDX)) {

        vring_set_avail_event(vq, vq->last_avail_idx);

    }



    if (vring_desc_flags(vdev, desc_pa, i) & VRING_DESC_F_INDIRECT) {

        if (vring_desc_len(vdev, desc_pa, i) % sizeof(VRingDesc)) {

            error_report("Invalid size for indirect buffer table");

            exit(1);

        }



        

        max = vring_desc_len(vdev, desc_pa, i) / sizeof(VRingDesc);

        desc_pa = vring_desc_addr(vdev, desc_pa, i);

        i = 0;

    }



    

    do {

        struct iovec *sg;



        if (vring_desc_flags(vdev, desc_pa, i) & VRING_DESC_F_WRITE) {

            if (elem->in_num >= ARRAY_SIZE(elem->in_sg)) {

                error_report("Too many write descriptors in indirect table");

                exit(1);

            }

            elem->in_addr[elem->in_num] = vring_desc_addr(vdev, desc_pa, i);

            sg = &elem->in_sg[elem->in_num++];

        } else {

            if (elem->out_num >= ARRAY_SIZE(elem->out_sg)) {

                error_report("Too many read descriptors in indirect table");

                exit(1);

            }

            elem->out_addr[elem->out_num] = vring_desc_addr(vdev, desc_pa, i);

            sg = &elem->out_sg[elem->out_num++];

        }



        sg->iov_len = vring_desc_len(vdev, desc_pa, i);



        

        if ((elem->in_num + elem->out_num) > max) {

            error_report("Looped descriptor");

            exit(1);

        }

    } while ((i = virtqueue_next_desc(vdev, desc_pa, i, max)) != max);



    

    virtqueue_map(elem);



    elem->index = head;



    vq->inuse++;



    trace_virtqueue_pop(vq, elem, elem->in_num, elem->out_num);

    return elem->in_num + elem->out_num;

}
static int handle_alloc(BlockDriverState *bs, uint64_t guest_offset,

    uint64_t *host_offset, uint64_t *bytes, QCowL2Meta **m)

{

    BDRVQcow2State *s = bs->opaque;

    int l2_index;

    uint64_t *l2_table;

    uint64_t entry;

    unsigned int nb_clusters;

    int ret;



    uint64_t alloc_cluster_offset;



    trace_qcow2_handle_alloc(qemu_coroutine_self(), guest_offset, *host_offset,

                             *bytes);

    assert(*bytes > 0);



    

    nb_clusters =

        size_to_clusters(s, offset_into_cluster(s, guest_offset) + *bytes);



    l2_index = offset_to_l2_index(s, guest_offset);

    nb_clusters = MIN(nb_clusters, s->l2_size - l2_index);



    

    ret = get_cluster_table(bs, guest_offset, &l2_table, &l2_index);

    if (ret < 0) {

        return ret;

    }



    entry = be64_to_cpu(l2_table[l2_index]);



    

    if (entry & QCOW_OFLAG_COMPRESSED) {

        nb_clusters = 1;

    } else {

        nb_clusters = count_cow_clusters(s, nb_clusters, l2_table, l2_index);

    }



    

    assert(nb_clusters > 0);



    qcow2_cache_put(bs, s->l2_table_cache, (void **) &l2_table);



    

    alloc_cluster_offset = start_of_cluster(s, *host_offset);

    ret = do_alloc_cluster_offset(bs, guest_offset, &alloc_cluster_offset,

                                  &nb_clusters);

    if (ret < 0) {

        goto fail;

    }



    

    if (nb_clusters == 0) {

        *bytes = 0;

        return 0;

    }



    

    if (!alloc_cluster_offset) {

        ret = qcow2_pre_write_overlap_check(bs, 0, alloc_cluster_offset,

                                            nb_clusters * s->cluster_size);

        assert(ret < 0);

        goto fail;

    }



    

    int requested_sectors =

        (*bytes + offset_into_cluster(s, guest_offset))

        >> BDRV_SECTOR_BITS;

    int avail_sectors = nb_clusters

                        << (s->cluster_bits - BDRV_SECTOR_BITS);

    int alloc_n_start = offset_into_cluster(s, guest_offset)

                        >> BDRV_SECTOR_BITS;

    int nb_sectors = MIN(requested_sectors, avail_sectors);

    QCowL2Meta *old_m = *m;



    *m = g_malloc0(sizeof(**m));



    **m = (QCowL2Meta) {

        .next           = old_m,



        .alloc_offset   = alloc_cluster_offset,

        .offset         = start_of_cluster(s, guest_offset),

        .nb_clusters    = nb_clusters,

        .nb_available   = nb_sectors,



        .cow_start = {

            .offset     = 0,

            .nb_sectors = alloc_n_start,

        },

        .cow_end = {

            .offset     = nb_sectors * BDRV_SECTOR_SIZE,

            .nb_sectors = avail_sectors - nb_sectors,

        },

    };

    qemu_co_queue_init(&(*m)->dependent_requests);

    QLIST_INSERT_HEAD(&s->cluster_allocs, *m, next_in_flight);



    *host_offset = alloc_cluster_offset + offset_into_cluster(s, guest_offset);

    *bytes = MIN(*bytes, (nb_sectors * BDRV_SECTOR_SIZE)

                         - offset_into_cluster(s, guest_offset));

    assert(*bytes != 0);



    return 1;



fail:

    if (*m && (*m)->nb_clusters > 0) {

        QLIST_REMOVE(*m, next_in_flight);

    }

    return ret;

}
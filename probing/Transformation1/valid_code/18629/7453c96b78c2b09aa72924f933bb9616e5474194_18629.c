static int calculate_refcounts(BlockDriverState *bs, BdrvCheckResult *res,

                               BdrvCheckMode fix, bool *rebuild,

                               uint16_t **refcount_table, int64_t *nb_clusters)

{

    BDRVQcowState *s = bs->opaque;

    int64_t i;

    QCowSnapshot *sn;

    int ret;



    if (!*refcount_table) {

        int64_t old_size = 0;

        ret = realloc_refcount_array(s, refcount_table,

                                     &old_size, *nb_clusters);

        if (ret < 0) {

            res->check_errors++;

            return ret;

        }

    }



    

    ret = inc_refcounts(bs, res, refcount_table, nb_clusters,

                        0, s->cluster_size);

    if (ret < 0) {

        return ret;

    }



    

    ret = check_refcounts_l1(bs, res, refcount_table, nb_clusters,

                             s->l1_table_offset, s->l1_size, CHECK_FRAG_INFO);

    if (ret < 0) {

        return ret;

    }



    

    for (i = 0; i < s->nb_snapshots; i++) {

        sn = s->snapshots + i;

        ret = check_refcounts_l1(bs, res, refcount_table, nb_clusters,

                                 sn->l1_table_offset, sn->l1_size, 0);

        if (ret < 0) {

            return ret;

        }

    }

    ret = inc_refcounts(bs, res, refcount_table, nb_clusters,

                        s->snapshots_offset, s->snapshots_size);

    if (ret < 0) {

        return ret;

    }



    

    ret = inc_refcounts(bs, res, refcount_table, nb_clusters,

                        s->refcount_table_offset,

                        s->refcount_table_size * sizeof(uint64_t));

    if (ret < 0) {

        return ret;

    }



    return check_refblocks(bs, res, fix, rebuild, refcount_table, nb_clusters);

}
static int get_cluster_offset(BlockDriverState *bs,

                                    VmdkExtent *extent,

                                    VmdkMetaData *m_data,

                                    uint64_t offset,

                                    int allocate,

                                    uint64_t *cluster_offset)

{

    unsigned int l1_index, l2_offset, l2_index;

    int min_index, i, j;

    uint32_t min_count, *l2_table;

    bool zeroed = false;



    if (m_data) {

        m_data->valid = 0;

    }

    if (extent->flat) {

        *cluster_offset = extent->flat_start_offset;

        return VMDK_OK;

    }



    offset -= (extent->end_sector - extent->sectors) * SECTOR_SIZE;

    l1_index = (offset >> 9) / extent->l1_entry_sectors;

    if (l1_index >= extent->l1_size) {

        return VMDK_ERROR;

    }

    l2_offset = extent->l1_table[l1_index];

    if (!l2_offset) {

        return VMDK_UNALLOC;

    }

    for (i = 0; i < L2_CACHE_SIZE; i++) {

        if (l2_offset == extent->l2_cache_offsets[i]) {

            

            if (++extent->l2_cache_counts[i] == 0xffffffff) {

                for (j = 0; j < L2_CACHE_SIZE; j++) {

                    extent->l2_cache_counts[j] >>= 1;

                }

            }

            l2_table = extent->l2_cache + (i * extent->l2_size);

            goto found;

        }

    }

    

    min_index = 0;

    min_count = 0xffffffff;

    for (i = 0; i < L2_CACHE_SIZE; i++) {

        if (extent->l2_cache_counts[i] < min_count) {

            min_count = extent->l2_cache_counts[i];

            min_index = i;

        }

    }

    l2_table = extent->l2_cache + (min_index * extent->l2_size);

    if (bdrv_pread(

                extent->file,

                (int64_t)l2_offset * 512,

                l2_table,

                extent->l2_size * sizeof(uint32_t)

            ) != extent->l2_size * sizeof(uint32_t)) {

        return VMDK_ERROR;

    }



    extent->l2_cache_offsets[min_index] = l2_offset;

    extent->l2_cache_counts[min_index] = 1;

 found:

    l2_index = ((offset >> 9) / extent->cluster_sectors) % extent->l2_size;

    *cluster_offset = le32_to_cpu(l2_table[l2_index]);



    if (extent->has_zero_grain && *cluster_offset == VMDK_GTE_ZEROED) {

        zeroed = true;

    }



    if (!*cluster_offset || zeroed) {

        if (!allocate) {

            return zeroed ? VMDK_ZEROED : VMDK_UNALLOC;

        }



        

        *cluster_offset = bdrv_getlength(extent->file);

        if (!extent->compressed) {

            bdrv_truncate(

                extent->file,

                *cluster_offset + (extent->cluster_sectors << 9)

            );

        }



        *cluster_offset >>= 9;

        l2_table[l2_index] = cpu_to_le32(*cluster_offset);



        

        if (get_whole_cluster(

                bs, extent, *cluster_offset, offset, allocate) == -1) {

            return VMDK_ERROR;

        }



        if (m_data) {

            m_data->offset = *cluster_offset;

            m_data->l1_index = l1_index;

            m_data->l2_index = l2_index;

            m_data->l2_offset = l2_offset;

            m_data->valid = 1;

        }

    }

    *cluster_offset <<= 9;

    return VMDK_OK;

}
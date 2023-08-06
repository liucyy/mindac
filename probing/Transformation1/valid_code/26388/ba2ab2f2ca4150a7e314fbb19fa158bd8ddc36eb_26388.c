static int qcow2_create2(const char *filename, int64_t total_size,
                         const char *backing_file, const char *backing_format,
                         int flags, size_t cluster_size, int prealloc,
                         QEMUOptionParameter *options, int version,
                         Error **errp)
{
    
    int cluster_bits;
    cluster_bits = ffs(cluster_size) - 1;
    if (cluster_bits < MIN_CLUSTER_BITS || cluster_bits > MAX_CLUSTER_BITS ||
        (1 << cluster_bits) != cluster_size)
    {
        error_setg(errp, "Cluster size must be a power of two between %d and "
                   "%dk", 1 << MIN_CLUSTER_BITS, 1 << (MAX_CLUSTER_BITS - 10));
        return -EINVAL;
    
    BlockDriverState* bs;
    QCowHeader header;
    uint8_t* refcount_table;
    Error *local_err = NULL;
    int ret;
    ret = bdrv_create_file(filename, options, &local_err);
    if (ret < 0) {
        return ret;
    ret = bdrv_file_open(&bs, filename, NULL, BDRV_O_RDWR, &local_err);
    if (ret < 0) {
        return ret;
    
    memset(&header, 0, sizeof(header));
    header.magic = cpu_to_be32(QCOW_MAGIC);
    header.version = cpu_to_be32(version);
    header.cluster_bits = cpu_to_be32(cluster_bits);
    header.size = cpu_to_be64(0);
    header.l1_table_offset = cpu_to_be64(0);
    header.l1_size = cpu_to_be32(0);
    header.refcount_table_offset = cpu_to_be64(cluster_size);
    header.refcount_table_clusters = cpu_to_be32(1);
    header.refcount_order = cpu_to_be32(3 + REFCOUNT_SHIFT);
    header.header_length = cpu_to_be32(sizeof(header));
    if (flags & BLOCK_FLAG_ENCRYPT) {
        header.crypt_method = cpu_to_be32(QCOW_CRYPT_AES);
    } else {
        header.crypt_method = cpu_to_be32(QCOW_CRYPT_NONE);
    if (flags & BLOCK_FLAG_LAZY_REFCOUNTS) {
        header.compatible_features |=
            cpu_to_be64(QCOW2_COMPAT_LAZY_REFCOUNTS);
    ret = bdrv_pwrite(bs, 0, &header, sizeof(header));
    if (ret < 0) {
        error_setg_errno(errp, -ret, "Could not write qcow2 header");
    
    refcount_table = g_malloc0(cluster_size);
    ret = bdrv_pwrite(bs, cluster_size, refcount_table, cluster_size);
    g_free(refcount_table);
    if (ret < 0) {
        error_setg_errno(errp, -ret, "Could not write refcount table");
    
    BlockDriver* drv = bdrv_find_format("qcow2");
    assert(drv != NULL);
        BDRV_O_RDWR | BDRV_O_CACHE_WB | BDRV_O_NO_FLUSH, drv, &local_err);
    if (ret < 0) {
    ret = qcow2_alloc_clusters(bs, 2 * cluster_size);
    if (ret < 0) {
        error_setg_errno(errp, -ret, "Could not allocate clusters for qcow2 "
                         "header and refcount table");
    } else if (ret != 0) {
        error_report("Huh, first cluster in empty image is already in use?");
        abort();
    
    ret = bdrv_truncate(bs, total_size * BDRV_SECTOR_SIZE);
    if (ret < 0) {
        error_setg_errno(errp, -ret, "Could not resize image");
    
    if (backing_file) {
        ret = bdrv_change_backing_file(bs, backing_file, backing_format);
        if (ret < 0) {
            error_setg_errno(errp, -ret, "Could not assign backing file '%s' "
                             "with format '%s'", backing_file, backing_format);
    
    if (prealloc) {
        BDRVQcowState *s = bs->opaque;
        qemu_co_mutex_lock(&s->lock);
        ret = preallocate(bs);
        qemu_co_mutex_unlock(&s->lock);
        if (ret < 0) {
            error_setg_errno(errp, -ret, "Could not preallocate metadata");
    ret = 0;
out:
    bdrv_unref(bs);
    return ret;
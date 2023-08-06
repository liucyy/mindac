void qemu_coroutine_adjust_pool_size(int n)

{

    qemu_mutex_lock(&pool_lock);



    pool_max_size += n;



    

    assert(pool_max_size >= POOL_DEFAULT_SIZE);



    

    while (pool_size > pool_max_size) {

        Coroutine *co = QSLIST_FIRST(&pool);

        QSLIST_REMOVE_HEAD(&pool, pool_next);

        pool_size--;

        qemu_coroutine_delete(co);

    }



    qemu_mutex_unlock(&pool_lock);

}
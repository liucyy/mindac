static int mp_dacl_removexattr(FsContext *ctx,

                               const char *path, const char *name)

{

    int ret;

    char *buffer;



    buffer = rpath(ctx, path);

    ret  = lremovexattr(buffer, MAP_ACL_DEFAULT);

    if (ret == -1 && errno == ENODATA) {

        

        errno = 0;

        ret = 0;

    }

    g_free(buffer);

    return ret;

}
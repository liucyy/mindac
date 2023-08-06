static int local_open2(FsContext *fs_ctx, V9fsPath *dir_path, const char *name,

                       int flags, FsCred *credp, V9fsFidOpenState *fs)

{

    char *path;

    int fd = -1;

    int err = -1;

    int serrno = 0;

    V9fsString fullname;

    char buffer[PATH_MAX];



    

    flags |= O_NOFOLLOW;



    v9fs_string_init(&fullname);

    v9fs_string_sprintf(&fullname, "%s/%s", dir_path->data, name);

    path = fullname.data;



    

    if (fs_ctx->export_flags & V9FS_SM_MAPPED) {

        fd = open(rpath(fs_ctx, path, buffer), flags, SM_LOCAL_MODE_BITS);

        if (fd == -1) {

            err = fd;

            goto out;

        }

        credp->fc_mode = credp->fc_mode|S_IFREG;

        

        err = local_set_xattr(rpath(fs_ctx, path, buffer), credp);

        if (err == -1) {

            serrno = errno;

            goto err_end;

        }

    } else if (fs_ctx->export_flags & V9FS_SM_MAPPED_FILE) {

        fd = open(rpath(fs_ctx, path, buffer), flags, SM_LOCAL_MODE_BITS);

        if (fd == -1) {

            err = fd;

            goto out;

        }

        credp->fc_mode = credp->fc_mode|S_IFREG;

        

        err = local_set_mapped_file_attr(fs_ctx, path, credp);

        if (err == -1) {

            serrno = errno;

            goto err_end;

        }

    } else if ((fs_ctx->export_flags & V9FS_SM_PASSTHROUGH) ||

               (fs_ctx->export_flags & V9FS_SM_NONE)) {

        fd = open(rpath(fs_ctx, path, buffer), flags, credp->fc_mode);

        if (fd == -1) {

            err = fd;

            goto out;

        }

        err = local_post_create_passthrough(fs_ctx, path, credp);

        if (err == -1) {

            serrno = errno;

            goto err_end;

        }

    }

    err = fd;

    fs->fd = fd;

    goto out;



err_end:

    close(fd);

    remove(rpath(fs_ctx, path, buffer));

    errno = serrno;

out:

    v9fs_string_free(&fullname);

    return err;

}
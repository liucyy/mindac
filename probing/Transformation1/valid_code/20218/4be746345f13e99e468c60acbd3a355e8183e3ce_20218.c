static void scsi_block_realize(SCSIDevice *dev, Error **errp)

{

    SCSIDiskState *s = DO_UPCAST(SCSIDiskState, qdev, dev);

    int sg_version;

    int rc;



    if (!s->qdev.conf.bs) {

        error_setg(errp, "drive property not set");

        return;

    }



    

    rc = bdrv_ioctl(s->qdev.conf.bs, SG_GET_VERSION_NUM, &sg_version);

    if (rc < 0) {

        error_setg(errp, "cannot get SG_IO version number: %s.  "

                     "Is this a SCSI device?",

                     strerror(-rc));

        return;

    }

    if (sg_version < 30000) {

        error_setg(errp, "scsi generic interface too old");

        return;

    }



    

    rc = get_device_type(s);

    if (rc < 0) {

        error_setg(errp, "INQUIRY failed");

        return;

    }



    

    if (s->qdev.type == TYPE_ROM || s->qdev.type == TYPE_WORM) {

        s->qdev.blocksize = 2048;

    } else {

        s->qdev.blocksize = 512;

    }



    

    s->features |= (1 << SCSI_DISK_F_NO_REMOVABLE_DEVOPS);



    scsi_realize(&s->qdev, errp);

}
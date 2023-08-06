static bool scsi_target_emulate_report_luns(SCSITargetReq *r)

{

    BusChild *kid;

    int i, len, n;

    int channel, id;

    bool found_lun0;



    if (r->req.cmd.xfer < 16) {

        return false;

    }

    if (r->req.cmd.buf[2] > 2) {

        return false;

    }

    channel = r->req.dev->channel;

    id = r->req.dev->id;

    found_lun0 = false;

    n = 0;

    QTAILQ_FOREACH(kid, &r->req.bus->qbus.children, sibling) {

        DeviceState *qdev = kid->child;

        SCSIDevice *dev = SCSI_DEVICE(qdev);



        if (dev->channel == channel && dev->id == id) {

            if (dev->lun == 0) {

                found_lun0 = true;

            }

            n += 8;

        }

    }

    if (!found_lun0) {

        n += 8;

    }

    len = MIN(n + 8, r->req.cmd.xfer & ~7);

    if (len > sizeof(r->buf)) {

        

        return false;

    }



    memset(r->buf, 0, len);

    stl_be_p(&r->buf, n);

    i = found_lun0 ? 8 : 16;

    QTAILQ_FOREACH(kid, &r->req.bus->qbus.children, sibling) {

        DeviceState *qdev = kid->child;

        SCSIDevice *dev = SCSI_DEVICE(qdev);



        if (dev->channel == channel && dev->id == id) {

            store_lun(&r->buf[i], dev->lun);

            i += 8;

        }

    }

    assert(i == n + 8);

    r->len = len;

    return true;

}
static int scsi_hot_add(Monitor *mon, DeviceState *adapter,

                        DriveInfo *dinfo, int printinfo)

{

    SCSIBus *scsibus;

    SCSIDevice *scsidev;



    scsibus = DO_UPCAST(SCSIBus, qbus, QLIST_FIRST(&adapter->child_bus));

    if (!scsibus || strcmp(scsibus->qbus.info->name, "SCSI") != 0) {

        error_report("Device is not a SCSI adapter");

        return -1;

    }



    

    dinfo->unit = qemu_opt_get_number(dinfo->opts, "unit", -1);

    scsidev = scsi_bus_legacy_add_drive(scsibus, dinfo, dinfo->unit);

    if (!scsidev) {

        return -1;

    }

    dinfo->unit = scsidev->id;



    if (printinfo)

        monitor_printf(mon, "OK bus %d, unit %d\n",

                       scsibus->busnr, scsidev->id);

    return 0;

}
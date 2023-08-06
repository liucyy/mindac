static int mxf_read_material_package(MXFPackage *package, ByteIOContext *pb, int tag)

{

    switch(tag) {

    case 0x4403:

        package->tracks_count = get_be32(pb);

        if (package->tracks_count >= UINT_MAX / sizeof(UID))

            return -1;

        package->tracks_refs = av_malloc(package->tracks_count * sizeof(UID));

        if (!package->tracks_refs)

            return -1;

        url_fskip(pb, 4); 

        get_buffer(pb, (uint8_t *)package->tracks_refs, package->tracks_count * sizeof(UID));

        break;

    }

    return 0;

}
static int protocol_client_init(VncState *vs, uint8_t *data, size_t len)

{

    char buf[1024];

    VncShareMode mode;

    int size;



    mode = data[0] ? VNC_SHARE_MODE_SHARED : VNC_SHARE_MODE_EXCLUSIVE;

    switch (vs->vd->share_policy) {

    case VNC_SHARE_POLICY_IGNORE:

        

        break;

    case VNC_SHARE_POLICY_ALLOW_EXCLUSIVE:

        

        if (mode == VNC_SHARE_MODE_EXCLUSIVE) {

            VncState *client;

            QTAILQ_FOREACH(client, &vs->vd->clients, next) {

                if (vs == client) {

                    continue;

                }

                if (client->share_mode != VNC_SHARE_MODE_EXCLUSIVE &&

                    client->share_mode != VNC_SHARE_MODE_SHARED) {

                    continue;

                }

                vnc_disconnect_start(client);

            }

        }

        if (mode == VNC_SHARE_MODE_SHARED) {

            if (vs->vd->num_exclusive > 0) {

                vnc_disconnect_start(vs);

                return 0;

            }

        }

        break;

    case VNC_SHARE_POLICY_FORCE_SHARED:

        

        if (mode == VNC_SHARE_MODE_EXCLUSIVE) {

            vnc_disconnect_start(vs);

            return 0;

        }

        break;

    }

    vnc_set_share_mode(vs, mode);



    vs->client_width = surface_width(vs->vd->ds);

    vs->client_height = surface_height(vs->vd->ds);

    vnc_write_u16(vs, vs->client_width);

    vnc_write_u16(vs, vs->client_height);



    pixel_format_message(vs);



    if (qemu_name)

        size = snprintf(buf, sizeof(buf), "QEMU (%s)", qemu_name);

    else

        size = snprintf(buf, sizeof(buf), "QEMU");



    vnc_write_u32(vs, size);

    vnc_write(vs, buf, size);

    vnc_flush(vs);



    vnc_client_cache_auth(vs);

    vnc_qmp_event(vs, QAPI_EVENT_VNC_INITIALIZED);



    vnc_read_when(vs, protocol_client_msg, 1);



    return 0;

}
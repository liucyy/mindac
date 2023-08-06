int vnc_display_password(DisplayState *ds, const char *password)

{

    VncDisplay *vs = ds ? (VncDisplay *)ds->opaque : vnc_display;



    if (!vs) {

        return -EINVAL;

    }



    if (!password) {

        

        return vnc_display_disable_login(ds);

    }



    if (vs->password) {

        g_free(vs->password);

        vs->password = NULL;

    }

    vs->password = g_strdup(password);

    if (vs->auth == VNC_AUTH_NONE) {

        vs->auth = VNC_AUTH_VNC;

    }



    return 0;

}
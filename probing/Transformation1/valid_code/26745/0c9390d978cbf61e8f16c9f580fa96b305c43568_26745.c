static void nbd_client_closed(NBDClient *client)

{

    nb_fds--;

    if (nb_fds == 0 && !persistent && state == RUNNING) {

        state = TERMINATE;

    }

    nbd_update_server_watch();

    nbd_client_put(client);

}
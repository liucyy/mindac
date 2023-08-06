static int nbd_negotiate_handle_info(NBDClient *client, uint32_t length,

                                     uint32_t opt, uint16_t myflags,

                                     Error **errp)

{

    int rc;

    char name[NBD_MAX_NAME_SIZE + 1];

    NBDExport *exp;

    uint16_t requests;

    uint16_t request;

    uint32_t namelen;

    bool sendname = false;

    bool blocksize = false;

    uint32_t sizes[3];

    char buf[sizeof(uint64_t) + sizeof(uint16_t)];

    const char *msg;



    

    if (length < sizeof(namelen) + sizeof(requests)) {

        msg = "overall request too short";



    if (nbd_read(client->ioc, &namelen, sizeof(namelen), errp) < 0) {

        return -EIO;


    be32_to_cpus(&namelen);

    length -= sizeof(namelen);

    if (namelen > length - sizeof(requests) || (length - namelen) % 2) {

        msg = "name length is incorrect";







    if (nbd_read(client->ioc, name, namelen, errp) < 0) {

        return -EIO;


    name[namelen] = '\0';

    length -= namelen;

    trace_nbd_negotiate_handle_export_name_request(name);



    if (nbd_read(client->ioc, &requests, sizeof(requests), errp) < 0) {

        return -EIO;


    be16_to_cpus(&requests);

    length -= sizeof(requests);

    trace_nbd_negotiate_handle_info_requests(requests);

    if (requests != length / sizeof(request)) {

        msg = "incorrect number of  requests for overall length";



    while (requests--) {

        if (nbd_read(client->ioc, &request, sizeof(request), errp) < 0) {

            return -EIO;


        be16_to_cpus(&request);

        length -= sizeof(request);

        trace_nbd_negotiate_handle_info_request(request,

                                                nbd_info_lookup(request));

        

        switch (request) {

        case NBD_INFO_NAME:

            sendname = true;

            break;

        case NBD_INFO_BLOCK_SIZE:

            blocksize = true;

            break;



    assert(length == 0);



    exp = nbd_export_find(name);

    if (!exp) {

        return nbd_negotiate_send_rep_err(client->ioc, NBD_REP_ERR_UNKNOWN,

                                          opt, errp, "export '%s' not present",

                                          name);




    

    if (sendname) {

        rc = nbd_negotiate_send_info(client, opt, NBD_INFO_NAME, namelen, name,

                                     errp);

        if (rc < 0) {

            return rc;





    

    if (exp->description) {

        size_t len = strlen(exp->description);



        rc = nbd_negotiate_send_info(client, opt, NBD_INFO_DESCRIPTION,

                                     len, exp->description, errp);

        if (rc < 0) {

            return rc;





    

    

    sizes[0] = (opt == NBD_OPT_INFO || blocksize) ? BDRV_SECTOR_SIZE : 1;

    

    sizes[1] = 4096;

    

    sizes[2] = MIN(blk_get_max_transfer(exp->blk), NBD_MAX_BUFFER_SIZE);

    trace_nbd_negotiate_handle_info_block_size(sizes[0], sizes[1], sizes[2]);

    cpu_to_be32s(&sizes[0]);

    cpu_to_be32s(&sizes[1]);

    cpu_to_be32s(&sizes[2]);

    rc = nbd_negotiate_send_info(client, opt, NBD_INFO_BLOCK_SIZE,

                                 sizeof(sizes), sizes, errp);

    if (rc < 0) {

        return rc;




    

    trace_nbd_negotiate_new_style_size_flags(exp->size,

                                             exp->nbdflags | myflags);

    stq_be_p(buf, exp->size);

    stw_be_p(buf + 8, exp->nbdflags | myflags);

    rc = nbd_negotiate_send_info(client, opt, NBD_INFO_EXPORT,

                                 sizeof(buf), buf, errp);

    if (rc < 0) {

        return rc;




    

    if (opt == NBD_OPT_INFO && !blocksize) {

        return nbd_negotiate_send_rep_err(client->ioc,

                                          NBD_REP_ERR_BLOCK_SIZE_REQD, opt,

                                          errp,

                                          "request NBD_INFO_BLOCK_SIZE to "

                                          "use this export");




    

    rc = nbd_negotiate_send_rep(client->ioc, NBD_REP_ACK, opt, errp);

    if (rc < 0) {

        return rc;




    if (opt == NBD_OPT_GO) {

        client->exp = exp;

        QTAILQ_INSERT_TAIL(&client->exp->clients, client, next);

        nbd_export_get(client->exp);

        rc = 1;


    return rc;



 invalid:

    if (nbd_drop(client->ioc, length, errp) < 0) {

        return -EIO;


    return nbd_negotiate_send_rep_err(client->ioc, NBD_REP_ERR_INVALID, opt,

                                      errp, "%s", msg);
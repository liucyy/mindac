static void vscsi_got_payload(VSCSIState *s, vscsi_crq *crq)

{

    vscsi_req *req;

    int done;



    req = vscsi_get_req(s);

    if (req == NULL) {

        fprintf(stderr, "VSCSI: Failed to get a request !\n");

        return;

    }



    

    if (crq->s.IU_length > sizeof(union viosrp_iu)) {

        fprintf(stderr, "VSCSI: SRP IU too long (%d bytes) !\n",

                crq->s.IU_length);

        return;

    }



    

    if (spapr_tce_dma_read(&s->vdev, crq->s.IU_data_ptr, &req->iu,

                           crq->s.IU_length)) {

        fprintf(stderr, "vscsi_got_payload: DMA read failure !\n");

        g_free(req);

    }

    memcpy(&req->crq, crq, sizeof(vscsi_crq));



    if (crq->s.format == VIOSRP_MAD_FORMAT) {

        done = vscsi_handle_mad_req(s, req);

    } else {

        done = vscsi_handle_srp_req(s, req);

    }



    if (done) {

        vscsi_put_req(req);

    }

}
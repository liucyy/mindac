static void ccid_handle_bulk_out(USBCCIDState *s, USBPacket *p)

{

    CCID_Header *ccid_header;



    if (p->iov.size + s->bulk_out_pos > BULK_OUT_DATA_SIZE) {

        goto err;

    }

    usb_packet_copy(p, s->bulk_out_data + s->bulk_out_pos, p->iov.size);

    s->bulk_out_pos += p->iov.size;

    if (s->bulk_out_pos < 10) {

        DPRINTF(s, 1, "%s: header incomplete\n", __func__);

        goto err;

    }



    ccid_header = (CCID_Header *)s->bulk_out_data;

    if (p->iov.size == CCID_MAX_PACKET_SIZE) {

        DPRINTF(s, D_VERBOSE,

            "usb-ccid: bulk_in: expecting more packets (%zd/%d)\n",

            p->iov.size, ccid_header->dwLength);

        return;

    }



    DPRINTF(s, D_MORE_INFO, "%s %x %s\n", __func__,

            ccid_header->bMessageType,

            ccid_message_type_to_str(ccid_header->bMessageType));

    switch (ccid_header->bMessageType) {

    case CCID_MESSAGE_TYPE_PC_to_RDR_GetSlotStatus:

        ccid_write_slot_status(s, ccid_header);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_IccPowerOn:

        DPRINTF(s, 1, "%s: PowerOn: %d\n", __func__,

                ((CCID_IccPowerOn *)(ccid_header))->bPowerSelect);

        s->powered = true;

        if (!ccid_card_inserted(s)) {

            ccid_report_error_failed(s, ERROR_ICC_MUTE);

        }

        

        ccid_write_data_block_atr(s, ccid_header);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_IccPowerOff:

        ccid_reset_error_status(s);

        s->powered = false;

        ccid_write_slot_status(s, ccid_header);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_XfrBlock:

        ccid_on_apdu_from_guest(s, (CCID_XferBlock *)s->bulk_out_data);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_SetParameters:

        ccid_reset_error_status(s);

        ccid_set_parameters(s, ccid_header);

        ccid_write_parameters(s, ccid_header);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_ResetParameters:

        ccid_reset_error_status(s);

        ccid_reset_parameters(s);

        ccid_write_parameters(s, ccid_header);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_GetParameters:

        ccid_reset_error_status(s);

        ccid_write_parameters(s, ccid_header);

        break;

    case CCID_MESSAGE_TYPE_PC_to_RDR_Mechanical:

        ccid_report_error_failed(s, 0);

        ccid_write_slot_status(s, ccid_header);

        break;

    default:

        DPRINTF(s, 1,

                "handle_data: ERROR: unhandled message type %Xh\n",

                ccid_header->bMessageType);

        

        ccid_report_error_failed(s, ERROR_CMD_NOT_SUPPORTED);

        ccid_write_slot_status(s, ccid_header);

        break;

    }

    s->bulk_out_pos = 0;

    return;



err:

    p->status = USB_RET_STALL;

    s->bulk_out_pos = 0;

    return;

}
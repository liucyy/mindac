void vga_ioport_write(void *opaque, uint32_t addr, uint32_t val)

{

    VGACommonState *s = opaque;

    int index;



    

    if (vga_ioport_invalid(s, addr)) {

        return;

    }

#ifdef DEBUG_VGA

    printf("VGA: write addr=0x%04x data=0x%02x\n", addr, val);

#endif



    switch(addr) {

    case VGA_ATT_W:

        if (s->ar_flip_flop == 0) {

            val &= 0x3f;

            s->ar_index = val;

        } else {

            index = s->ar_index & 0x1f;

            switch(index) {

            case VGA_ATC_PALETTE0 ... VGA_ATC_PALETTEF:

                s->ar[index] = val & 0x3f;

                break;

            case VGA_ATC_MODE:

                s->ar[index] = val & ~0x10;

                break;

            case VGA_ATC_OVERSCAN:

                s->ar[index] = val;

                break;

            case VGA_ATC_PLANE_ENABLE:

                s->ar[index] = val & ~0xc0;

                break;

            case VGA_ATC_PEL:

                s->ar[index] = val & ~0xf0;

                break;

            case VGA_ATC_COLOR_PAGE:

                s->ar[index] = val & ~0xf0;

                break;

            default:

                break;

            }

        }

        s->ar_flip_flop ^= 1;

        break;

    case VGA_MIS_W:

        s->msr = val & ~0x10;

        s->update_retrace_info(s);

        break;

    case VGA_SEQ_I:

        s->sr_index = val & 7;

        break;

    case VGA_SEQ_D:

#ifdef DEBUG_VGA_REG

        printf("vga: write SR%x = 0x%02x\n", s->sr_index, val);

#endif

        s->sr[s->sr_index] = val & sr_mask[s->sr_index];

        if (s->sr_index == VGA_SEQ_CLOCK_MODE) {

            s->update_retrace_info(s);

        }

        vga_update_memory_access(s);

        break;

    case VGA_PEL_IR:

        s->dac_read_index = val;

        s->dac_sub_index = 0;

        s->dac_state = 3;

        break;

    case VGA_PEL_IW:

        s->dac_write_index = val;

        s->dac_sub_index = 0;

        s->dac_state = 0;

        break;

    case VGA_PEL_D:

        s->dac_cache[s->dac_sub_index] = val;

        if (++s->dac_sub_index == 3) {

            memcpy(&s->palette[s->dac_write_index * 3], s->dac_cache, 3);

            s->dac_sub_index = 0;

            s->dac_write_index++;

        }

        break;

    case VGA_GFX_I:

        s->gr_index = val & 0x0f;

        break;

    case VGA_GFX_D:

#ifdef DEBUG_VGA_REG

        printf("vga: write GR%x = 0x%02x\n", s->gr_index, val);

#endif

        s->gr[s->gr_index] = val & gr_mask[s->gr_index];

        vga_update_memory_access(s);

        break;

    case VGA_CRT_IM:

    case VGA_CRT_IC:

        s->cr_index = val;

        break;

    case VGA_CRT_DM:

    case VGA_CRT_DC:

#ifdef DEBUG_VGA_REG

        printf("vga: write CR%x = 0x%02x\n", s->cr_index, val);

#endif

        

        if (s->cr[VGA_CRTC_V_SYNC_END] & VGA_CR11_LOCK_CR0_CR7) {

            if (s->cr_index <= VGA_CRTC_OVERFLOW) {

                

                if (s->cr_index == VGA_CRTC_OVERFLOW) {

                    s->cr[VGA_CRTC_OVERFLOW] =

                        (s->cr[VGA_CRTC_OVERFLOW] & ~0x10) | (val & 0x10);

                }

                return;

            } else if ((vga_cga_hacks & VGA_CGA_HACK_FONT_HEIGHT) &&

                       !(s->sr[VGA_SEQ_CLOCK_MODE] & VGA_SR01_CHAR_CLK_8DOTS)) {

                

                if (s->cr_index == VGA_CRTC_MAX_SCAN &&

                    val == 7 &&

                    (s->cr[VGA_CRTC_MAX_SCAN] & 0xf) == 0xf) {

                    return;

                } else if (s->cr_index == VGA_CRTC_CURSOR_START &&

                           val == 6 &&

                           (s->cr[VGA_CRTC_MAX_SCAN] & 0xf) == 0xf) {

                    val = 0xd;

                } else if (s->cr_index == VGA_CRTC_CURSOR_END &&

                           val == 7 &&

                           (s->cr[VGA_CRTC_MAX_SCAN] & 0xf) == 0xf) {

                    val = 0xe;

                }

            }

        }

        s->cr[s->cr_index] = val;



        switch(s->cr_index) {

        case VGA_CRTC_H_TOTAL:

        case VGA_CRTC_H_SYNC_START:

        case VGA_CRTC_H_SYNC_END:

        case VGA_CRTC_V_TOTAL:

        case VGA_CRTC_OVERFLOW:

        case VGA_CRTC_V_SYNC_END:

        case VGA_CRTC_MODE:

            s->update_retrace_info(s);

            break;

        }

        break;

    case VGA_IS1_RM:

    case VGA_IS1_RC:

        s->fcr = val & 0x10;

        break;

    }

}
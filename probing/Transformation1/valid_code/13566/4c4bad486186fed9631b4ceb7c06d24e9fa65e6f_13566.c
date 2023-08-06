static int ram_load_postcopy(QEMUFile *f)

{

    int flags = 0, ret = 0;

    bool place_needed = false;

    bool matching_page_sizes = qemu_host_page_size == TARGET_PAGE_SIZE;

    MigrationIncomingState *mis = migration_incoming_get_current();

    

    void *postcopy_host_page = postcopy_get_tmp_page(mis);

    void *last_host = NULL;

    bool all_zero = false;



    while (!ret && !(flags & RAM_SAVE_FLAG_EOS)) {

        ram_addr_t addr;

        void *host = NULL;

        void *page_buffer = NULL;

        void *place_source = NULL;

        uint8_t ch;



        addr = qemu_get_be64(f);

        flags = addr & ~TARGET_PAGE_MASK;

        addr &= TARGET_PAGE_MASK;



        trace_ram_load_postcopy_loop((uint64_t)addr, flags);

        place_needed = false;

        if (flags & (RAM_SAVE_FLAG_COMPRESS | RAM_SAVE_FLAG_PAGE)) {

            host = host_from_stream_offset(f, addr, flags);

            if (!host) {

                error_report("Illegal RAM offset " RAM_ADDR_FMT, addr);

                ret = -EINVAL;

                break;

            }

            page_buffer = host;

            

            page_buffer = postcopy_host_page +

                          ((uintptr_t)host & ~qemu_host_page_mask);

            

            if (!((uintptr_t)host & ~qemu_host_page_mask)) {

                all_zero = true;

            } else {

                

                if (host != (last_host + TARGET_PAGE_SIZE)) {

                    error_report("Non-sequential target page %p/%p",

                                  host, last_host);

                    ret = -EINVAL;

                    break;

                }

            }





            

            place_needed = (((uintptr_t)host + TARGET_PAGE_SIZE) &

                                     ~qemu_host_page_mask) == 0;

            place_source = postcopy_host_page;

        }

        last_host = host;



        switch (flags & ~RAM_SAVE_FLAG_CONTINUE) {

        case RAM_SAVE_FLAG_COMPRESS:

            ch = qemu_get_byte(f);

            memset(page_buffer, ch, TARGET_PAGE_SIZE);

            if (ch) {

                all_zero = false;

            }

            break;



        case RAM_SAVE_FLAG_PAGE:

            all_zero = false;

            if (!place_needed || !matching_page_sizes) {

                qemu_get_buffer(f, page_buffer, TARGET_PAGE_SIZE);

            } else {

                

                qemu_get_buffer_in_place(f, (uint8_t **)&place_source,

                                         TARGET_PAGE_SIZE);

            }

            break;

        case RAM_SAVE_FLAG_EOS:

            

            break;

        default:

            error_report("Unknown combination of migration flags: %#x"

                         " (postcopy mode)", flags);

            ret = -EINVAL;

        }



        if (place_needed) {

            

            if (all_zero) {

                ret = postcopy_place_page_zero(mis,

                                               host + TARGET_PAGE_SIZE -

                                               qemu_host_page_size);

            } else {

                ret = postcopy_place_page(mis, host + TARGET_PAGE_SIZE -

                                               qemu_host_page_size,

                                               place_source);

            }

        }

        if (!ret) {

            ret = qemu_file_get_error(f);

        }

    }



    return ret;

}
static void load_linux(FWCfgState *fw_cfg,

                       const char *kernel_filename,

                       const char *initrd_filename,

                       const char *kernel_cmdline,

                       hwaddr max_ram_size)

{

    uint16_t protocol;

    int setup_size, kernel_size, initrd_size = 0, cmdline_size;

    uint32_t initrd_max;

    uint8_t header[8192], *setup, *kernel, *initrd_data;

    hwaddr real_addr, prot_addr, cmdline_addr, initrd_addr = 0;

    FILE *f;

    char *vmode;



    

    cmdline_size = (strlen(kernel_cmdline)+16) & ~15;



    

    f = fopen(kernel_filename, "rb");

    if (!f || !(kernel_size = get_file_size(f)) ||

        fread(header, 1, MIN(ARRAY_SIZE(header), kernel_size), f) !=

        MIN(ARRAY_SIZE(header), kernel_size)) {

        fprintf(stderr, "qemu: could not load kernel '%s': %s\n",

                kernel_filename, strerror(errno));

        exit(1);

    }



    

#if 0

    fprintf(stderr, "header magic: %#x\n", ldl_p(header+0x202));

#endif

    if (ldl_p(header+0x202) == 0x53726448) {

        protocol = lduw_p(header+0x206);

    } else {

        

        if (load_multiboot(fw_cfg, f, kernel_filename, initrd_filename,

                           kernel_cmdline, kernel_size, header)) {

            return;

        }

        protocol = 0;

    }



    if (protocol < 0x200 || !(header[0x211] & 0x01)) {

        

        real_addr    = 0x90000;

        cmdline_addr = 0x9a000 - cmdline_size;

        prot_addr    = 0x10000;

    } else if (protocol < 0x202) {

        

        real_addr    = 0x90000;

        cmdline_addr = 0x9a000 - cmdline_size;

        prot_addr    = 0x100000;

    } else {

        

        real_addr    = 0x10000;

        cmdline_addr = 0x20000;

        prot_addr    = 0x100000;

    }



#if 0

    fprintf(stderr,

            "qemu: real_addr     = 0x" TARGET_FMT_plx "\n"

            "qemu: cmdline_addr  = 0x" TARGET_FMT_plx "\n"

            "qemu: prot_addr     = 0x" TARGET_FMT_plx "\n",

            real_addr,

            cmdline_addr,

            prot_addr);

#endif



    

    if (protocol >= 0x203) {

        initrd_max = ldl_p(header+0x22c);

    } else {

        initrd_max = 0x37ffffff;

    }



    if (initrd_max >= max_ram_size-ACPI_DATA_SIZE)

    	initrd_max = max_ram_size-ACPI_DATA_SIZE-1;



    fw_cfg_add_i32(fw_cfg, FW_CFG_CMDLINE_ADDR, cmdline_addr);

    fw_cfg_add_i32(fw_cfg, FW_CFG_CMDLINE_SIZE, strlen(kernel_cmdline)+1);

    fw_cfg_add_string(fw_cfg, FW_CFG_CMDLINE_DATA, kernel_cmdline);



    if (protocol >= 0x202) {

        stl_p(header+0x228, cmdline_addr);

    } else {

        stw_p(header+0x20, 0xA33F);

        stw_p(header+0x22, cmdline_addr-real_addr);

    }



    

    vmode = strstr(kernel_cmdline, "vga=");

    if (vmode) {

        unsigned int video_mode;

        

        vmode += 4;

        if (!strncmp(vmode, "normal", 6)) {

            video_mode = 0xffff;

        } else if (!strncmp(vmode, "ext", 3)) {

            video_mode = 0xfffe;

        } else if (!strncmp(vmode, "ask", 3)) {

            video_mode = 0xfffd;

        } else {

            video_mode = strtol(vmode, NULL, 0);

        }

        stw_p(header+0x1fa, video_mode);

    }



    

    

    if (protocol >= 0x200) {

        header[0x210] = 0xB0;

    }

    

    if (protocol >= 0x201) {

        header[0x211] |= 0x80;	

        stw_p(header+0x224, cmdline_addr-real_addr-0x200);

    }



    

    if (initrd_filename) {

        if (protocol < 0x200) {

            fprintf(stderr, "qemu: linux kernel too old to load a ram disk\n");

            exit(1);

        }



        initrd_size = get_image_size(initrd_filename);

        if (initrd_size < 0) {

            fprintf(stderr, "qemu: error reading initrd %s: %s\n",

                    initrd_filename, strerror(errno));

            exit(1);

        }



        initrd_addr = (initrd_max-initrd_size) & ~4095;



        initrd_data = g_malloc(initrd_size);

        load_image(initrd_filename, initrd_data);



        fw_cfg_add_i32(fw_cfg, FW_CFG_INITRD_ADDR, initrd_addr);

        fw_cfg_add_i32(fw_cfg, FW_CFG_INITRD_SIZE, initrd_size);

        fw_cfg_add_bytes(fw_cfg, FW_CFG_INITRD_DATA, initrd_data, initrd_size);



        stl_p(header+0x218, initrd_addr);

        stl_p(header+0x21c, initrd_size);

    }



    

    setup_size = header[0x1f1];

    if (setup_size == 0) {

        setup_size = 4;

    }

    setup_size = (setup_size+1)*512;

    kernel_size -= setup_size;



    setup  = g_malloc(setup_size);

    kernel = g_malloc(kernel_size);

    fseek(f, 0, SEEK_SET);

    if (fread(setup, 1, setup_size, f) != setup_size) {

        fprintf(stderr, "fread() failed\n");

        exit(1);

    }

    if (fread(kernel, 1, kernel_size, f) != kernel_size) {

        fprintf(stderr, "fread() failed\n");

        exit(1);

    }

    fclose(f);

    memcpy(setup, header, MIN(sizeof(header), setup_size));



    fw_cfg_add_i32(fw_cfg, FW_CFG_KERNEL_ADDR, prot_addr);

    fw_cfg_add_i32(fw_cfg, FW_CFG_KERNEL_SIZE, kernel_size);

    fw_cfg_add_bytes(fw_cfg, FW_CFG_KERNEL_DATA, kernel, kernel_size);



    fw_cfg_add_i32(fw_cfg, FW_CFG_SETUP_ADDR, real_addr);

    fw_cfg_add_i32(fw_cfg, FW_CFG_SETUP_SIZE, setup_size);

    fw_cfg_add_bytes(fw_cfg, FW_CFG_SETUP_DATA, setup, setup_size);



    option_rom[nb_option_roms].name = "linuxboot.bin";

    option_rom[nb_option_roms].bootindex = 0;

    nb_option_roms++;

}
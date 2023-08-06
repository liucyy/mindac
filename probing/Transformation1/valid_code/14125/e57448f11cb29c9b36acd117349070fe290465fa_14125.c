static void init_proc_970GX (CPUPPCState *env)

{

    gen_spr_ne_601(env);

    gen_spr_7xx(env);

    

    gen_tbl(env);

    

    

    spr_register(env, SPR_HID0, "HID0",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_clear,

                 0x60000000);

    

    spr_register(env, SPR_HID1, "HID1",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    spr_register(env, SPR_750_HID2, "HID2",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    spr_register(env, SPR_970_HID5, "HID5",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    

    gen_low_BATs(env);

#if 0 

    env->slb_nr = 32;

#endif

    init_excp_970(env);

    env->dcache_line_size = 128;

    env->icache_line_size = 128;

    

    ppc970_irq_init(env);

#if !defined(CONFIG_USER_ONLY)

    

    env->hreset_vector = 0x0000000000000100ULL;

#endif

}
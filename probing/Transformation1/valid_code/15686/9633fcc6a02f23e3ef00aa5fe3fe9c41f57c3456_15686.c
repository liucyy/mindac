static void init_proc_755 (CPUPPCState *env)

{

    gen_spr_ne_601(env);

    gen_spr_7xx(env);

    gen_spr_G2_755(env);

    

    gen_tbl(env);

    

    

    spr_register(env, SPR_L2CR, "L2CR",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, NULL,

                 0x00000000);

    

    spr_register(env, SPR_L2PMCR, "L2PMCR",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    gen_spr_thrm(env);

    

    

    spr_register(env, SPR_HID0, "HID0",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    spr_register(env, SPR_HID1, "HID1",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    spr_register(env, SPR_HID2, "HID2",

                 SPR_NOACCESS, SPR_NOACCESS,

                 &spr_read_generic, &spr_write_generic,

                 0x00000000);

    

    gen_low_BATs(env);

    gen_high_BATs(env);

    gen_6xx_7xx_soft_tlb(env, 64, 2);

    init_excp_7x5(env);

    env->dcache_line_size = 32;

    env->icache_line_size = 32;

    

    ppc6xx_irq_init(env);

}
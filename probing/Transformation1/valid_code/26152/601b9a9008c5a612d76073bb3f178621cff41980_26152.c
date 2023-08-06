uint32_t HELPER(sigp)(CPUS390XState *env, uint64_t order_code, uint32_t r1,

                      uint64_t cpu_addr)

{

    int cc = SIGP_CC_ORDER_CODE_ACCEPTED;



    HELPER_LOG("%s: %016" PRIx64 " %08x %016" PRIx64 "\n",

               __func__, order_code, r1, cpu_addr);



    



    switch (order_code) {

    case SIGP_SET_ARCH:

        

        break;

    case SIGP_SENSE:

        

        if (cpu_addr) {

            

            return 3;

        }

        env->regs[r1] &= 0xffffffff00000000ULL;

        cc = 1;

        break;

#if !defined(CONFIG_USER_ONLY)

    case SIGP_RESTART:

        qemu_system_reset_request();

        cpu_loop_exit(CPU(s390_env_get_cpu(env)));

        break;

    case SIGP_STOP:

        qemu_system_shutdown_request();

        cpu_loop_exit(CPU(s390_env_get_cpu(env)));

        break;

#endif

    default:

        

        fprintf(stderr, "XXX unknown sigp: 0x%" PRIx64 "\n", order_code);

        cc = SIGP_CC_NOT_OPERATIONAL;

    }



    return cc;

}
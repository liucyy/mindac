static int kvmppc_get_books_sregs(PowerPCCPU *cpu)

{

    CPUPPCState *env = &cpu->env;

    struct kvm_sregs sregs;

    int ret;

    int i;



    ret = kvm_vcpu_ioctl(CPU(cpu), KVM_GET_SREGS, &sregs);

    if (ret < 0) {

        return ret;

    }



    if (!env->external_htab) {

        ppc_store_sdr1(env, sregs.u.s.sdr1);

    }



    

#ifdef TARGET_PPC64

    

    memset(env->slb, 0, sizeof(env->slb));

    for (i = 0; i < ARRAY_SIZE(env->slb); i++) {

        target_ulong rb = sregs.u.s.ppc64.slb[i].slbe;

        target_ulong rs = sregs.u.s.ppc64.slb[i].slbv;

        

        if (rb & SLB_ESID_V) {

            ppc_store_slb(cpu, rb & 0xfff, rb & ~0xfffULL, rs);

        }

    }

#endif



    

    for (i = 0; i < 16; i++) {

        env->sr[i] = sregs.u.s.ppc32.sr[i];

    }



    

    for (i = 0; i < 8; i++) {

        env->DBAT[0][i] = sregs.u.s.ppc32.dbat[i] & 0xffffffff;

        env->DBAT[1][i] = sregs.u.s.ppc32.dbat[i] >> 32;

        env->IBAT[0][i] = sregs.u.s.ppc32.ibat[i] & 0xffffffff;

        env->IBAT[1][i] = sregs.u.s.ppc32.ibat[i] >> 32;

    }



    return 0;

}
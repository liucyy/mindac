static void rtas_start_cpu(PowerPCCPU *cpu_, sPAPRMachineState *spapr,
                           uint32_t token, uint32_t nargs,
                           target_ulong args,
                           uint32_t nret, target_ulong rets)
{
    target_ulong id, start, r3;
    PowerPCCPU *cpu;
    if (nargs != 3 || nret != 1) {
        rtas_st(rets, 0, RTAS_OUT_PARAM_ERROR);
        return;
    }
    id = rtas_ld(args, 0);
    start = rtas_ld(args, 1);
    r3 = rtas_ld(args, 2);
    cpu = spapr_find_cpu(id);
    if (cpu != NULL) {
        CPUState *cs = CPU(cpu);
        CPUPPCState *env = &cpu->env;
        PowerPCCPUClass *pcc = POWERPC_CPU_GET_CLASS(cpu);
        if (!cs->halted) {
            rtas_st(rets, 0, RTAS_OUT_HW_ERROR);
            return;
        }
        
        kvm_cpu_synchronize_state(cs);
        env->msr = (1ULL << MSR_SF) | (1ULL << MSR_ME);
        env->nip = start;
        env->gpr[3] = r3;
        cs->halted = 0;
        spapr_cpu_set_endianness(cpu);
        spapr_cpu_update_tb_offset(cpu);
        qemu_cpu_kick(cs);
        rtas_st(rets, 0, RTAS_OUT_SUCCESS);
        return;
    }
    
    rtas_st(rets, 0, RTAS_OUT_PARAM_ERROR);
}
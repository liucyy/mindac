static void arm_cpu_reset(CPUState *s)

{

    ARMCPU *cpu = ARM_CPU(s);

    ARMCPUClass *acc = ARM_CPU_GET_CLASS(cpu);

    CPUARMState *env = &cpu->env;



    acc->parent_reset(s);



    memset(env, 0, offsetof(CPUARMState, end_reset_fields));



    g_hash_table_foreach(cpu->cp_regs, cp_reg_reset, cpu);

    g_hash_table_foreach(cpu->cp_regs, cp_reg_check_reset, cpu);



    env->vfp.xregs[ARM_VFP_FPSID] = cpu->reset_fpsid;

    env->vfp.xregs[ARM_VFP_MVFR0] = cpu->mvfr0;

    env->vfp.xregs[ARM_VFP_MVFR1] = cpu->mvfr1;

    env->vfp.xregs[ARM_VFP_MVFR2] = cpu->mvfr2;



    cpu->power_state = cpu->start_powered_off ? PSCI_OFF : PSCI_ON;

    s->halted = cpu->start_powered_off;



    if (arm_feature(env, ARM_FEATURE_IWMMXT)) {

        env->iwmmxt.cregs[ARM_IWMMXT_wCID] = 0x69051000 | 'Q';

    }



    if (arm_feature(env, ARM_FEATURE_AARCH64)) {

        

        env->aarch64 = 1;

#if defined(CONFIG_USER_ONLY)

        env->pstate = PSTATE_MODE_EL0t;

        

        env->cp15.sctlr_el[1] |= SCTLR_UCT | SCTLR_UCI | SCTLR_DZE;

        

        env->cp15.cpacr_el1 = deposit64(env->cp15.cpacr_el1, 20, 2, 3);

#else

        

        if (arm_feature(env, ARM_FEATURE_EL3)) {

            env->pstate = PSTATE_MODE_EL3h;

        } else if (arm_feature(env, ARM_FEATURE_EL2)) {

            env->pstate = PSTATE_MODE_EL2h;

        } else {

            env->pstate = PSTATE_MODE_EL1h;

        }

        env->pc = cpu->rvbar;

#endif

    } else {

#if defined(CONFIG_USER_ONLY)

        

        env->cp15.cpacr_el1 = deposit64(env->cp15.cpacr_el1, 20, 4, 0xf);

#endif

    }



#if defined(CONFIG_USER_ONLY)

    env->uncached_cpsr = ARM_CPU_MODE_USR;

    

    env->vfp.xregs[ARM_VFP_FPEXC] = 1 << 30;

    if (arm_feature(env, ARM_FEATURE_IWMMXT)) {

        env->cp15.c15_cpar = 3;

    } else if (arm_feature(env, ARM_FEATURE_XSCALE)) {

        env->cp15.c15_cpar = 1;

    }

#else

    

    env->uncached_cpsr = ARM_CPU_MODE_SVC;

    env->daif = PSTATE_D | PSTATE_A | PSTATE_I | PSTATE_F;



    if (arm_feature(env, ARM_FEATURE_M)) {

        uint32_t initial_msp; 

        uint32_t initial_pc; 

        uint8_t *rom;



        if (arm_feature(env, ARM_FEATURE_M_SECURITY)) {

            env->v7m.secure = true;

        }



        

        env->v7m.ccr = R_V7M_CCR_STKALIGN_MASK;



        

        env->regs[14] = 0xffffffff;



        

        rom = rom_ptr(0);

        if (rom) {

            

            initial_msp = ldl_p(rom);

            initial_pc = ldl_p(rom + 4);

        } else {

            

            initial_msp = ldl_phys(s->as, 0);

            initial_pc = ldl_phys(s->as, 4);

        }



        env->regs[13] = initial_msp & 0xFFFFFFFC;

        env->regs[15] = initial_pc & ~1;

        env->thumb = initial_pc & 1;

    }



    

    if (A32_BANKED_CURRENT_REG_GET(env, sctlr) & SCTLR_V) {

        env->regs[15] = 0xFFFF0000;

    }



    env->vfp.xregs[ARM_VFP_FPEXC] = 0;

#endif



    if (arm_feature(env, ARM_FEATURE_PMSA)) {

        if (cpu->pmsav7_dregion > 0) {

            if (arm_feature(env, ARM_FEATURE_V8)) {

                memset(env->pmsav8.rbar[M_REG_NS], 0,

                       sizeof(*env->pmsav8.rbar[M_REG_NS])

                       * cpu->pmsav7_dregion);

                memset(env->pmsav8.rlar[M_REG_NS], 0,

                       sizeof(*env->pmsav8.rlar[M_REG_NS])

                       * cpu->pmsav7_dregion);

                if (arm_feature(env, ARM_FEATURE_M_SECURITY)) {

                    memset(env->pmsav8.rbar[M_REG_S], 0,

                           sizeof(*env->pmsav8.rbar[M_REG_S])

                           * cpu->pmsav7_dregion);

                    memset(env->pmsav8.rlar[M_REG_S], 0,

                           sizeof(*env->pmsav8.rlar[M_REG_S])

                           * cpu->pmsav7_dregion);

                }

            } else if (arm_feature(env, ARM_FEATURE_V7)) {

                memset(env->pmsav7.drbar, 0,

                       sizeof(*env->pmsav7.drbar) * cpu->pmsav7_dregion);

                memset(env->pmsav7.drsr, 0,

                       sizeof(*env->pmsav7.drsr) * cpu->pmsav7_dregion);

                memset(env->pmsav7.dracr, 0,

                       sizeof(*env->pmsav7.dracr) * cpu->pmsav7_dregion);

            }

        }

        env->pmsav7.rnr[M_REG_NS] = 0;

        env->pmsav7.rnr[M_REG_S] = 0;

        env->pmsav8.mair0[M_REG_NS] = 0;

        env->pmsav8.mair0[M_REG_S] = 0;

        env->pmsav8.mair1[M_REG_NS] = 0;

        env->pmsav8.mair1[M_REG_S] = 0;

    }



    set_flush_to_zero(1, &env->vfp.standard_fp_status);

    set_flush_inputs_to_zero(1, &env->vfp.standard_fp_status);

    set_default_nan_mode(1, &env->vfp.standard_fp_status);

    set_float_detect_tininess(float_tininess_before_rounding,

                              &env->vfp.fp_status);

    set_float_detect_tininess(float_tininess_before_rounding,

                              &env->vfp.standard_fp_status);

#ifndef CONFIG_USER_ONLY

    if (kvm_enabled()) {

        kvm_arm_reset_vcpu(cpu);

    }

#endif



    hw_breakpoint_update_all(cpu);

    hw_watchpoint_update_all(cpu);

}
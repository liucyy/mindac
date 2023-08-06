static void do_fbranch(DisasContext *dc, int32_t offset, uint32_t insn, int cc,

                      TCGv r_cond)

{

    unsigned int cond = GET_FIELD(insn, 3, 6), a = (insn & (1 << 29));

    target_ulong target = dc->pc + offset;



    if (cond == 0x0) {

        

        if (a) {

            dc->pc = dc->npc + 4;

            dc->npc = dc->pc + 4;

        } else {

            dc->pc = dc->npc;

            dc->npc = dc->pc + 4;

        }

    } else if (cond == 0x8) {

        

        if (a) {

            dc->pc = target;

            dc->npc = dc->pc + 4;

        } else {

            dc->pc = dc->npc;

            dc->npc = target;

            tcg_gen_mov_tl(cpu_pc, cpu_npc);

        }

    } else {

        flush_cond(dc, r_cond);

        gen_fcond(r_cond, cc, cond);

        if (a) {

            gen_branch_a(dc, target, dc->npc, r_cond);

            dc->is_br = 1;

        } else {

            dc->pc = dc->npc;

            dc->jump_pc[0] = target;

            dc->jump_pc[1] = dc->npc + 4;

            dc->npc = JUMP_PC;

        }

    }

}
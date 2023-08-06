static int read_block(ALSDecContext *ctx, ALSBlockData *bd)

{

    GetBitContext *gb        = &ctx->gb;



    *bd->shift_lsbs = 0;

    

    if (get_bits1(gb)) {

        if (read_var_block_data(ctx, bd))

            return -1;

    } else {

        read_const_block_data(ctx, bd);

    }



    return 0;

}
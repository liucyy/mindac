static int decode_profile_tier_level(GetBitContext *gb, AVCodecContext *avctx,

                                      PTLCommon *ptl)

{

    int i;



    if (get_bits_left(gb) < 2+1+5 + 32 + 4 + 16 + 16 + 12)

        return -1;



    ptl->profile_space = get_bits(gb, 2);

    ptl->tier_flag     = get_bits1(gb);

    ptl->profile_idc   = get_bits(gb, 5);

    if (ptl->profile_idc == FF_PROFILE_HEVC_MAIN)

        av_log(avctx, AV_LOG_DEBUG, "Main profile bitstream\n");

    else if (ptl->profile_idc == FF_PROFILE_HEVC_MAIN_10)

        av_log(avctx, AV_LOG_DEBUG, "Main 10 profile bitstream\n");

    else if (ptl->profile_idc == FF_PROFILE_HEVC_MAIN_STILL_PICTURE)

        av_log(avctx, AV_LOG_DEBUG, "Main Still Picture profile bitstream\n");

    else if (ptl->profile_idc == FF_PROFILE_HEVC_REXT)

        av_log(avctx, AV_LOG_DEBUG, "Range Extension profile bitstream\n");

    else

        av_log(avctx, AV_LOG_WARNING, "Unknown HEVC profile: %d\n", ptl->profile_idc);



    for (i = 0; i < 32; i++)

        ptl->profile_compatibility_flag[i] = get_bits1(gb);

    ptl->progressive_source_flag    = get_bits1(gb);

    ptl->interlaced_source_flag     = get_bits1(gb);

    ptl->non_packed_constraint_flag = get_bits1(gb);

    ptl->frame_only_constraint_flag = get_bits1(gb);



    skip_bits(gb, 16); 

    skip_bits(gb, 16); 

    skip_bits(gb, 12); 



    return 0;

}
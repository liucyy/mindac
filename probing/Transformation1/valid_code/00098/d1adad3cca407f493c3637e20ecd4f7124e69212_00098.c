static inline void RENAME(yuvPlanartouyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,

                                           long width, long height,

                                           long lumStride, long chromStride, long dstStride, long vertLumPerChroma)

{

    long y;

    const x86_reg chromWidth= width>>1;

    for (y=0; y<height; y++) {

#if COMPILE_TEMPLATE_MMX

        

        __asm__ volatile(

            "xor                %%"REG_a", %%"REG_a"    \n\t"

            ".p2align                   4               \n\t"

            "1:                                         \n\t"

            PREFETCH"   32(%1, %%"REG_a", 2)            \n\t"

            PREFETCH"   32(%2, %%"REG_a")               \n\t"

            PREFETCH"   32(%3, %%"REG_a")               \n\t"

            "movq         (%2, %%"REG_a"), %%mm0        \n\t" 

            "movq                   %%mm0, %%mm2        \n\t" 

            "movq         (%3, %%"REG_a"), %%mm1        \n\t" 

            "punpcklbw              %%mm1, %%mm0        \n\t" 

            "punpckhbw              %%mm1, %%mm2        \n\t" 



            "movq       (%1, %%"REG_a",2), %%mm3        \n\t" 

            "movq      8(%1, %%"REG_a",2), %%mm5        \n\t" 

            "movq                   %%mm0, %%mm4        \n\t" 

            "movq                   %%mm2, %%mm6        \n\t" 

            "punpcklbw              %%mm3, %%mm0        \n\t" 

            "punpckhbw              %%mm3, %%mm4        \n\t" 

            "punpcklbw              %%mm5, %%mm2        \n\t" 

            "punpckhbw              %%mm5, %%mm6        \n\t" 



            MOVNTQ"                 %%mm0,   (%0, %%"REG_a", 4)     \n\t"

            MOVNTQ"                 %%mm4,  8(%0, %%"REG_a", 4)     \n\t"

            MOVNTQ"                 %%mm2, 16(%0, %%"REG_a", 4)     \n\t"

            MOVNTQ"                 %%mm6, 24(%0, %%"REG_a", 4)     \n\t"



            "add                       $8, %%"REG_a"    \n\t"

            "cmp                       %4, %%"REG_a"    \n\t"

            " jb                       1b               \n\t"

            ::"r"(dst), "r"(ysrc), "r"(usrc), "r"(vsrc), "g" (chromWidth)

            : "%"REG_a

        );

#else





#if HAVE_FAST_64BIT

        int i;

        uint64_t *ldst = (uint64_t *) dst;

        const uint8_t *yc = ysrc, *uc = usrc, *vc = vsrc;

        for (i = 0; i < chromWidth; i += 2) {

            uint64_t k, l;

            k = uc[0] + (yc[0] << 8) +

                (vc[0] << 16) + (yc[1] << 24);

            l = uc[1] + (yc[2] << 8) +

                (vc[1] << 16) + (yc[3] << 24);

            *ldst++ = k + (l << 32);

            yc += 4;

            uc += 2;

            vc += 2;

        }



#else

        int i, *idst = (int32_t *) dst;

        const uint8_t *yc = ysrc, *uc = usrc, *vc = vsrc;

        for (i = 0; i < chromWidth; i++) {

#if HAVE_BIGENDIAN

            *idst++ = (uc[0] << 24)+ (yc[0] << 16) +

                (vc[0] << 8) + (yc[1] << 0);

#else

            *idst++ = uc[0] + (yc[0] << 8) +

               (vc[0] << 16) + (yc[1] << 24);

#endif

            yc += 2;

            uc++;

            vc++;

        }

#endif

#endif

        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1) {

            usrc += chromStride;

            vsrc += chromStride;

        }

        ysrc += lumStride;

        dst += dstStride;

    }

#if COMPILE_TEMPLATE_MMX

    __asm__(EMMS"       \n\t"

            SFENCE"     \n\t"

            :::"memory");

#endif

}
static inline void RENAME(yvu9_to_yuy2)(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,

                                        uint8_t *dst,

                                        long width, long height,

                                        long srcStride1, long srcStride2,

                                        long srcStride3, long dstStride)

{

    x86_reg x;

    long y,w,h;

    w=width/2; h=height;

    for (y=0;y<h;y++) {

        const uint8_t* yp=src1+srcStride1*y;

        const uint8_t* up=src2+srcStride2*(y>>2);

        const uint8_t* vp=src3+srcStride3*(y>>2);

        uint8_t* d=dst+dstStride*y;

        x=0;

#if COMPILE_TEMPLATE_MMX

        for (;x<w-7;x+=8) {

            __asm__ volatile(

                PREFETCH"   32(%1, %0)          \n\t"

                PREFETCH"   32(%2, %0)          \n\t"

                PREFETCH"   32(%3, %0)          \n\t"

                "movq      (%1, %0, 4), %%mm0   \n\t" 

                "movq         (%2, %0), %%mm1   \n\t" 

                "movq         (%3, %0), %%mm2   \n\t" 

                "movq            %%mm0, %%mm3   \n\t" 

                "movq            %%mm1, %%mm4   \n\t" 

                "movq            %%mm2, %%mm5   \n\t" 

                "punpcklbw       %%mm1, %%mm1   \n\t" 

                "punpcklbw       %%mm2, %%mm2   \n\t" 

                "punpckhbw       %%mm4, %%mm4   \n\t" 

                "punpckhbw       %%mm5, %%mm5   \n\t" 



                "movq            %%mm1, %%mm6   \n\t"

                "punpcklbw       %%mm2, %%mm1   \n\t" 

                "punpcklbw       %%mm1, %%mm0   \n\t" 

                "punpckhbw       %%mm1, %%mm3   \n\t" 

                MOVNTQ"          %%mm0,  (%4, %0, 8)    \n\t"

                MOVNTQ"          %%mm3, 8(%4, %0, 8)    \n\t"



                "punpckhbw       %%mm2, %%mm6   \n\t" 

                "movq     8(%1, %0, 4), %%mm0   \n\t"

                "movq            %%mm0, %%mm3   \n\t"

                "punpcklbw       %%mm6, %%mm0   \n\t" 

                "punpckhbw       %%mm6, %%mm3   \n\t" 

                MOVNTQ"          %%mm0, 16(%4, %0, 8)   \n\t"

                MOVNTQ"          %%mm3, 24(%4, %0, 8)   \n\t"



                "movq            %%mm4, %%mm6   \n\t"

                "movq    16(%1, %0, 4), %%mm0   \n\t"

                "movq            %%mm0, %%mm3   \n\t"

                "punpcklbw       %%mm5, %%mm4   \n\t"

                "punpcklbw       %%mm4, %%mm0   \n\t" 

                "punpckhbw       %%mm4, %%mm3   \n\t" 

                MOVNTQ"          %%mm0, 32(%4, %0, 8)   \n\t"

                MOVNTQ"          %%mm3, 40(%4, %0, 8)   \n\t"



                "punpckhbw       %%mm5, %%mm6   \n\t"

                "movq    24(%1, %0, 4), %%mm0   \n\t"

                "movq            %%mm0, %%mm3   \n\t"

                "punpcklbw       %%mm6, %%mm0   \n\t" 

                "punpckhbw       %%mm6, %%mm3   \n\t" 

                MOVNTQ"          %%mm0, 48(%4, %0, 8)   \n\t"

                MOVNTQ"          %%mm3, 56(%4, %0, 8)   \n\t"



                : "+r" (x)

                : "r"(yp), "r" (up), "r"(vp), "r"(d)

                :"memory");

        }

#endif

        for (; x<w; x++) {

            const long x2 = x<<2;

            d[8*x+0] = yp[x2];

            d[8*x+1] = up[x];

            d[8*x+2] = yp[x2+1];

            d[8*x+3] = vp[x];

            d[8*x+4] = yp[x2+2];

            d[8*x+5] = up[x];

            d[8*x+6] = yp[x2+3];

            d[8*x+7] = vp[x];

        }

    }

#if COMPILE_TEMPLATE_MMX

    __asm__(

            EMMS"       \n\t"

            SFENCE"     \n\t"

            ::: "memory"

        );

#endif

}
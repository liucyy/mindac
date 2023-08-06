int ff_get_cpu_flags_x86(void)

{

    int rval = 0;



#ifdef cpuid



    int eax, ebx, ecx, edx;

    int max_std_level, max_ext_level, std_caps = 0, ext_caps = 0;

    int family = 0, model = 0;

    union { int i[3]; char c[12]; } vendor;



    if (!cpuid_test())

        return 0; 



    cpuid(0, max_std_level, vendor.i[0], vendor.i[2], vendor.i[1]);



    if (max_std_level >= 1) {

        cpuid(1, eax, ebx, ecx, std_caps);

        family = ((eax >> 8) & 0xf) + ((eax >> 20) & 0xff);

        model  = ((eax >> 4) & 0xf) + ((eax >> 12) & 0xf0);

        if (std_caps & (1 << 15))

            rval |= AV_CPU_FLAG_CMOV;

        if (std_caps & (1 << 23))

            rval |= AV_CPU_FLAG_MMX;

        if (std_caps & (1 << 25))

            rval |= AV_CPU_FLAG_MMXEXT;

#if HAVE_SSE

        if (std_caps & (1 << 25))

            rval |= AV_CPU_FLAG_SSE;

        if (std_caps & (1 << 26))

            rval |= AV_CPU_FLAG_SSE2;

        if (ecx & 1)

            rval |= AV_CPU_FLAG_SSE3;

        if (ecx & 0x00000200 )

            rval |= AV_CPU_FLAG_SSSE3;

        if (ecx & 0x00080000 )

            rval |= AV_CPU_FLAG_SSE4;

        if (ecx & 0x00100000 )

            rval |= AV_CPU_FLAG_SSE42;

#if HAVE_AVX

        

        if ((ecx & 0x18000000) == 0x18000000) {

            

            xgetbv(0, eax, edx);

            if ((eax & 0x6) == 0x6) {

                rval |= AV_CPU_FLAG_AVX;

                if (ecx & 0x00001000)

                    rval |= AV_CPU_FLAG_FMA3;

            }

        }

#endif 

#endif 

    }

    if (max_std_level >= 7) {

        cpuid(7, eax, ebx, ecx, edx);

#if HAVE_AVX2

        if (ebx & 0x00000020)

            rval |= AV_CPU_FLAG_AVX2;

#endif 

        

        if (ebx & 0x00000008) {

            rval |= AV_CPU_FLAG_BMI1;

            if (ebx & 0x00000100)

                rval |= AV_CPU_FLAG_BMI2;

        }

    }



    cpuid(0x80000000, max_ext_level, ebx, ecx, edx);



    if (max_ext_level >= 0x80000001) {

        cpuid(0x80000001, eax, ebx, ecx, ext_caps);

        if (ext_caps & (1U << 31))

            rval |= AV_CPU_FLAG_3DNOW;

        if (ext_caps & (1 << 30))

            rval |= AV_CPU_FLAG_3DNOWEXT;

        if (ext_caps & (1 << 23))

            rval |= AV_CPU_FLAG_MMX;

        if (ext_caps & (1 << 22))

            rval |= AV_CPU_FLAG_MMXEXT;



        

        if (!strncmp(vendor.c, "AuthenticAMD", 12) &&

            rval & AV_CPU_FLAG_SSE2 && !(ecx & 0x00000040)) {

            rval |= AV_CPU_FLAG_SSE2SLOW;

        }



        

        if (rval & AV_CPU_FLAG_AVX) {

            if (ecx & 0x00000800)

                rval |= AV_CPU_FLAG_XOP;

            if (ecx & 0x00010000)

                rval |= AV_CPU_FLAG_FMA4;

        }

    }



    if (!strncmp(vendor.c, "GenuineIntel", 12)) {

        if (family == 6 && (model == 9 || model == 13 || model == 14)) {

            

            if (rval & AV_CPU_FLAG_SSE2)

                rval ^= AV_CPU_FLAG_SSE2SLOW | AV_CPU_FLAG_SSE2;

            if (rval & AV_CPU_FLAG_SSE3)

                rval ^= AV_CPU_FLAG_SSE3SLOW | AV_CPU_FLAG_SSE3;

        }

        

        if (family == 6 && model == 28)

            rval |= AV_CPU_FLAG_ATOM;

    }



#endif 



    return rval;

}
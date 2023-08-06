void syscall_init(void)

{

    IOCTLEntry *ie;

    const argtype *arg_type;

    int size;

    int i;



#define STRUCT(name, list...) thunk_register_struct(STRUCT_ ## name, #name, struct_ ## name ## _def);

#define STRUCT_SPECIAL(name) thunk_register_struct_direct(STRUCT_ ## name, #name, &struct_ ## name ## _def);

#include "syscall_types.h"

#undef STRUCT

#undef STRUCT_SPECIAL



    

    ie = ioctl_entries;

    while (ie->target_cmd != 0) {

        if (((ie->target_cmd >> TARGET_IOC_SIZESHIFT) & TARGET_IOC_SIZEMASK) ==

            TARGET_IOC_SIZEMASK) {

            arg_type = ie->arg_type;

            if (arg_type[0] != TYPE_PTR) {

                fprintf(stderr, "cannot patch size for ioctl 0x%x\n",

                        ie->target_cmd);

                exit(1);

            }

            arg_type++;

            size = thunk_type_size(arg_type, 0);

            ie->target_cmd = (ie->target_cmd &

                              ~(TARGET_IOC_SIZEMASK << TARGET_IOC_SIZESHIFT)) |

                (size << TARGET_IOC_SIZESHIFT);

        }



        

        for (i=0; i < ERRNO_TABLE_SIZE; i++)

                target_to_host_errno_table[host_to_target_errno_table[i]] = i;



        

#if defined(__i386__) && defined(TARGET_I386) && defined(TARGET_ABI32)

        if (ie->target_cmd != ie->host_cmd) {

            fprintf(stderr, "ERROR: ioctl: target=0x%x host=0x%x\n",

                    ie->target_cmd, ie->host_cmd);

        }

#endif

        ie++;

    }

}
static void bochs_bios_init(void)

{

    void *fw_cfg;

    uint8_t *smbios_table;

    size_t smbios_len;

    uint64_t *numa_fw_cfg;

    int i, j;



    register_ioport_write(0x400, 1, 2, bochs_bios_write, NULL);

    register_ioport_write(0x401, 1, 2, bochs_bios_write, NULL);

    register_ioport_write(0x402, 1, 1, bochs_bios_write, NULL);

    register_ioport_write(0x403, 1, 1, bochs_bios_write, NULL);

    register_ioport_write(0x8900, 1, 1, bochs_bios_write, NULL);



    register_ioport_write(0x501, 1, 2, bochs_bios_write, NULL);

    register_ioport_write(0x502, 1, 2, bochs_bios_write, NULL);

    register_ioport_write(0x500, 1, 1, bochs_bios_write, NULL);

    register_ioport_write(0x503, 1, 1, bochs_bios_write, NULL);



    fw_cfg = fw_cfg_init(BIOS_CFG_IOPORT, BIOS_CFG_IOPORT + 1, 0, 0);

    fw_cfg_add_i32(fw_cfg, FW_CFG_ID, 1);

    fw_cfg_add_i64(fw_cfg, FW_CFG_RAM_SIZE, (uint64_t)ram_size);

    fw_cfg_add_bytes(fw_cfg, FW_CFG_ACPI_TABLES, (uint8_t *)acpi_tables,

                     acpi_tables_len);



    smbios_table = smbios_get_table(&smbios_len);

    if (smbios_table)

        fw_cfg_add_bytes(fw_cfg, FW_CFG_SMBIOS_ENTRIES,

                         smbios_table, smbios_len);



    

    numa_fw_cfg = qemu_mallocz((1 + smp_cpus + nb_numa_nodes) * 8);

    numa_fw_cfg[0] = cpu_to_le64(nb_numa_nodes);

    for (i = 0; i < smp_cpus; i++) {

        for (j = 0; j < nb_numa_nodes; j++) {

            if (node_cpumask[j] & (1 << i)) {

                numa_fw_cfg[i + 1] = cpu_to_le64(j);

                break;

            }

        }

    }

    for (i = 0; i < nb_numa_nodes; i++) {

        numa_fw_cfg[smp_cpus + 1 + i] = cpu_to_le64(node_mem[i]);

    }

    fw_cfg_add_bytes(fw_cfg, FW_CFG_NUMA, (uint8_t *)numa_fw_cfg,

                     (1 + smp_cpus + nb_numa_nodes) * 8);

}
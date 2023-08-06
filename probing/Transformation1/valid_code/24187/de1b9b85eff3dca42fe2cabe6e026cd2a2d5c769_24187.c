static void qxl_init_ramsize(PCIQXLDevice *qxl)

{

    

    if (qxl->vgamem_size_mb < 8) {

        qxl->vgamem_size_mb = 8;

    }

    

    if (qxl->vgamem_size_mb > 256) {

        qxl->vgamem_size_mb = 256;

    }

    qxl->vgamem_size = qxl->vgamem_size_mb * 1024 * 1024;



    

    if (qxl->ram_size_mb != -1) {

        qxl->vga.vram_size = qxl->ram_size_mb * 1024 * 1024;

    }

    if (qxl->vga.vram_size < qxl->vgamem_size * 2) {

        qxl->vga.vram_size = qxl->vgamem_size * 2;

    }



    

    if (qxl->vram32_size_mb != -1) {

        qxl->vram32_size = qxl->vram32_size_mb * 1024 * 1024;

    }

    if (qxl->vram32_size < 4096) {

        qxl->vram32_size = 4096;

    }



    

    if (qxl->vram_size_mb != -1) {

        qxl->vram_size = qxl->vram_size_mb * 1024 * 1024;

    }

    if (qxl->vram_size < qxl->vram32_size) {

        qxl->vram_size = qxl->vram32_size;

    }



    if (qxl->revision == 1) {

        qxl->vram32_size = 4096;

        qxl->vram_size = 4096;

    }

    qxl->vgamem_size = pow2ceil(qxl->vgamem_size);

    qxl->vga.vram_size = pow2ceil(qxl->vga.vram_size);

    qxl->vram32_size = pow2ceil(qxl->vram32_size);

    qxl->vram_size = pow2ceil(qxl->vram_size);

}
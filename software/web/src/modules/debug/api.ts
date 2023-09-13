export interface state_fast {
    uptime: number;
    free_dram: number;
    free_iram: number;
    free_psram: number;
}

export interface state_slow {
    largest_free_dram_block: number;
    largest_free_psram_block: number;
    heap_dram: number;
    heap_iram: number;
    heap_psram: number;
}

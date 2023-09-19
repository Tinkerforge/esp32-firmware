export interface state_fast {
    uptime: number;
    free_dram: number;
    free_iram: number;
    free_psram: number;
    heap_check_time_avg: number;
    heap_check_time_max: number;
    cpu_usage: number;
}

export interface state_slow {
    largest_free_dram_block: number;
    largest_free_psram_block: number;
    heap_dram: number;
    heap_iram: number;
    heap_psram: number;
    psram_size: number;
    heap_integrity_ok: boolean;
    main_stack_hwm: number;
    flash_mode: string;
    flash_benchmark: number;
    psram_benchmark: number;
}

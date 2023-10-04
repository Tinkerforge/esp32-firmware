interface spi_state {
    clk: number;
    dummy_cycles: number;
    spi_mode: string;
}

export interface state_static {
    heap_dram: number;
    heap_iram: number;
    heap_psram: number;
    psram_size: number;
    cpu_clk: number;
    apb_clk: number;
    spi_buses: spi_state[];
    flash_mode: string;
    flash_benchmark: number;
    psram_benchmark: number;
}

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
    heap_integrity_ok: boolean;
}

interface task_hwm {
    task_name: string;
    hwm: number;
    stack_size: number;
}

export type state_hwm = task_hwm[];

interface Peer {
    ip: string;
    port: number;
    trusted: boolean;
    dns_name: string;
    id: string;
    wss_path: string;
    ski: string;
    autoregister: boolean;
    model_brand: string;
    model_model: string;
    model_type: string;
    state: number;
}

export interface addPeer {
    ski: string;
    trusted: boolean;
    ip: string;
    port: number;
    dns_name: string;
    wss_path: string;
}

export interface removePeer {
    ski: string;
}

export interface config {
    cert_id: number;
    key_id: number;
    peers: Peer[];
}

export interface state {
    ski: string;
    connections: number;
}

export interface scan {
}

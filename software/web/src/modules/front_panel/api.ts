// TODO: use enum generator.
const enum TileType {
    EmptyTile           = 0,
    Wallbox             = 1,
    ChargeManagement    = 2,
    Meter               = 3,
    DayAheadPrices      = 4,
    SolarForecast       = 5,
    EnergyManagerStatus = 6,
    HeatingStatus       = 7,
}

type TileTypeEmptyTile = [TileType.EmptyTile, null];
type TileTypeWallbox = [TileType.Wallbox, number];
type TileTypeChargeManagement = [TileType.ChargeManagement, null];
type TileTypeMeter = [TileType.Meter, number];
type TileTypeDayAheadPrices = [TileType.DayAheadPrices, number];
type TileTypeSolarForecast = [TileType.SolarForecast, number];
type TileTypeEnergyManagerStatus = [TileType.EnergyManagerStatus, null];
type TileTypeHeatingStatus = [TileType.HeatingStatus, null];

type TileConfig =
    TileTypeEmptyTile |
    TileTypeWallbox |
    TileTypeChargeManagement |
    TileTypeMeter |
    TileTypeDayAheadPrices |
    TileTypeSolarForecast |
    TileTypeEnergyManagerStatus |
    TileTypeHeatingStatus;

export interface config {
    enable: boolean;
    tiles: TileConfig[]
}

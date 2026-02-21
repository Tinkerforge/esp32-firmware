/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import { h, Fragment, Component, createRef } from "preact";
import { __ } from "../translation";
import * as util from "../util";

// Fixed temperature range endpoints
const TEMP_WARM = 20;  // °C
const TEMP_COLD = -10; // °C

// Endpoint identifiers for drag tracking
type EndpointId = "ext_warm" | "ext_cold" | "blk_warm" | "blk_cold";

export interface HeatingCurveChartProps {
    extended_hours_warm: number;
    extended_hours_cold: number;
    blocking_hours_warm: number;
    blocking_hours_cold: number;
    current_temperature?: number; // °C, null/undefined if unavailable
    show_extended: boolean;
    show_blocking: boolean;
    max_hours: number; // upper bound for dragging (control period hours)
    onExtendedHoursWarmChange?: (value: number) => void;
    onExtendedHoursColdChange?: (value: number) => void;
    onBlockingHoursWarmChange?: (value: number) => void;
    onBlockingHoursColdChange?: (value: number) => void;
}

interface HeatingCurveChartState {
    dragging: EndpointId | null;
    hovered: EndpointId | null;
}

export class HeatingCurveChart extends Component<HeatingCurveChartProps, HeatingCurveChartState> {
    // Chart layout constants
    static readonly MARGIN = { top: 20, right: 45, bottom: 40, left: 45 };
    static readonly VIEWBOX_WIDTH = 500;
    static readonly VIEWBOX_HEIGHT = 260;

    svgRef = createRef<SVGSVGElement>();

    constructor() {
        super();
        this.state = { dragging: null, hovered: null };
    }

    get plotWidth() {
        return HeatingCurveChart.VIEWBOX_WIDTH - HeatingCurveChart.MARGIN.left - HeatingCurveChart.MARGIN.right;
    }

    get plotHeight() {
        return HeatingCurveChart.VIEWBOX_HEIGHT - HeatingCurveChart.MARGIN.top - HeatingCurveChart.MARGIN.bottom;
    }

    // Map temperature (20 to -10) to x pixel coordinate
    tempToX(temp: number): number {
        const fraction = (TEMP_WARM - temp) / (TEMP_WARM - TEMP_COLD);
        return HeatingCurveChart.MARGIN.left + fraction * this.plotWidth;
    }

    // Map hours to y pixel coordinate (0 at bottom, yMax at top)
    hoursToY(hours: number, yMax: number): number {
        const fraction = hours / yMax;
        return HeatingCurveChart.MARGIN.top + this.plotHeight - fraction * this.plotHeight;
    }

    // Inverse: map y pixel coordinate back to hours
    yToHours(y: number, yMax: number): number {
        const fraction = (HeatingCurveChart.MARGIN.top + this.plotHeight - y) / this.plotHeight;
        return fraction * yMax;
    }

    // Convert a screen-space PointerEvent Y coordinate to SVG viewBox Y coordinate
    screenToSvgY(clientY: number): number {
        const svg = this.svgRef.current;
        if (!svg) return 0;
        const pt = svg.createSVGPoint();
        pt.x = 0;
        pt.y = clientY;
        const ctm = svg.getScreenCTM();
        if (!ctm) return 0;
        const svgPt = pt.matrixTransform(ctm.inverse());
        return svgPt.y;
    }

    // Linear interpolation of hours at a given temperature
    interpolateHours(hours_warm: number, hours_cold: number, temp: number): number {
        const fraction = (TEMP_WARM - temp) / (TEMP_WARM - TEMP_COLD);
        return hours_warm + fraction * (hours_cold - hours_warm);
    }

    // Compute a nice Y-axis maximum from the configured hour values
    getYMax(): number {
        const { extended_hours_warm, extended_hours_cold, blocking_hours_warm, blocking_hours_cold } = this.props;
        const dataMax = Math.max(extended_hours_warm, extended_hours_cold, blocking_hours_warm, blocking_hours_cold, 1);

        // Round up to a nice number for clean tick marks
        if (dataMax <= 4) return Math.ceil(dataMax);
        if (dataMax <= 8) return Math.ceil(dataMax / 2) * 2;
        return Math.ceil(dataMax / 4) * 4;
    }

    // Pointer event handlers for drag interaction
    onEndpointPointerDown = (endpoint: EndpointId, ev: PointerEvent) => {
        ev.preventDefault();
        ev.stopPropagation();
        (ev.target as Element).setPointerCapture(ev.pointerId);
        this.setState({ dragging: endpoint });
    }

    onPointerMove = (ev: PointerEvent) => {
        const { dragging } = this.state;
        if (!dragging) return;

        ev.preventDefault();
        const yMax = this.getYMax();
        const svgY = this.screenToSvgY(ev.clientY);
        let hours = this.yToHours(svgY, yMax);

        // Clamp to valid range and round to nearest integer
        hours = Math.round(Math.max(0, Math.min(this.props.max_hours, hours)));

        // Cross-validate: extended + blocking must not exceed max_hours at each endpoint
        const { max_hours, extended_hours_warm, extended_hours_cold, blocking_hours_warm, blocking_hours_cold,
                show_extended, show_blocking } = this.props;
        if (show_extended && show_blocking) {
            switch (dragging) {
                case "ext_warm":  hours = Math.min(hours, max_hours - blocking_hours_warm);  break;
                case "ext_cold":  hours = Math.min(hours, max_hours - blocking_hours_cold);  break;
                case "blk_warm":  hours = Math.min(hours, max_hours - extended_hours_warm);  break;
                case "blk_cold":  hours = Math.min(hours, max_hours - extended_hours_cold);  break;
            }
            hours = Math.max(0, hours);
        }

        // Call the appropriate callback
        switch (dragging) {
            case "ext_warm":
                if (this.props.onExtendedHoursWarmChange) this.props.onExtendedHoursWarmChange(hours);
                break;
            case "ext_cold":
                if (this.props.onExtendedHoursColdChange) this.props.onExtendedHoursColdChange(hours);
                break;
            case "blk_warm":
                if (this.props.onBlockingHoursWarmChange) this.props.onBlockingHoursWarmChange(hours);
                break;
            case "blk_cold":
                if (this.props.onBlockingHoursColdChange) this.props.onBlockingHoursColdChange(hours);
                break;
        }
    }

    onPointerUp = (ev: PointerEvent) => {
        if (this.state.dragging) {
            this.setState({ dragging: null });
        }
    }

    // Helper: is an endpoint active (being dragged or hovered)?
    isActive(id: EndpointId): boolean {
        return this.state.dragging === id || this.state.hovered === id;
    }

    // Helper: has any callback props (i.e. is interactive)
    get isInteractive(): boolean {
        return !!(this.props.onExtendedHoursWarmChange || this.props.onExtendedHoursColdChange
               || this.props.onBlockingHoursWarmChange || this.props.onBlockingHoursColdChange);
    }

    // Resolve label Y positions so that two labels on the same side don't overlap.
    // Each raw Y is the circle center; the label normally sits above (-8) or below (+16)
    // relative to the circle when near the top edge.
    // Returns [adjustedExtLabelY, adjustedBlkLabelY].
    resolveEndpointLabelYs(extCircleY: number, blkCircleY: number, bothVisible: boolean): [number, number] {
        const M = HeatingCurveChart.MARGIN;
        const minGap = 14; // minimum vertical distance between label baselines

        // Default label offset: above the circle, or below if too close to top
        const labelY = (cy: number) => cy + (cy < M.top + 20 ? 16 : -8);

        let extLY = labelY(extCircleY);
        let blkLY = labelY(blkCircleY);

        if (!bothVisible) return [extLY, blkLY];

        const gap = Math.abs(extLY - blkLY);
        if (gap < minGap) {
            // Push apart symmetrically around midpoint
            const mid = (extLY + blkLY) / 2;
            const halfGap = minGap / 2;
            if (extCircleY <= blkCircleY) {
                // ext is higher (smaller Y) -> its label goes up, blk goes down
                extLY = mid - halfGap;
                blkLY = mid + halfGap;
            } else {
                extLY = mid + halfGap;
                blkLY = mid - halfGap;
            }
        }
        return [extLY, blkLY];
    }

    render() {
        const { extended_hours_warm, extended_hours_cold, blocking_hours_warm, blocking_hours_cold,
                current_temperature, show_extended, show_blocking } = this.props;
        const { dragging, hovered } = this.state;
        const M = HeatingCurveChart.MARGIN;
        const VW = HeatingCurveChart.VIEWBOX_WIDTH;
        const VH = HeatingCurveChart.VIEWBOX_HEIGHT;
        const yMax = this.getYMax();
        const interactive = this.isInteractive;

        // Temperature tick marks (every 5°C from 20 to -10)
        const tempTicks: number[] = [];
        for (let t = TEMP_WARM; t >= TEMP_COLD; t -= 5) {
            tempTicks.push(t);
        }

        // Hour tick marks - choose step for ~4-6 ticks
        const hourTicks: number[] = [];
        let hourStep = 1;
        if (yMax > 8) hourStep = 2;
        if (yMax > 16) hourStep = 4;
        for (let hr = 0; hr <= yMax; hr += hourStep) {
            hourTicks.push(hr);
        }

        // Extended operation line (blue): warm->cold
        const extX1 = this.tempToX(TEMP_WARM);
        const extY1 = this.hoursToY(extended_hours_warm, yMax);
        const extX2 = this.tempToX(TEMP_COLD);
        const extY2 = this.hoursToY(extended_hours_cold, yMax);

        // Blocking operation line (red): warm->cold
        const blkX1 = this.tempToX(TEMP_WARM);
        const blkY1 = this.hoursToY(blocking_hours_warm, yMax);
        const blkX2 = this.tempToX(TEMP_COLD);
        const blkY2 = this.hoursToY(blocking_hours_cold, yMax);

        // Resolve endpoint label positions to avoid overlap when both lines are visible
        const bothVisible = show_extended && show_blocking;
        const [extWarmLabelY, blkWarmLabelY] = this.resolveEndpointLabelYs(extY1, blkY1, bothVisible);
        const [extColdLabelY, blkColdLabelY] = this.resolveEndpointLabelYs(extY2, blkY2, bothVisible);

        // Current temperature marker
        let currentTempX: number = null;
        let currentExtHours: number = null;
        let currentBlkHours: number = null;
        let currentExtLabelY: number = null;
        let currentBlkLabelY: number = null;
        if (current_temperature !== null && current_temperature !== undefined) {
            const clampedTemp = Math.max(TEMP_COLD, Math.min(TEMP_WARM, current_temperature));
            currentTempX = this.tempToX(clampedTemp);
            currentExtHours = this.interpolateHours(extended_hours_warm, extended_hours_cold, clampedTemp);
            currentBlkHours = this.interpolateHours(blocking_hours_warm, blocking_hours_cold, clampedTemp);

            // Resolve right-axis label positions to avoid overlap
            const extRY = this.hoursToY(currentExtHours, yMax) + 4;
            const blkRY = this.hoursToY(currentBlkHours, yMax) + 4;
            const minGap = 14;
            if (bothVisible && Math.abs(extRY - blkRY) < minGap) {
                const mid = (extRY + blkRY) / 2;
                const halfGap = minGap / 2;
                currentExtLabelY = extRY <= blkRY ? mid - halfGap : mid + halfGap;
                currentBlkLabelY = extRY <= blkRY ? mid + halfGap : mid - halfGap;
            } else {
                currentExtLabelY = extRY;
                currentBlkLabelY = blkRY;
            }
        }

        const textColor = "var(--bs-body-color)";
        const gridColor = "var(--bs-border-color)";
        const extColor = "#0d6efd"; // Bootstrap primary / blue
        const blkColor = "#dc3545"; // Bootstrap danger / red

        // Endpoint circle radius: larger when active (dragging/hovering)
        const baseR = 4;
        const activeR = 7;
        // Invisible hit-area radius for easier touch/click targeting
        const hitR = 14;

        // Cursor style for the SVG element
        const svgCursor = dragging ? "grabbing" : "default";

        return (
            <div class="card p-2">
                <svg
                    ref={this.svgRef}
                    viewBox={`0 0 ${VW} ${VH}`}
                    style={`width: 100%; height: auto; cursor: ${svgCursor}; touch-action: none;`}
                    xmlns="http://www.w3.org/2000/svg"
                    onPointerMove={interactive ? this.onPointerMove : undefined}
                    onPointerUp={interactive ? this.onPointerUp : undefined}
                    onPointerLeave={interactive ? this.onPointerUp : undefined}
                >
                    {/* Grid lines - horizontal (hours) */}
                    {hourTicks.map(hr => {
                        const y = this.hoursToY(hr, yMax);
                        return <line x1={M.left} y1={y} x2={VW - M.right} y2={y}
                                     stroke={gridColor} stroke-width="0.5" stroke-dasharray="3,3" />;
                    })}

                    {/* Grid lines - vertical (temperature) */}
                    {tempTicks.map(t => {
                        const x = this.tempToX(t);
                        return <line x1={x} y1={M.top} x2={x} y2={VH - M.bottom}
                                     stroke={gridColor} stroke-width="0.5" stroke-dasharray="3,3" />;
                    })}

                    {/* Axes */}
                    <line x1={M.left} y1={M.top} x2={M.left} y2={VH - M.bottom}
                          stroke={textColor} stroke-width="1" />
                    <line x1={M.left} y1={VH - M.bottom} x2={VW - M.right} y2={VH - M.bottom}
                          stroke={textColor} stroke-width="1" />

                    {/* X-axis labels (temperature) */}
                    {tempTicks.map(t => {
                        const x = this.tempToX(t);
                        return <text x={x} y={VH - M.bottom + 16} text-anchor="middle"
                                     font-size="11" fill={textColor}>{t}°</text>;
                    })}

                    {/* X-axis title */}
                    <text x={M.left + this.plotWidth / 2} y={VH - 2} text-anchor="middle"
                          font-size="12" fill={textColor}>{__("component.heating_curve_chart.temperature_daily_avg")}</text>

                    {/* Y-axis labels (hours) */}
                    {hourTicks.map(hr => {
                        const y = this.hoursToY(hr, yMax);
                        return <text x={M.left - 6} y={y + 4} text-anchor="end"
                                     font-size="11" fill={textColor}>{hr}h</text>;
                    })}

                    {/* Y-axis title */}
                    <text x={12} y={M.top + this.plotHeight / 2}
                          text-anchor="middle" font-size="12" fill={textColor}
                          transform={`rotate(-90, 12, ${M.top + this.plotHeight / 2})`}>
                        {__("component.heating_curve_chart.curve_hours")}
                    </text>

                    {/* Current temperature vertical dashed line */}
                    {currentTempX !== null &&
                        <line x1={currentTempX} y1={M.top} x2={currentTempX} y2={VH - M.bottom}
                              stroke={textColor} stroke-width="1" stroke-dasharray="4,3" opacity="0.6" />
                    }

                    {/* Extended operation line (blue) */}
                    {show_extended &&
                        <line x1={extX1} y1={extY1} x2={extX2} y2={extY2}
                              stroke={extColor} stroke-width="2.5" />
                    }

                    {/* Blocking operation line (red) */}
                    {show_blocking &&
                        <line x1={blkX1} y1={blkY1} x2={blkX2} y2={blkY2}
                              stroke={blkColor} stroke-width="2.5" />
                    }

                    {/* Endpoint markers and labels for extended (blue) */}
                    {show_extended && <>
                        {/* Warm endpoint (left side at 20°C) */}
                        <circle cx={extX1} cy={extY1}
                                r={this.isActive("ext_warm") ? activeR : baseR}
                                fill={extColor}
                                style={interactive ? `cursor: ${dragging === "ext_warm" ? "grabbing" : "grab"}` : ""}
                                opacity={this.isActive("ext_warm") ? 0.8 : 1} />
                        {interactive && <circle cx={extX1} cy={extY1} r={hitR}
                                fill="transparent" style="cursor: grab"
                                onPointerDown={(ev: PointerEvent) => this.onEndpointPointerDown("ext_warm", ev)}
                                onPointerEnter={() => this.setState({ hovered: "ext_warm" })}
                                onPointerLeave={() => { if (hovered === "ext_warm") this.setState({ hovered: null }); }}
                        />}
                        {/* Cold endpoint (right side at -10°C) */}
                        <circle cx={extX2} cy={extY2}
                                r={this.isActive("ext_cold") ? activeR : baseR}
                                fill={extColor}
                                style={interactive ? `cursor: ${dragging === "ext_cold" ? "grabbing" : "grab"}` : ""}
                                opacity={this.isActive("ext_cold") ? 0.8 : 1} />
                        {interactive && <circle cx={extX2} cy={extY2} r={hitR}
                                fill="transparent" style="cursor: grab"
                                onPointerDown={(ev: PointerEvent) => this.onEndpointPointerDown("ext_cold", ev)}
                                onPointerEnter={() => this.setState({ hovered: "ext_cold" })}
                                onPointerLeave={() => { if (hovered === "ext_cold") this.setState({ hovered: null }); }}
                        />}
                        <text x={extX1 - 6} y={extWarmLabelY}
                              text-anchor="end" font-size="11" font-weight="bold" fill={extColor}>
                            {extended_hours_warm}h
                        </text>
                        <text x={extX2 + 6} y={extColdLabelY}
                              text-anchor="start" font-size="11" font-weight="bold" fill={extColor}>
                            {extended_hours_cold}h
                        </text>
                    </>}

                    {/* Endpoint markers and labels for blocking (red) */}
                    {show_blocking && <>
                        {/* Warm endpoint (left side at 20°C) */}
                        <circle cx={blkX1} cy={blkY1}
                                r={this.isActive("blk_warm") ? activeR : baseR}
                                fill={blkColor}
                                style={interactive ? `cursor: ${dragging === "blk_warm" ? "grabbing" : "grab"}` : ""}
                                opacity={this.isActive("blk_warm") ? 0.8 : 1} />
                        {interactive && <circle cx={blkX1} cy={blkY1} r={hitR}
                                fill="transparent" style="cursor: grab"
                                onPointerDown={(ev: PointerEvent) => this.onEndpointPointerDown("blk_warm", ev)}
                                onPointerEnter={() => this.setState({ hovered: "blk_warm" })}
                                onPointerLeave={() => { if (hovered === "blk_warm") this.setState({ hovered: null }); }}
                        />}
                        {/* Cold endpoint (right side at -10°C) */}
                        <circle cx={blkX2} cy={blkY2}
                                r={this.isActive("blk_cold") ? activeR : baseR}
                                fill={blkColor}
                                style={interactive ? `cursor: ${dragging === "blk_cold" ? "grabbing" : "grab"}` : ""}
                                opacity={this.isActive("blk_cold") ? 0.8 : 1} />
                        {interactive && <circle cx={blkX2} cy={blkY2} r={hitR}
                                fill="transparent" style="cursor: grab"
                                onPointerDown={(ev: PointerEvent) => this.onEndpointPointerDown("blk_cold", ev)}
                                onPointerEnter={() => this.setState({ hovered: "blk_cold" })}
                                onPointerLeave={() => { if (hovered === "blk_cold") this.setState({ hovered: null }); }}
                        />}
                        <text x={blkX1 - 6} y={blkWarmLabelY}
                              text-anchor="end" font-size="11" font-weight="bold" fill={blkColor}>
                            {blocking_hours_warm}h
                        </text>
                        <text x={blkX2 + 6} y={blkColdLabelY}
                              text-anchor="start" font-size="11" font-weight="bold" fill={blkColor}>
                            {blocking_hours_cold}h
                        </text>
                    </>}

                    {/* Current temperature intersection: dotted lines to right axis + labels */}
                    {currentTempX !== null && show_extended && <>
                        <circle cx={currentTempX} cy={this.hoursToY(currentExtHours, yMax)} r="4"
                                fill="none" stroke={extColor} stroke-width="2" />
                        <line x1={currentTempX} y1={this.hoursToY(currentExtHours, yMax)}
                              x2={VW - M.right} y2={this.hoursToY(currentExtHours, yMax)}
                              stroke={extColor} stroke-width="1" stroke-dasharray="3,3" />
                        <text x={VW - M.right + 4} y={currentExtLabelY}
                              text-anchor="start" font-size="11" font-weight="bold" fill={extColor}>
                            {util.toLocaleFixed(currentExtHours, 1)}h
                        </text>
                    </>}
                    {currentTempX !== null && show_blocking && <>
                        <circle cx={currentTempX} cy={this.hoursToY(currentBlkHours, yMax)} r="4"
                                fill="none" stroke={blkColor} stroke-width="2" />
                        <line x1={currentTempX} y1={this.hoursToY(currentBlkHours, yMax)}
                              x2={VW - M.right} y2={this.hoursToY(currentBlkHours, yMax)}
                              stroke={blkColor} stroke-width="1" stroke-dasharray="3,3" />
                        <text x={VW - M.right + 4} y={currentBlkLabelY}
                              text-anchor="start" font-size="11" font-weight="bold" fill={blkColor}>
                            {util.toLocaleFixed(currentBlkHours, 1)}h
                        </text>
                    </>}

                    {/* Current temperature label at top of dashed line */}
                    {currentTempX !== null &&
                        <text x={currentTempX} y={M.top - 6} text-anchor="middle"
                              font-size="10" fill={textColor}>
                            {__("component.heating_curve_chart.current_temperature")} ({util.toLocaleFixed(current_temperature, 1)}°)
                        </text>
                    }
                </svg>
                {/* Legend below chart */}
                <div class="d-flex justify-content-center gap-3 mt-1" style="font-size: 0.8rem;">
                    {show_extended &&
                        <span class="d-flex align-items-center gap-1">
                            <svg width="18" height="10"><line x1="0" y1="5" x2="18" y2="5" stroke={extColor} stroke-width="2.5" /></svg>
                            {__("component.heating_curve_chart.extended_operation")}
                        </span>
                    }
                    {show_blocking &&
                        <span class="d-flex align-items-center gap-1">
                            <svg width="18" height="10"><line x1="0" y1="5" x2="18" y2="5" stroke={blkColor} stroke-width="2.5" /></svg>
                            {__("component.heating_curve_chart.blocking_operation")}
                        </span>
                    }
                </div>
            </div>
        );
    }
}

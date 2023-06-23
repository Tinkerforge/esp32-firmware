/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

// from https://leeoniya.github.io/uPlot/demos/timeline-discrete.html

import uPlot from 'uplot';

export function uPlotTimelinePlugin(options: any) {
    const { fill, stroke } = options;

    const fillPaths = new Map();
    const strokePaths = new Map();

    function drawBoxes(ctx: CanvasRenderingContext2D) {
        fillPaths.forEach((fillPath, fillStyle) => {
            ctx.fillStyle = fillStyle;
            ctx.fill(fillPath);
        });

        strokePaths.forEach((strokePath, strokeStyle) => {
            ctx.strokeStyle = strokeStyle;
            ctx.stroke(strokePath);
        });

        fillPaths.clear();
        strokePaths.clear();
    }

    function putBox(ctx: CanvasRenderingContext2D, rect: uPlot.RectH | uPlot.RectV, lft: number, top: number, wid: number, hgt: number, strokeWidth: number, iy: number, ix: number, value: number|null) {
        let fillStyle = fill(iy + 1, ix, value);
        let fillPath = fillPaths.get(fillStyle);

        if (fillPath == null)
            fillPaths.set(fillStyle, fillPath = new Path2D());

        rect(fillPath, lft, top, wid, hgt);

        if (strokeWidth) {
            let strokeStyle = stroke(iy + 1, ix, value);
            let strokePath = strokePaths.get(strokeStyle);

            if (strokePath == null)
                strokePaths.set(strokeStyle, strokePath = new Path2D());

            rect(strokePath, lft + strokeWidth / 2, top + strokeWidth / 2, wid - strokeWidth, hgt - strokeWidth);
        }
    }

    function drawPaths(u: uPlot, sidx: number, idx0: number, idx1: number): uPlot.Series.Paths|null {
        uPlot.orient(u, sidx, (series, dataX, dataY, scaleX, scaleY, valToPosX, valToPosY, xOff, yOff, xDim, yDim, moveTo, lineTo, rect) => {
            let strokeWidth = Math.round((series.width || 0) * devicePixelRatio);

            u.ctx.save();
            rect(u.ctx, u.bbox.left, u.bbox.top, u.bbox.width, u.bbox.height);
            u.ctx.clip();

            let hgt = options.bar_height * devicePixelRatio;
            let iy = sidx - 1;
            let y0 = 0;
            let last_rgt: number = undefined;

            for (let i = 1; i < sidx; i++) {
                if (u.series[i].show) {
                    y0 += (options.bar_height + options.bar_spacing) * devicePixelRatio;
                }
            }

            for (let ix = 0; ix < dataY.length; ix++) {
                if (dataY[ix] === null) {
                    last_rgt = undefined;
                }
                else {
                    let lft: number;

                    if (last_rgt !== undefined) {
                        lft = last_rgt;
                    }
                    else {
                        lft = Math.round(valToPosX(dataX[ix], scaleX, xDim, xOff));
                    }

                    let nextIx = ix;
                    while (dataY[++nextIx] === undefined && nextIx < dataY.length) {}

                    let rgt0 = valToPosX(dataX[nextIx - 1], scaleX, xDim, xOff);
                    let rgt1 = rgt0;

                    if (dataY[nextIx] !== null && dataY[nextIx] !== undefined) {
                        rgt1 = valToPosX(dataX[nextIx], scaleX, xDim, xOff);
                    }

                    let rgt = Math.round(rgt0 + (rgt1 - rgt0) / 2);

                    last_rgt = rgt;

                    putBox(
                        u.ctx,
                        rect,
                        lft,
                        Math.round(yOff + y0),
                        Math.max(rgt - lft, 1),
                        Math.round(hgt),
                        strokeWidth,
                        iy,
                        ix,
                        dataY[ix]
                    );

                    ix = nextIx - 1;
                }
            }

            u.ctx.lineWidth = strokeWidth;
            drawBoxes(u.ctx);

            u.ctx.restore();
        });

        return null;
    }

    return {
        hooks: {
            addSeries: (u: uPlot, seriesIdx: number) => {
                if (seriesIdx > 0) {
                    uPlot.assign(u.series[seriesIdx], {
                        paths: drawPaths,
                    });
                }
            },
        },
        opts: (u: uPlot, opts: uPlot.Options) => {
            uPlot.assign(opts, {
                cursor: {
                    y: false,
                    dataIdx: (u: uPlot, seriesIdx: number, closestIdx: number, xValue: number) => {
                        if (seriesIdx == 0) {
                            return closestIdx;
                        }

                        let didx = u.posToIdx(Math.round(u.cursor.left));
                        let data = u.data[seriesIdx][didx];

                        while (data === undefined && didx > 0) {
                            --didx;
                            data = u.data[seriesIdx][didx];
                        }

                        if (data === undefined) {
                            return undefined;
                        }

                        return didx;
                    },
                    points: {
                        show: false,
                    }
                },
                scales: {
                    y: {
                        range: [0, 1],
                    }
                }
            });

            uPlot.assign(opts.axes[1], {
                splits: (u: uPlot, axisIdx: number) => {
                    let count = 0;
                    let splits = [];

                    for (let i = 1; i < u.series.length; i++) {
                        if (u.series[i].show) {
                            let mid = Math.round(options.bar_height / 2 + count * (options.bar_height +  options.bar_spacing));

                            splits.push(u.posToVal(mid, "y"));

                            ++count;
                        }
                    }

                    return splits;
                },
                values: (u: uPlot) => {
                    let values: string[] = [];

                    for (let i = 1; i < u.series.length; i++) {
                        if (u.series[i].show) {
                            let label = u.series[i].label;

                            if (label.length > 15) {
                                label = label.slice(0, 12) + '\u2026';
                            }

                            values.push(label);
                        }
                    }

                    return values;
                },
                grid: {
                    show: false,
                },
                ticks: {
                    show: false,
                },
                side: 3,
            });

            opts.series.forEach((s: uPlot.Series, i: number) => {
                if (i > 0) {
                    uPlot.assign(s, {
                        paths: drawPaths,
                    });
                }
            });
        }
    };
}

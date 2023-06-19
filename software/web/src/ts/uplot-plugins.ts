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

function pointWithin(px: number, py: number, rlft: number, rtop: number, rrgt: number, rbtm: number) {
    return px >= rlft && px <= rrgt && py >= rtop && py <= rbtm;
}

class Quadtree {
    MAX_OBJECTS = 10;
    MAX_LEVELS = 4;

    x: number;
    y: number;
    w: number;
    h: number;
    l: number;
    o: {x: number, y: number, w: number, h: number, sidx: number, didx: number}[];
    q: Quadtree[];

    constructor(x: number, y: number, w: number, h: number, l?: number) {
        let t = this;

        t.x = x;
        t.y = y;
        t.w = w;
        t.h = h;
        t.l = l || 0;
        t.o = [];
        t.q = null;
    };

    split() {
        let t = this,
            x = t.x,
            y = t.y,
            w = t.w / 2,
            h = t.h / 2,
            l = t.l + 1;

        t.q = [
            // top right
            new Quadtree(x + w, y,     w, h, l),
            // top left
            new Quadtree(x,     y,     w, h, l),
            // bottom left
            new Quadtree(x,     y + h, w, h, l),
            // bottom right
            new Quadtree(x + w, y + h, w, h, l),
        ];
    }

    // invokes callback with index of each overlapping quad
    quads(x: number, y: number, w: number, h: number, cb: (q: Quadtree) => void) {
        let t            = this,
            q            = t.q,
            hzMid        = t.x + t.w / 2,
            vtMid        = t.y + t.h / 2,
            startIsNorth = y     < vtMid,
            startIsWest  = x     < hzMid,
            endIsEast    = x + w > hzMid,
            endIsSouth   = y + h > vtMid;

        // top-right quad
        startIsNorth && endIsEast && cb(q[0]);
        // top-left quad
        startIsWest && startIsNorth && cb(q[1]);
        // bottom-left quad
        startIsWest && endIsSouth && cb(q[2]);
        // bottom-right quad
        endIsEast && endIsSouth && cb(q[3]);
    }

    add(o: {x: number, y: number, w: number, h: number, sidx: number, didx: number}) {
        let t = this;

        if (t.q != null) {
            t.quads(o.x, o.y, o.w, o.h, (q: Quadtree) => {
                q.add(o);
            });
        }
        else {
            let os = t.o;

            os.push(o);

            if (os.length > this.MAX_OBJECTS && t.l < this.MAX_LEVELS) {
                t.split();

                for (let i = 0; i < os.length; i++) {
                    let oi = os[i];

                    t.quads(oi.x, oi.y, oi.w, oi.h, (q: Quadtree) => {
                        q.add(oi);
                    });
                }

                t.o.length = 0;
            }
        }
    }

    get(x: number, y: number, w: number, h: number, cb: (q: {x: number, y: number, w: number, h: number, sidx: number, didx: number}) => void) {
        let t = this;
        let os = t.o;

        for (let i = 0; i < os.length; i++)
            cb(os[i]);

        if (t.q != null) {
            t.quads(x, y, w, h, (q: Quadtree) => {
                q.get(x, y, w, h, cb);
            });
        }
    }

    clear() {
        this.o.length = 0;
        this.q = null;
    }
}

function roundDec(val: number, dec: number) {
    return Math.round(val * (dec = 10**dec)) / dec;
}

const SPACE_BETWEEN = 1;
const SPACE_AROUND  = 2;
const SPACE_EVENLY  = 3;

const coord = (i: number, offs: number, iwid: number, gap: number) => roundDec(offs + i * (iwid + gap), 6);

function distr(numItems: number, sizeFactor: number, justify: number, onlyIdx: number, each: (i: number, offPct: number, dimPct: number) => void) {
    let space = 1 - sizeFactor;

    let gap =  (
        justify == SPACE_BETWEEN ? space / (numItems - 1) :
        justify == SPACE_AROUND  ? space / (numItems    ) :
        justify == SPACE_EVENLY  ? space / (numItems + 1) : 0
    );

    if (isNaN(gap) || gap == Infinity)
        gap = 0;

    let offs = (
        justify == SPACE_BETWEEN ? 0       :
        justify == SPACE_AROUND  ? gap / 2 :
        justify == SPACE_EVENLY  ? gap     : 0
    );

    let iwid = sizeFactor / numItems;
    let _iwid = roundDec(iwid, 6);

    if (onlyIdx == null) {
        for (let i = 0; i < numItems; i++)
            each(i, coord(i, offs, iwid, gap), _iwid);
    }
    else
        each(onlyIdx, coord(onlyIdx, offs, iwid, gap), _iwid);
}

const { round, min, max, ceil } = Math;

export function uPlotTimelinePlugin(opts: any) {
    const { fill, stroke } = opts;

    const laneWidth   = 0.9;
    const laneDistr   = SPACE_BETWEEN;

    function walk(yIdx: number, count: number, dim: number, draw: (iy: number, y0: number, hgt: number) => void) {
        distr(count, laneWidth, laneDistr, yIdx, (i: number, offPct: number, dimPct: number) => {
            let laneOffPx = dim * offPct;
            let laneWidPx = dim * dimPct;

            draw(i, laneOffPx, laneWidPx);
        });
    }

    const size  = opts.size  ?? [0.6, Infinity];
    const align = opts.align ?? 0;

    const gapFactor = 1 - size[0];
    const maxWidth  = (size[1] ?? Infinity) * devicePixelRatio;

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

    function putBox(ctx: CanvasRenderingContext2D, rect: uPlot.RectH | uPlot.RectV, xOff: number, yOff: number, lft: number, top: number, wid: number, hgt: number, strokeWidth: number, iy: number, ix: number, value: number|null) {
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

        qt.add({
            x: round(lft - xOff),
            y: round(top - yOff),
            w: wid,
            h: hgt,
            sidx: iy + 1,
            didx: ix
        });
    }

    function drawPaths(u: uPlot, sidx: number, idx0: number, idx1: number): uPlot.Series.Paths|null {
        uPlot.orient(u, sidx, (series, dataX, dataY, scaleX, scaleY, valToPosX, valToPosY, xOff, yOff, xDim, yDim, moveTo, lineTo, rect) => {
            let strokeWidth = round((series.width || 0) * devicePixelRatio);

            u.ctx.save();
            rect(u.ctx, u.bbox.left, u.bbox.top, u.bbox.width, u.bbox.height);
            u.ctx.clip();

            walk(sidx - 1, u.series.length - 1, yDim, (iy: number, y0: number, hgt: number) => {
                let last_rgt: number = undefined;

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
                            lft = round(valToPosX(dataX[ix], scaleX, xDim, xOff));
                        }

                        let nextIx = ix;
                        while (dataY[++nextIx] === undefined && nextIx < dataY.length) {}

                        let rgt0 = valToPosX(dataX[nextIx - 1], scaleX, xDim, xOff);
                        let rgt1 = rgt0;

                        if (dataY[nextIx] !== null && dataY[nextIx] !== undefined) {
                            rgt1 = valToPosX(dataX[nextIx], scaleX, xDim, xOff);
                        }

                        let rgt = round(rgt0 + (rgt1 - rgt0) / 2);

                        last_rgt = rgt;

                        putBox(
                            u.ctx,
                            rect,
                            xOff,
                            yOff,
                            lft,
                            round(yOff + y0),
                            max(rgt - lft, 1),
                            round(hgt),
                            strokeWidth,
                            iy,
                            ix,
                            dataY[ix]
                        );

                        ix = nextIx - 1;
                    }
                }
            });

            u.ctx.lineWidth = strokeWidth;
            drawBoxes(u.ctx);

            u.ctx.restore();
        });

        return null;
    }

    let qt: Quadtree;

    return {
        hooks: {
            drawClear: (u: uPlot) => {
                qt = qt || new Quadtree(0, 0, u.bbox.width, u.bbox.height);
                qt.clear();

                // force-clear the path cache to cause drawBars() to rebuild new quadtree
                u.series.forEach(s => {
                    (s as any)._paths = null;
                });
            },
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
                        if (seriesIdx == 0)
                            return closestIdx;

                        let cx = round(u.cursor.left * devicePixelRatio);
                        let hovered = Array(u.series.length - 1).fill(null);

                        if (cx >= 0) {
                            let yMids = Array(u.series.length - 1).fill(0);

                            walk(null, u.series.length - 1, u.bbox.height, (iy: number, y0: number, hgt: number) => {
                                // vertical midpoints of each series' timeline (stored relative to .u-over)
                                yMids[iy] = round(y0 + hgt / 2);
                            });

                            let cy = yMids[seriesIdx - 1];

                            hovered[seriesIdx - 1] = null;

                            qt.get(cx, cy, 1, 1, (o: {x: number, y: number, w: number, h: number, sidx: number, didx: number}) => {
                                if (pointWithin(cx, cy, o.x, o.y, o.x + o.w, o.y + o.h))
                                    hovered[seriesIdx - 1] = o;
                            });
                        }

                        return hovered[seriesIdx - 1]?.didx;
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
                    let yMids = Array(u.series.length - 1).fill(0);
                    let ySplits = Array(u.series.length - 1).fill(0);

                    walk(null, u.series.length - 1, u.bbox.height, (iy: number, y0: number, hgt: number) => {
                        // vertical midpoints of each series' timeline (stored relative to .u-over)
                        yMids[iy] = round(y0 + hgt / 2);
                        ySplits[iy] = u.posToVal(yMids[iy] / devicePixelRatio, "y");
                    });

                    return ySplits;
                },
                values:     () => Array(u.series.length - 1).fill(null).map((v, i) => u.series[i + 1].label),
                grid:       {show: false},
                ticks:      {show: false},
                side:       3,
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

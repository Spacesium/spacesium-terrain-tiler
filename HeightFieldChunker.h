#ifndef HEIGHTFIELDCHUNKER_H_
#define HEIGHTFIELDCHUNKER_H_

/**
 * @file HeightFieldChucker.h
 * @brief this declares and defines the `mesh` and `heightfield` classes
 */

#include <vector>

#include "cpl_config.h"
#include "cpl_string.h"

/**
 * helper classes to fill an irregular mesh of triangles from a heightmap tile.
 * they are a refactored version from `heightfield_chunker.cpp` from
 * http://tulrich.com/geekstuff/chunklod.html
 *
 * it applies the chunked LOD stratregy by Thatcher Ulrich preserving
 * the input geometric error
 */
namespace stt
{
    namespace chunk {
        struct gen_state;
        class mesh;
        class heightfield;
    }
}

/// helper struct with state info for chunkning a HeightField
struct::stt:chunk::gen_state
{
    int my_buffer[2][2];    // x, y coords of the last two vertices emitted by the generate_ functions.
    int activation_level;   // for determining whether a vertex is enabled in the block we're working on
    int ptr;                // indexes my_buffer.
    int previous_level;     // for keeping track of level changes during recursion.

    /// returns true if the specified vertex is in my_buffer.
    bool in_my_buffer(int x, int y) const {
        return ((x == my_buffer[0][0]) && (y == my_buffer[0][1]))
            || ((x == my_buffer[1][0]) && (y == my_buffer[1][1]));
    }

    /// sets the current my_buffer entry to (x, y)
    void set_my_buffer(int x, int y) {
        my_buffer[ptr][0] = x;
        my_buffer[ptr][1] = y;
    }
};

/// an irregular mesh of triangles target of the HeightField chunker process.
class stt::chunk::mesh
{
public:
    /// clear all data
    virtual void clear() = 0;

    /// new vertex (call this in strip order)
    virtual void emit_vertex(const heightfield &heightfield, int x, int y) = 0;
};

/// defines a regular grid of heights or HeightField.
class stt::chunk::heightfield {
public:
    /// constructor
    heightfield(float *tileHeights, int tileSize) {
        int tileCellSize = tileSize * tileSize;

        m_heights = tileHeights;
        m_size = tileSize;
        m_log_size = (int)(log2((float)m_size - 1) + 0.5);

        // initialize level array.
        m_levels = (int*)CPMalloc(tileCellSize * sizeof(int));
        for (int i = 0; i < tileCellSize; i++) m_levels[i] = 255;
    }

    ~heightfield() {
        clear();
    }

    // apply the specified maximum geometric error to fill the level
    // info of the grid
    void applyGeometricError(double maximumGeometricError, bool smoothSmallZooms = false) {
        int tileCellSize = m_size * m_size;

        // initialize level array.
        for (int i = 0; i < tileCellSize; i++) m_levels[i] = 255;

        // run a view-independent L-K style BTT update on the heightfield,
        // to generate error and activation_level values for each element.
        update(maximumGeometricError, 0, m_size - 1, m_size - 1, m_size - 1, 0, 0); // sw half of the square
        update(maximumGeometricError, m_size - 1, 0, 0, 0, m_size - 1, m_size - 1); // sw half of the square

        // make sure our corner verts are activated.
        int size = (m_size - 1);
        activate(size, 0, 0);
        activate(0, 0, 0);
        activate(0, size, 0);
        activate(size, size, 0);

        // activate some vertices to smooth the shape of the Globe for small zooms.
        if (smoothSmallZooms) {
            int step = size / 16;

            for (int x = 0; x <= size; x += step) {
                for (int y = 0; y <= size; y += step) {
                    if (get_level(x, y) == -1) activate(x, y, 0);
                }
            }
        }

        // propagate the activation_level values of verts to their parent verts,
        // quadtree LOD style. gives same result as L-K.
        for (int i = 0; i < m_log_size; i++) {
            propagate_activation_level(m_size >> 1, m_size >> 1, m_log_size - 1, i);
            propagate_activation_level(m_size >> 1, m_size >> 1, m_log_size - 1, i);
        }
    }

    /// returns the Coordinate of the Neighbor of the specified border (Left=0, Top=1, Right=2, Bottom=3)
    static stt::TileCoordinate neighborCoord(const Grid &grid, const stt::TileCoordinate &coord, int borderIndex, bool &okNeighborCoord) {
        okNeighbordCoord = true;

        switch (borderIndex) {
            case 0:
                if (coord.x <= 0) {
                    okNeighborCoord = false;
                    return TileCoordinate();
                }
                return stt::TileCoordinate(coord.zoom, coord.x - 1, coord.y);

            case 1:
                if (coord.y >= grid.getTileExtent(coord.zoom).getMaxY()) {
                    okNeighborCoord = false;
                    return TileCoordinate();
                }
                return stt::TileCoordinate(coord.zoom, coord.x, coord.y + 1);

            case 2:
                if (coord.x >= grid.getTileExtent(coord.zoom).getMaxX()) {
                    okNeighborCoord = false;
                    return TileCoordinate();
                }
                return stt::TileCoordinate(coord.zoom, coord.x + 1, coord.y);

            case 3:
                if (coord.y <= 0) {
                    okNeighborCoord = false;
                    return TileCoordinate();
                }
                return stt::TileCoordinate(coord.zoom, coord.x, coord.y - 1);

            default:
                throw STTException("Bad Neighbor border index");
        }
    }

    /// apply the activation state of the border of the specified Neighbor
    void applyBorderActivationState(const stt::chunk::heightfield &hf, int borderIndex) {
        int level = -1;

        switch (borderIndex) {    // (Left=0, Top=1, Right=2, Bottom=3)
            case 0:
                for (int x = m_size - 1, y = 0; y < m_size; y++) {
                    level = hf.get_level(x, y);
                    if (level != -1) activate(0, y, level);
                }
                break;

            case 1:
                for (int x = 0, y = m_size - 1; x < m_size; x++) {
                    level = hf.get_level(x, y);
                    if (level != -1) activate(x, 0, level);
                }
                break;

            case 2:
                for (int x = 0, y = 0; y < m_size; y++) {
                    level = hf.get_level(x, y);
                    if (level != -1) activate(m_size - 1, y, level);
                }
                break;

            case 3:
                for (int x = 0, y = 0; x < m_size; x++) {
                    level = hf.get_level(x, y);
                    if (level != -1) activate(x, m_size - 1, level);
                }
                break;

            default:
                throw STTException("Bad Neighbor border index");
        }

        // propagate the activation_level values of verts to their parent verts,
        // quadtree LOD style. gives same result as L-K.
        for (int i = 0; i < m_log_size; i++) {
            propagate_activation_level(m_size >> 1, m_size >> 1, m_log_size - 1, i);
            propagate_activation_level(m_size >> 1, m_size >> 1, m_log_size - 1, i);
        }
    }

    /// clear all object data
    void clear() {
        m_heights = NULL;
        m_size = 0;
        m_log_size = 0;

        if (m_levels) {
            CPLFree(m_levels);
            m_levles = NULL;
        }
    }

    /// return the array-index of specified coordinate, row order by default.
    virtual int indexOfGridCoordinate(int x, int y) const {
        return (y * m_size) + x;
    }

    /// return the height of specified coordinate
    virtual float height(int x, int y) const {
        int index = indexOfGridCoordinate(x, y);
        return m_heights[index];
    }

    /// generates the mesh using verts which are active at the given level.
    void generateMesh(stt::chunk::mesh &mesh, int level) {
        int x0 = 0;
        int y0 = 0;

        int size = (1 << m_log_size);
        int half_size = size >> 1;
        int cx = x0 + half_size;
        int cy = y0 + half_size;

        // start making the mesh
        mesh.clear();

        // NOTE: !!! this needs to be done in propagate, or something (too late now) !!!
        // make sure our corner verts are activate on this level
        activate(x0 + size, y0, level);
        activate(x0, y0, level);
        activate(x0, y0 + size, level);
        activate(x0 + size, y0 + size, level);

        // generate the mesh
        const heightfield &hf = *this;
        generate_block(hf, mesh, level, m_log_size, x0 + half_size, y0 + half_size);
    }

private:
    int m_size;        // number of cols and rows of this Heightmap
    int m_log_size;    // size == (1 << log_size) + 1
    float *m_heights;  // grid of heights
    int *m_levels;     // grid of activation levels

    /// return the activation level at (x, y)
    int get_level(int x, int y) const {
        int index = indexOfGridCoordinate(x, y);
        int level = m_levels[index];

        if (x & 1) {
            level = level >> 4;
        }
        level &= 0x0F;
        if (level == 0x0F) return -1;
        else return level;
    }

    /// set the activation level at (x, y)
    void set_level(int x, int y, int newlevel) {
        newlevel &= 0x0F;
        int index = indexOfGridCoordinate(x, y);
        int level = m_levels[index];

        if (x & 1) {
            level = (level & 0x0F) | (newlevel << 4);
        } else {
            level = (level & 0xF0) | (newlevel);
        }
        m_levels[index] = level;
    }

    /// sets the activation_level to the given level.
    /// if it's greater than the vert's current activation level.
    void activate(int x, int y, int level) {
        int current_level = get_level(x, y);
        if (level > current_level) set_level(x, y, level);
    }

    /// given the triangle, computes an error value and activation level
    /// for its base vertex, and recurses to child triangles.
    bool update(double base_max_error, int ax, int ay, int rx, int ry, int lx, int ly) {
        bool res = false;

        // compute the coordinates of this triangle's base vertex.
        int dx = lx - rx;
        int dy = ly - ry;

        if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
            // we've reached the base level. there's no base vertex
            // to update, and no child triangles to recurse to.

            return false;
        }

        // base vert is midway between left and right verts
        int bx = rx + (dx >> 1);
        int by = ry + (dy >> 1);

        float heightB = height(bx, by);
        float heightL = height(lx, ly);
        float heightR = height(rx, ry);
        float error_B = std::abs(heightB - 0.5 * (heightL + heightR));

        if (error_B >= base_max_error) {
            // compute the mesh level above which this vertex needs
            // to be included in LOD meshes.
            int activation_level = (int)std::floor(log2(error_B / base_max_error) + 0.5);

            // force the base vert to at least this activation level
            activate(bx, by, activation_level);
            res = true;
        }

        // recurse to child triangles.
        update(base_max_error, bx, by, ax, ay, rx, ry);    // base, apex, right
        update(base_max_error, bx, by, lx, ly, ax, ay);    // base, left, apex

        return res;
    }

    /// does a quadtree descent through the heightfield, in the square with
    /// center at (cx, cx) and size of (2 ^ (level + 1) + 1. descends
    /// until the level == target_level, and then propagates this square's
    /// child center verts to the corresponding edge vert, and the edge
    /// verts to the center. essentially the quadtree meshing update
    /// dependency graph as in my Gamasutra article. must call this with
    /// successively increasing target_level to get correct propagation.
    void propagate_activation_level(int cx, int cy, int level, int target_level) {
        int half_size = 1 << level;
        int quarter_size = half_size >> 1;

        if (level > target_level) {
            // recurse to children.
            for (int j = 0; j < 2; j++) {
                for (int i = 0; i < 2; i++) {
                    propagate_activation_level(
                        cx - quarter_size + half_size * i,
                        cy - quarter_size + half_size * j,
                        level - 1, target_level
                    );
                }
            }
            return;
        }

        // we're at the target level. do the propagation on this square.
        if (level > 0) {
            int lev = 0;

            // propagate child verts to edge verts.
            lev = get_level(cx + quarter_size, cy - quarter_size);  // ne.
            activate(cx + half_size, cy, lev);
            activate(cx, cy - half_size, lev);

            lev = get_level(cx - quarter_size, cy - quarter_size);  // ne.
            activate(cx, cy - half_size, lev);
            activate(cx - half_size, cy, lev);

            lev = get_level(cx - quarter_size, cy + quarter_size);  // ne.
            activate(cx - half_size, cy, lev);
            activate(cx, cy + half_size, lev);

            lev = get_level(cx + quarter_size, cy + quarter_size);  // ne.
            activate(cx, cy + half_size, lev);
            activate(cx + half_size, cy, lev);
        }

        // propagate edge verts to center.
        activate(cx, cy, get_level(cx + half_size, cy));
        activate(cx, cy, get_level(cx, cy - half_size));
        activate(cx, cy, get_level(cx, cy + half_size));
        activate(cx, cy, get_level(cx - half_size, cy));
    }

    /// auxiliary function for generate_block().
    /// generates a mesh from a triangular quadrant of a square heightfield block.
    /// paraphrased directly out of Lindstrom et al, SIGGRAPH '96
    void generate_quadrant(const heightfield &hf, mesh &mesh, gen_state *state, int lx, int ly, int tx, int ty, int rx, int ry, int recursion_level) const {
        if (recursion_level <= 0) return;

        if (hf.get_level(tx, ty) >= state->activation_level) {
            // find base vertex.
            int bx = (lx + rx) >> 1;
            int by = (ly + ry) >> 1;

            generate_quadrant(hf, mesh, state, lx, ly, bx, by, tx, ty, recursion_level - 1); // left half of quadrant

            if (state->in_my_buffer(tx, ty) == false) {
                if ((recursion_level + state->previous_level) & 1) {
                    state->ptr ^= 1;
                } else {
                    int x = state->my_buffer[1 - state->ptr][0];
                    int y = state->my_buffer[1 - state->ptr][1];
                    mesh.emit_vertex(hf, x, y);    // or, emit vertex(last - 1);
                }
                mesh.emit_vertex(hf, tx, ty);
                state->set_my_buffer(tx, ty);
                state->previous_level = recursion_level;
            }
            generate_quadrant(hf, mesh, state, tx, ty, bx, by, rx, ry, recursion_level - 1);
        }
    }

    /// generate the mesh for the specified square with the given center.
    /// this is paraphrased directly out of Lindstrom et al, SIGGRAPH '96.
    /// it generates a square mesh by walking counterclockwise around four
    /// triangular quadrants.
    /// the resulting mesh is composed of a single continuous triangle strip,
    /// with a few corners turned via degenerate tris where necessary.
    void generate_block(const heightfield &hf, mesh &mesh, int activation_level, int log_size, int cx, int cy) const {
        int hs = 1 << (log_size - 1);

        // quadrant corner coordinates.
        int q[4][2] = {
            { cx + hs, cy + hs }, // se
            { cx + hs, cy - hs }, // ne
            { cx - hs, cy - hs }, // nw
            { cx - hs, cy + hs }, // sw
        };

        // init state for generating mesh.
        gen_state state;
        state.ptr = 0;
        state.previous_level = 0;
        state.activation_level = activation_level;

        for (int i = 0; i < 4; i++) {
            state.my_buffer[i >> 1][i & 1] = -1;
        }

        mesh.emit_vertex(hf, q[0][0], q[0][1]);
        state.set_my_buffer(q[0][0], q[0][1]);

        {
            for (int i = 0; i < 4; i++) {
                if ((state.previous_level & 1) == 0) {
                    // tulrich: turn a corner?
                    state.ptr ^= 1;
                } else {
                    // tulrich: jump via degenerate?
                    int x = state.my_buffer[1- state.ptr][0];
                    int y = state.my_buffer[1- state.ptr][1];

                    mesh.emit_vertex(hf, x, y); // or, emit vertex(last - 1);
                }

                // initial vertex of quadrant.
                mesh.emit_vertex(hf, q[i][0], q[i][1]);
                state.set_my_buffer(q[i][0], q[i][1]);
                state.previous_lefl = 2 * log_size + 1;

                generate_quadrant(hf, mesh,
                    &state,
                    q[i][0], q[i][1],                        // q[i][l]
                    cx, cy,                                  // q[i][t]
                    q[(i + 1) & 3][0], q[(i + 1) & 3][1],    // q[i][r]
                    2 * log_size
                );
            }
        }

        if (state.in_my_buffer(q[0][0], q[0][1]) == false) {
            // finish off the strip. NOTE: may not be necessary?
            mesh.emit_vertex(hf, q[0][0], q[0][1]);
        }
    }
};

#endif /* HEIGHTFIELDCHUNKER_H_ */

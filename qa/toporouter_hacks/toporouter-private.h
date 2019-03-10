#ifndef __TOPOROUTER_PRIVATE_h
#define __TOPOROUTER_PRIVATE_h

#include <cstdint>
#include <functional>

#include <assert.h>

#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#include "gts.h"

namespace toporouter
{

struct BBOX;
struct EDGE;
struct OPROUTE;
struct ARC;
struct ROUTE;

struct VERTEX
{
    VERTEX();
    ~VERTEX();

    GtsVertex m_v;
    BBOX*     m_bbox;
    VERTEX*   m_parent;
    VERTEX*   m_child;
    EDGE*     m_routingedge;
    uint32_t  m_flags;
    double    m_gcost, m_hcost;
    uint32_t  m_gn;
    ARC*      m_arc;
    OPROUTE*  m_oproute;
    ROUTE*    m_route;
    double    m_thickness;

    double x() const
    {
        return GTS_POINT( &m_v )->x;
    };
    double y() const
    {
        return GTS_POINT( &m_v )->y;
    };
    double z() const
    {
        return GTS_POINT( &m_v )->z;
    };
};

#define TEST_FLAG(x, y) (x == y->Type)

#define TOPOROUTER_FLAG_VERBOSE (1 << 0)
#define TOPOROUTER_FLAG_HARDDEST (1 << 1)
#define TOPOROUTER_FLAG_HARDSRC (1 << 2)
#define TOPOROUTER_FLAG_MATCH (1 << 3)
#define TOPOROUTER_FLAG_LAYERHINT (1 << 4)
#define TOPOROUTER_FLAG_LEASTINVALID (1 << 5)
#define TOPOROUTER_FLAG_AFTERORDER (1 << 6)
#define TOPOROUTER_FLAG_AFTERRUBIX (1 << 7)
#define TOPOROUTER_FLAG_GOFAR (1 << 8)
#define TOPOROUTER_FLAG_DETOUR (1 << 9)

#define EPSILON 0.0001f

//#define DEBUG_ROAR 1

#define coord_distance(a, b, c, d) sqrt(pow(a - c, 2) + pow(b - d, 2))
#define coord_distance2(a, b, c, d) (pow(a - c, 2) + pow(b - d, 2))

#define tvdistance(a, b) sqrt(pow(vx(a) - vx(b), 2) + pow(vy(a) - vy(b), 2))
#define tvdistance2(a, b) (pow(vx(a) - vx(b), 2) + pow(vy(a) - vy(b), 2))

#define edge_v1(e) (GTS_SEGMENT(e)->v1)
#define edge_v2(e) (GTS_SEGMENT(e)->v2)
#define tedge_v1(e) (TOPOROUTER_VERTEX(GTS_SEGMENT(e)->v1))
#define tedge_v2(e) (TOPOROUTER_VERTEX(GTS_SEGMENT(e)->v2))

#define tedge(v1, v2) TOPOROUTER_EDGE(gts_vertices_are_connected(GTS_VERTEX(v1), GTS_VERTEX(v2)))

#define edge_routing(e) (TOPOROUTER_IS_CONSTRAINT(e) ? TOPOROUTER_CONSTRAINT(e)->routing : e->routing)
#define vrouting(v) (edge_routing(v->routingedge))

#define edge_routing_next(e, list) ((list->next) ? TOPOROUTER_VERTEX(list->next->data) : TOPOROUTER_VERTEX(edge_v2(e)))
#define edge_routing_prev(e, list) ((list->prev) ? TOPOROUTER_VERTEX(list->prev->data) : TOPOROUTER_VERTEX(edge_v1(e)))

#define vx(v) (GTS_POINT(v)->x)
#define vy(v) (GTS_POINT(v)->y)
#define vz(v) (GTS_POINT(v)->z)

#define close_enough_xy(a, b) (vx(a) > vx(b) - EPSILON && vx(a) < vx(b) + EPSILON && vy(a) > vy(b) - EPSILON && vy(a) < vy(b) + EPSILON)

#define tev1x(e) (vx(tedge_v1(e))
#define tev1y(e) (vy(tedge_v1(e))
#define tev1z(e) (vz(tedge_v1(e))
#define tev2x(e) (vx(tedge_v2(e))
#define tev2y(e) (vy(tedge_v2(e))
#define tev2z(e) (vz(tedge_v2(e))

#define tvertex_intersect(a, b, c, d) (TOPOROUTER_VERTEX(vertex_intersect(GTS_VERTEX(a), GTS_VERTEX(b), GTS_VERTEX(c), GTS_VERTEX(d))))

#define TOPOROUTER_IS_BBOX(obj) (gts_object_is_from_class(obj, toporouter_bbox_class()))
#define TOPOROUTER_BBOX(obj) GTS_OBJECT_CAST(obj, toporouter_bbox_t, toporouter_bbox_class())
#define TOPOROUTER_BBOX_CLASS(klass) GTS_OBJECT_CLASS_CAST(klass, toporouter_bbox_class_t, toporouter_bbox_class())

typedef enum
{
    PAD,
    PIN,
    VIA,
    T_ARC,
    VIA_SHADOW,
    LINE,
    OTHER,
    T_BOARD,
    EXPANSION_AREA,
    POLYGON,
    TEMP
} toporouter_term_t;

struct _toporouter_bbox_t
{
    GtsBBox b;

    toporouter_term_t type;
    void *data;
    int layer;

    GtsSurface *surface;
    GtsTriangle *enclosing;

    GList *constraints;
    GtsPoint *point,
        *realpoint;

    //  char *netlist, *style;

    struct _toporouter_cluster_t *cluster;
};

struct _toporouter_bbox_class_t
{
    GtsBBoxClass parent_class;
};

typedef struct _toporouter_bbox_t toporouter_bbox_t;
typedef struct _toporouter_bbox_class_t toporouter_bbox_class_t;

#define TOPOROUTER_IS_EDGE(obj) (gts_object_is_from_class(obj, toporouter_edge_class()))
#define TOPOROUTER_EDGE(obj) GTS_OBJECT_CAST(obj, toporouter_edge_t, toporouter_edge_class())
#define TOPOROUTER_EDGE_CLASS(klass) GTS_OBJECT_CLASS_CAST(klass, toporouter_edge_class_t, toporouter_edge_class())

#define EDGE_FLAG_DIRECTCONNECTION (1 << 0)

struct _toporouter_edge_t
{
    GtsEdge e;
    //NetListType *netlist;

    guint flags;

    GList *routing;
};

struct _toporouter_edge_class_t
{
    GtsEdgeClass parent_class;
};

typedef struct _toporouter_edge_t toporouter_edge_t;
typedef struct _toporouter_edge_class_t toporouter_edge_class_t;

#define TOPOROUTER_IS_VERTEX(obj) (gts_object_is_from_class(obj, toporouter_vertex_class()))
#define TOPOROUTER_VERTEX(obj) GTS_OBJECT_CAST(obj, toporouter_vertex_t, toporouter_vertex_class())
#define TOPOROUTER_VERTEX_CLASS(klass) GTS_OBJECT_CLASS_CAST(klass, toporouter_vertex_class_t, toporouter_vertex_class())

#define VERTEX_FLAG_VIZ (1 << 1)
#define VERTEX_FLAG_CCW (1 << 2)
#define VERTEX_FLAG_CW (1 << 3)
#define VERTEX_FLAG_RED (1 << 4)
#define VERTEX_FLAG_GREEN (1 << 5)
#define VERTEX_FLAG_BLUE (1 << 6)
#define VERTEX_FLAG_TEMP (1 << 7)
#define VERTEX_FLAG_ROUTE (1 << 8)
#define VERTEX_FLAG_FAKE (1 << 10)
#define VERTEX_FLAG_SPECCUT (1 << 11)

struct _toporouter_vertex_t
{
    GtsVertex v;
    //GList *boxes;
    struct _toporouter_bbox_t *bbox;

    struct _toporouter_vertex_t *parent;
    struct _toporouter_vertex_t *child;

    toporouter_edge_t *routingedge;

    guint flags;

    gdouble gcost, hcost;
    guint gn;

    struct _toporouter_arc_t *arc;

    struct _toporouter_oproute_t *oproute;
    struct _toporouter_route_t *route;

    gdouble thickness;
};

struct _toporouter_vertex_class_t
{
    GtsVertexClass parent_class;
};

typedef struct _toporouter_vertex_t toporouter_vertex_t;
typedef struct _toporouter_vertex_class_t toporouter_vertex_class_t;

#define TOPOROUTER_IS_CONSTRAINT(obj) (gts_object_is_from_class(obj, toporouter_constraint_class()))
#define TOPOROUTER_CONSTRAINT(obj) GTS_OBJECT_CAST(obj, toporouter_constraint_t, toporouter_constraint_class())
#define TOPOROUTER_CONSTRAINT_CLASS(klass) GTS_OBJECT_CLASS_CAST(klass, toporouter_constraint_class_t, toporouter_constraint_class())

struct _toporouter_constraint_t
{
    GtsConstraint c;
    toporouter_bbox_t *box;
    GList *routing;
};

struct _toporouter_constraint_class_t
{
    GtsConstraintClass parent_class;
};

typedef struct
{
    gdouble x, y;
} toporouter_spoint_t;

typedef struct _toporouter_constraint_t toporouter_constraint_t;
typedef struct _toporouter_constraint_class_t toporouter_constraint_class_t;

typedef struct
{
    GtsSurface *surface;
    //  GtsTriangle *t;
    //  GtsVertex *v1, *v2, *v3;

    GList *vertices;
    GList *constraints;
    GList *edges;

} toporouter_layer_t;

#define TOPOROUTER_VERTEX_REGION(x) ((toporouter_vertex_region_t *)x)
typedef struct
{

    GList *points;
    toporouter_vertex_t *v1, *v2;
    toporouter_vertex_t *origin;

} toporouter_vertex_region_t;

struct _toporouter_rubberband_arc_t
{
    toporouter_vertex_t *pathv, *arcv;
    gdouble r, d;
    gint wind;
    GList *list;
};

typedef struct _toporouter_rubberband_arc_t toporouter_rubberband_arc_t;
#define TOPOROUTER_RUBBERBAND_ARC(x) ((toporouter_rubberband_arc_t *)x)

struct _toporouter_route_t
{

    struct _toporouter_netlist_t *netlist;

    struct _toporouter_cluster_t *src, *dest;
    struct _toporouter_cluster_t *psrc, *pdest;

    gdouble score, detourscore;

    toporouter_vertex_t *curpoint;
    GHashTable *alltemppoints;

    GList *path;

    guint flags;

    GList *destvertices, *srcvertices;

    GList *topopath;

    gdouble pscore;
    GList *ppath;

    gint *ppathindices;
};

typedef struct _toporouter_route_t toporouter_route_t;

#define TOPOROUTER_ROUTE(x) ((toporouter_route_t *)x)

struct _toporouter_netlist_t
{
    GPtrArray *clusters, *routes;
    char *netlist, *style;
    GList *routed;

    struct _toporouter_netlist_t *pair;
};

typedef struct _toporouter_netlist_t toporouter_netlist_t;

#define TOPOROUTER_NETLIST(x) ((toporouter_netlist_t *)x)

struct _toporouter_cluster_t
{
    gint c, pc;
    GPtrArray *boxes;
    toporouter_netlist_t *netlist;
};

typedef struct _toporouter_cluster_t toporouter_cluster_t;

#define TOPOROUTER_CLUSTER(x) ((toporouter_cluster_t *)x)

#define TOPOROUTER_OPROUTE(x) ((toporouter_oproute_t *)x)

#define oproute_next(a, b) (b->next ? TOPOROUTER_ARC(b->next->data) : a->term2)
#define oproute_prev(a, b) (b->prev ? TOPOROUTER_ARC(b->prev->data) : a->term1)

#define TOPOROUTER_SERPINTINE(x) ((toporouter_serpintine_t *)x)

struct _toporouter_serpintine_t
{
    GList *arcs;
    gdouble x, y;
    gdouble x0, y0, x1, y1;

    gpointer start;
    gdouble halfa, radius;
    guint nhalfcycles;
};
typedef struct _toporouter_serpintine_t toporouter_serpintine_t;

struct _toporouter_oproute_t
{
    GList *arcs;
    toporouter_vertex_t *term1, *term2;
    char *style;
    char *netlist;
    guint layergroup;
    gdouble tof;
    GList *path;

    toporouter_serpintine_t *serp;
};

typedef struct _toporouter_oproute_t toporouter_oproute_t;

#define TOPOROUTER_IS_ARC(obj) (gts_object_is_from_class(obj, toporouter_arc_class()))
#define TOPOROUTER_ARC(obj) GTS_OBJECT_CAST(obj, toporouter_arc_t, toporouter_arc_class())
#define TOPOROUTER_ARC_CLASS(klass) GTS_OBJECT_CLASS_CAST(klass, toporouter_arc_class_t, toporouter_arc_class())

struct _toporouter_arc_t
{
    GtsObject object;

    gdouble x0, y0, x1, y1;
    toporouter_vertex_t *centre, *v;
    gdouble r;
    gint dir;

    GList *clearance;

    toporouter_oproute_t *oproute;

    toporouter_vertex_t *v1, *v2;
};

struct _toporouter_arc_class_t
{
    GtsObjectClass parent_class;
    gboolean binary;
};

typedef struct _toporouter_arc_t toporouter_arc_t;
typedef struct _toporouter_arc_class_t toporouter_arc_class_t;

typedef struct _toporouter_t toporouter_t;

typedef struct
{
    guint id;

    guint *pairwise_nodetour;
    gdouble pairwise_detour_sum;
    gdouble score;
    guint pairwise_fails;

    toporouter_route_t *routedata;

    toporouter_t *r;

} toporouter_netscore_t;

#define TOPOROUTER_NETSCORE(x) ((toporouter_netscore_t *)x)

struct _toporouter_t
{
    GSList *bboxes;
    GNode *bboxtree;

    std::function<bool()> updateCallback;

    toporouter_layer_t *layers;

    GList *paths;

    GList *keepoutlayers;

    guint flags;

    GList *destboxes, *consumeddestboxes;

    /* settings: */
    guint viamax;
    gdouble viacost;
    gdouble stublength;
    gdouble serpintine_half_amplitude;

    gdouble wiring_score;

    GPtrArray *routes;
    GPtrArray *netlists;

    GList *routednets, *failednets;

    gint (*netsort)(toporouter_netscore_t **, toporouter_netscore_t **);

    struct timeval starttime;

    FILE *debug;
};

typedef gint (*oproute_adjseg_func)(toporouter_t *,
                                    GList **,
                                    GList **,
                                    guint *,
                                    gdouble, gdouble, gdouble, gdouble,
                                    toporouter_oproute_t *,
                                    toporouter_oproute_t *);

typedef struct
{
    double s; /* scale factor */

    int mode;
    void *data;

    char *filename;
    double iw, ih; /* image dimensions */
} drawing_context_t;

#define FOREACH_CLUSTER(clusters)                                                                                                                                                \
    do                                                                                                                                                                           \
    {                                                                                                                                                                            \
        for (toporouter_cluster_t **i_cluster = ((toporouter_cluster_t **)clusters->pdata) + clusters->len - 1; i_cluster >= (toporouter_cluster_t **)clusters->pdata && clusters->len > 0; --i_cluster) \
        {                                                                                                                                                                        \
            toporouter_cluster_t *cluster = *i_cluster;

#define FOREACH_BBOX(boxes)                                                                                                                                 \
    do                                                                                                                                                      \
    {                                                                                                                                                       \
        for (toporouter_bbox_t **i_bbox = ((toporouter_bbox_t **)boxes->pdata) + boxes->len - 1; i_bbox >= (toporouter_bbox_t **)boxes->pdata && boxes->len > 0; --i_bbox) \
        {                                                                                                                                                   \
            toporouter_bbox_t *box = *i_bbox;

#define FOREACH_ROUTE(routes)                                                                                                                                      \
    do                                                                                                                                                             \
    {                                                                                                                                                              \
        for (toporouter_route_t **i = ((toporouter_route_t **)routes->pdata) + routes->len - 1; i >= (toporouter_route_t **)routes->pdata && routes->len > 0; --i) \
        {                                                                                                                                                          \
            toporouter_route_t *routedata = *i;

#define FOREACH_NETSCORE(netscores)                                                                                                                                                     \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        for (toporouter_netscore_t **i = ((toporouter_netscore_t **)netscores->pdata) + netscores->len - 1; i >= (toporouter_netscore_t **)netscores->pdata && netscores->len > 0; --i) \
        {                                                                                                                                                                               \
            toporouter_netscore_t *netscore = *i;

#define FOREACH_NETLIST(netlists)                                                                                                                                                \
    do                                                                                                                                                                           \
    {                                                                                                                                                                            \
        for (toporouter_netlist_t **i = ((toporouter_netlist_t **)netlists->pdata) + netlists->len - 1; i >= (toporouter_netlist_t **)netlists->pdata && netlists->len > 0; --i) \
        {                                                                                                                                                                        \
            toporouter_netlist_t *netlist = *i;

#define FOREACH_END \
    }               \
    }               \
    while (0)

void create_board_edge(gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble max, gint layer, GList **vlist);
void insert_constraints_from_list(toporouter_t *r, toporouter_layer_t *l, GList *vlist, toporouter_bbox_t *box);
toporouter_bbox_t *toporouter_bbox_create(int layer, GList *vertices, toporouter_term_t type, gpointer data);


int toporoute(toporouter_t *r);
toporouter_t *toporouter_new(void);
/*PadType *AddPad(toporouter_t *r, char *Name,
                gdouble P1X, gdouble P1Y, gdouble P2X, gdouble P2Y, gdouble Thickness,
                gdouble Radius, ShapeType Shape, unsigned int Layer, gdouble OriginX, gdouble OriginY);*/
void AllocateLayers(toporouter_t *r, int NumLayers);
toporouter_bbox_t *toporouter_bbox_locate(toporouter_t *r, toporouter_term_t type, void *data, gdouble x, gdouble y, guint layergroup);
toporouter_netlist_t *netlist_create(toporouter_t *r, char *netlist, char *style);
toporouter_cluster_t *cluster_create(toporouter_t *r, toporouter_netlist_t *netlist);
void cluster_join_bbox(toporouter_cluster_t *cluster, toporouter_bbox_t *box);
void build_cdt(toporouter_t *r, toporouter_layer_t *l);
int read_board_constraints(toporouter_t *r, toporouter_layer_t *l, int layer);
void TopoSetSettings(gdouble Keepaway, gdouble LineThickness);
void TopoSetPCBSettings(int MaxWidth, int MaxHeight, int NumLayers);
void AddRat(toporouter_t *r, double P1X, double P1Y, double P2X, double P2Y, int Group1, int Group2);
toporouter_edge_class_t *toporouter_edge_class(void);
GtsVertex *insert_vertex(toporouter_t *r, toporouter_layer_t *l, gdouble x, gdouble y, toporouter_bbox_t *box);
GList *
rect_with_attachments(gdouble rad,
											gdouble x0, gdouble y0,
											gdouble x1, gdouble y1,
											gdouble x2, gdouble y2,
											gdouble x3, gdouble y3,
											gdouble z);
guint hybrid_router(toporouter_t *r);
toporouter_route_t *routedata_create(void);
toporouter_cluster_t *cluster_find(toporouter_t *r, gdouble x, gdouble y, gdouble z);
toporouter_oproute_t *oproute_rubberband(toporouter_t *r, GList *path);
gint coord_wind(gdouble ax, gdouble ay, gdouble bx, gdouble by, gdouble cx, gdouble cy);


}; // namespace toporouter

#endif // __TOPOROUTER_PRIVATE_h

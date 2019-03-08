#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>

#include "toporouter-private.h"
#include "toporouter.h"


#include <class_board.h>
#include <class_drawsegment.h>
#include <class_module.h>
#include <class_pad.h>

#include <connectivity/connectivity_data.h>
#include <ratsnest_data.h>


using namespace toporouter;

TOPOROUTER_ENGINE::TOPOROUTER_ENGINE()
{
    m_router = toporouter::toporouter_new();
    m_preview = new TOPOROUTER_PREVIEW( this );
}

TOPOROUTER_ENGINE::~TOPOROUTER_ENGINE()
{
    free( m_router );
    delete m_preview;
}

void TOPOROUTER_ENGINE::SetBoard( BOARD* aBoard )
{
    m_board = aBoard;
    m_ruleResolver = new RULE_RESOLVER( m_board );
}

void TOPOROUTER_ENGINE::ClearWorld()
{
}

void TOPOROUTER_ENGINE::syncBoardOutline( toporouter_layer_t* layer, int layerId )
{
    auto brd = board();

    GList* vlist = nullptr;

    toporouter_bbox_t* bbox = nullptr;

    SHAPE_POLY_SET outlines;

    wxString errorText;
    wxPoint  errorLoc;
    auto     rv = brd->GetBoardPolygonOutlines( outlines, &errorText, &errorLoc );

    auto bb = outlines.BBox();

    for( int i = 0; i < outlines.COutline( 0 ).SegmentCount(); i++ )
    {
        const auto s = outlines.COutline( 0 ).CSegment( i );
        create_board_edge( s.A.x, s.A.y, s.B.x, s.B.y, std::max( bb.GetSize().x, bb.GetSize().y ),
                layerId, &vlist );
    }

#if 0
    for( auto item : brd->Drawings() )
    {
        auto ds = dyn_cast<DRAWSEGMENT*>( item );

        log("ds %p\n", ds);

        if( !ds || ds->GetLayer() != Edge_Cuts )
            continue;

        switch( ds->GetShape() )
        {
        case S_SEGMENT: 
            log("sync edge %p", ds);
            create_board_edge( ds->GetStart().x, ds->GetStart().y, ds->GetEnd().x, ds->GetEnd().y, 100000000.0, layerId, &vlist );
            break;

        default: log( "unsupported board outline shape" );
        }
    }
#endif


    bbox = toporouter_bbox_create( layerId, vlist, T_BOARD, nullptr );
    m_router->bboxes = g_slist_prepend( m_router->bboxes, bbox );
    insert_constraints_from_list( m_router, layer, vlist, bbox );
    g_list_free( vlist );
}

void TOPOROUTER_ENGINE::SyncWorld()
{
    m_router->layers = new toporouter_layer_t[rules()->GetGroupCount()];

    printf("\nSyncWorld\n--------------------------\n");

    for( int layerId = 0; layerId < rules()->GetGroupCount(); layerId++ )
    {
        auto cur_layer = &m_router->layers[layerId];

        cur_layer->vertices = nullptr;
        cur_layer->constraints = nullptr;
        cur_layer->surface = nullptr;

        syncBoardOutline( cur_layer, layerId );
        syncPads( cur_layer, layerId );
        //printf( "Surf [%d] = %p\n", layerId, cur_layer->surface );

        build_cdt( m_router, cur_layer );
    }

    
    m_router->bboxtree = gts_bb_tree_new( m_router->bboxes );
    syncConnectivity();

}

void TOPOROUTER_ENGINE::syncPads( toporouter_layer_t* layer, int layerId )
{
    auto brd = board();

    PCB_LAYER_ID kiLayerId = rules()->GetLayerGroup( layerId ).second;

    for( auto mod : brd->Modules() )
    {
        for( auto pad : mod->Pads() )
        {
            if( !pad->GetLayerSet()[kiLayerId] )
                continue;

            switch( pad->GetShape() )
            {
            case PAD_SHAPE_CIRCLE:
            {
                auto               c = pad->GetCenter();
                auto               t = pad->GetSize().x / 2;
                GList*             vlist = NULL;
                toporouter_bbox_t* bbox = NULL;

                printf( "sync pad %d %d r %d\n", c.x, c.y, t );

                vlist = rect_with_attachments( t /* add clearance */, c.x - t, c.y - t, c.x - t,
                        c.y + t, c.x + t, c.y + t, c.x + t, c.y - t, layerId );
                bbox = toporouter_bbox_create( layerId, vlist, PAD, pad );
                m_router->bboxes = g_slist_prepend( m_router->bboxes, bbox );
                insert_constraints_from_list( m_router, layer, vlist, bbox );
                g_list_free( vlist );

                //bbox->point = GTS_POINT( gts_vertex_new(vertex_class, x[0], y[0], 0.) );
                bbox->point = GTS_POINT( insert_vertex( m_router, layer, c.x, c.y, bbox ) );
                //g_assert(TOPOROUTER_VERTEX(bbox->point)->bbox == bbox);
                break;
            }
            }
        }
    }
}

void TOPOROUTER_ENGINE::syncConnectivity()
{
    auto brd = board();
    auto connectivity = brd->GetConnectivity();
    auto cnAlgo = connectivity->GetConnectivityAlgo();

    toporouter_netlist_t *nl = netlist_create(m_router, "Kicad", "Dupa");

    for( const auto cnCluster : cnAlgo->GetClusters() )
    {
        if( !cnCluster->Size() || !cnCluster->HasValidNet() )
            continue;

        toporouter_cluster_t* cluster = cluster_create( m_router, nl );

        printf("process CNcluster %p\n", cnCluster.get());
        for( auto item : *cnCluster )
        {
            auto parent = item->Parent();

            printf("process item %p\n", item);
            
            switch(parent->Type())
            {
                case PCB_PAD_T:
                {
                    toporouter_bbox_t *box = toporouter_bbox_locate(m_router, PAD, parent, item->Anchors()[0]->Pos().x, item->Anchors()[0]->Pos().y, 0 );
                    cluster_join_bbox(cluster, box);
                }
                break;
                default:
                    break;
            }
        }

    }



    for( int i = 1 /* skip "No Net" at [0] */; i < connectivity->GetNetCount(); ++i )
    {
        RN_NET* net = connectivity->GetRatsnestForNet( i );

        if( !net )
            continue;

        for( const auto& edge : net->GetUnconnected() )
        {
            //if ( !edge.IsVisible() )
            //    continue;

            const auto&    sourceNode = edge.GetSourceNode();
            const auto&    targetNode = edge.GetTargetNode();

            if( !sourceNode->Valid() || !targetNode->Valid() )
                continue;

            toporouter_route_t *routedata = routedata_create();
            printf("- NEW route %p\n", routedata );
        	routedata->src = cluster_find(m_router, sourceNode->Pos().x, sourceNode->Pos().y, 0);
	        routedata->dest = cluster_find(m_router, targetNode->Pos().x, targetNode->Pos().y, 0);

            printf("import rat %p %p\n", routedata->src, routedata->dest );
        	routedata->netlist = routedata->src->netlist;

	        //g_assert(routedata->src->netlist == routedata->dest->netlist);

	        g_ptr_array_add(m_router->routes, routedata);
	        g_ptr_array_add(routedata->netlist->routes, routedata);

	        m_router->failednets = g_list_prepend(m_router->failednets, routedata);

            printf("RAT score %.1f failednets %d\n", routedata->score, g_list_length( m_router->failednets ) );

        }
    }

}


TOPOROUTER_ENGINE* TOPOROUTER_ENGINE::GetInstance()
{
    static TOPOROUTER_ENGINE* p = nullptr;
    if( !p )
        p = new TOPOROUTER_ENGINE();

    return p;
}

void TOPOROUTER_ENGINE::Run()
{
   	hybrid_router(m_router);
}

BOARD* TOPOROUTER_ENGINE::board()
{
    return m_board;
}


RULE_RESOLVER::RULE_RESOLVER( BOARD* aBoard )
{
    m_board = aBoard;
}

RULE_RESOLVER::~RULE_RESOLVER()
{
}

double RULE_RESOLVER::GetClearance( const std::string name )
{
    return 0.1e9;
}

double RULE_RESOLVER::GetLineWidth( const std::string name )
{
    return 0.1e9;
}

int RULE_RESOLVER::GetGroupCount()
{
    return 1; // m_board->GetDesignSettings().GetCopperLayerCount(); fixme NB layers
}

std::pair<int, PCB_LAYER_ID> RULE_RESOLVER::GetLayerGroup( int l )
{
    int          cnt = m_board->GetDesignSettings().GetCopperLayerCount();
    PCB_LAYER_ID rv;
    if( l == 0 )
        rv = F_Cu;
    else if( l == cnt - 1 )
        rv = B_Cu;

    return std::pair<int, PCB_LAYER_ID>( l, rv );
}

void TOPOROUTER_ENGINE::log( const std::string fmt, ... )
{
    char    str[1024];
    va_list vl;
    va_start( vl, fmt );
    vsnprintf( str, 1024, fmt.c_str(), vl );
    printf( "TopoR: %s\n", str );
    fflush( stdout );
    va_end( vl );
}

TOPOROUTER_PREVIEW::TOPOROUTER_PREVIEW( TOPOROUTER_ENGINE* engine ) : EDA_ITEM( NOT_USED )
{
    m_router = engine;
};

TOPOROUTER_PREVIEW::~TOPOROUTER_PREVIEW()
{
}
using namespace KIGFX;

void TOPOROUTER_PREVIEW::ViewDraw( int aLayer, VIEW* aView ) const
{
    auto gal = aView->GetGAL();
    gal->SetTarget( TARGET_NONCACHED );

    auto rtr = m_router->GetRouter();

    for( int i = 0; i < m_router->rules()->GetGroupCount(); i++ )
    {
        drawSurface( gal, rtr, rtr->layers[i].surface );
    }

    drawRouted( gal );
}

static int giterate_vector( gpointer item, gpointer data )
{
    std::vector<gpointer>* v = reinterpret_cast<std::vector<gpointer>*>( data );
    v->push_back( item );
    return 0;
}


void TOPOROUTER_PREVIEW::drawSurface( GAL* gal, toporouter_t* router, GtsSurface* surf ) const
{
    std::vector<gpointer> edges, vertices;

    gts_surface_foreach_edge( surf, giterate_vector, &edges );
    gts_surface_foreach_vertex( surf, giterate_vector, &vertices );
    //printf( "draw %p edges %d vtx %d\n", surf, edges.size(), vertices.size() );

    gal->SetIsStroke( true );
    gal->SetIsFill( false );
    gal->SetStrokeColor( COLOR4D( 0.5, 0.5, 1.0, 1.0 ) );
    gal->SetLineWidth( 10000.0 );
    gal->SetLayerDepth( gal->GetMinDepth() );

    for( auto item : edges )
    {
        if( TOPOROUTER_IS_EDGE( (GtsObject*) item ) )
        {
            auto te = TOPOROUTER_EDGE( (GtsObject*) item );

            //      printf( "draw edge %p\n", te );

            VECTOR2D a( te->e.segment.v1->p.x, te->e.segment.v1->p.y );
            VECTOR2D b( te->e.segment.v2->p.x, te->e.segment.v2->p.y );

            gal->DrawLine( a, b );
        }
    }
    //	gts_surface_foreach_vertex(surf, toporouter_draw_vertex, nullptr );
}

void TOPOROUTER_PREVIEW::drawRouted( KIGFX::GAL* gal ) const
{
    gal->SetIsStroke( true );
    gal->SetIsFill( false );
    gal->SetStrokeColor( COLOR4D( 0.5, 1.0, 0.5, 1.0 ) );
    gal->SetLineWidth( 100000.0 );
    gal->SetLayerDepth( gal->GetMinDepth() );

    for( auto r : m_routed )
    {
        gal->DrawLine( r.A, r.B );
    }
}

void TOPOROUTER_ENGINE::ImportRoutes()
{
    GList *iter = m_router->routednets;
    std::vector<toporouter_oproute_t*> oproutes;
	toporouter_arc_t *arc, *parc = NULL;

	while (iter)
	{
		toporouter_route_t *routedata = TOPOROUTER_ROUTE(iter->data);
		toporouter_oproute_t *oproute = oproute_rubberband(m_router, routedata->path);
		oproutes.push_back(oproute);
		iter = iter->next;
	}

    for( auto oproute : oproutes )
    {
        GList *arcs = oproute->arcs;

    	if (!arcs)
	    {
            m_preview->AddRouted( vx(oproute->term1), vy(oproute->term1), vx(oproute->term2), vy(oproute->term2) ) ;
		    return;
	    } else {
            while (arcs)
            {
                arc = TOPOROUTER_ARC(arcs->data);

                if (parc && arc)
                {
                    //ar->wiring_score += export_pcb_drawarc(layer, parc, thickness, keepaway);
                    m_preview->AddRouted( parc->x1, parc->y1, arc->x0, arc->y0 );
                }
                else if (!parc)
                {
                    m_preview->AddRouted( vx(oproute->term1), vy(oproute->term1), arc->x0, arc->y0 );
                }

                parc = arc;
                arcs = arcs->next;
            }
            //ar->wiring_score += export_pcb_drawarc(layer, arc, thickness, keepaway);
            m_preview->AddRouted( arc->x1, arc->y1, vx(oproute->term2), vy(oproute->term2) );

        }
    }



}

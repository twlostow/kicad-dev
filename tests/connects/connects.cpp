#include <wx/wx.h>
#include <wx/app.h>

#include <wx/timer.h>
#include <wx/math.h>
#include <wx/log.h>
#include <wx/popupwin.h>


#include <profile.h>

#include <class_pad.h>
#include <class_module.h>
#include <class_board.h>
#include <class_track.h>
#include <class_zone.h>


#include <io_mgr.h>

#include <set>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#include <class_board.h>
#include <geometry/rtree.h>


/*class BOARD_CONNECTIVITY {

public:

    void SetBoard( BOARD *aBoard );
    void Update ( BOARD_ITEM *aItem );

};*/






struct CN_ITEM
{
    BOARD_CONNECTED_ITEM *parent;
    int subpolyIndex;

    EDA_RECT bbox;

    LSET layers;
    std::vector<wxPoint> anchors;
    std::set<CN_ITEM *> connected;

};

typedef RTree<CN_ITEM*, int, 2, float> CN_RTREE_BASE;

typedef std::set<CN_ITEM *> CN_SET;
typedef std::vector<CN_ITEM *> CN_VECTOR;

std::vector<CN_SET> CN_SETS;

CN_SET connectedItems;


class CN_TREE : public CN_RTREE_BASE
{
public:

    void Insert( CN_ITEM* aItem )
    {
        const EDA_RECT&    bbox    = aItem->parent->GetBoundingBox();
        const int       mmin[2] = { bbox.GetX(), bbox.GetY() };
        const int       mmax[2] = { bbox.GetRight(), bbox.GetBottom() };

        CN_RTREE_BASE::Insert( mmin, mmax, aItem );
    }

    void Remove( CN_ITEM* aItem )
    {
        const EDA_RECT&    bbox    = aItem->parent->GetBoundingBox();
        const int       mmin[2] = { bbox.GetX(), bbox.GetY() };
        const int       mmax[2] = { bbox.GetRight(), bbox.GetBottom() };

        CN_RTREE_BASE::Remove( mmin, mmax, aItem );
    }


    struct ConnectivityVisitor
    {
        CN_ITEM *refItem;
        CN_VECTOR& target;

        ConnectivityVisitor ( CN_ITEM *refItem_, CN_VECTOR& target_ ):
        refItem ( refItem_ ),
        target(target_) {}


        bool operator() ( CN_ITEM *aItem )
          {
              if (aItem == refItem)
                    return true;

              if( !( refItem->parent->GetLayerSet() & aItem->parent->GetLayerSet() ).any() )
                return true;


             BOOST_FOREACH( const wxPoint& anchor, refItem->anchors )
             {
                 if ( aItem->parent->HitTest ( anchor ))
                 {
                      target.push_back ( aItem );
                      return true;
                  }
             }

             BOOST_FOREACH( const wxPoint& anchor, aItem->anchors )
             {
                 if ( refItem->parent->HitTest ( anchor ))
                 {
                      target.push_back ( aItem );
                      return true;
                  }
             }

              return true;
          }

    };


    void Query( CN_ITEM* aRefItem, CN_VECTOR& target )    // const
    {
        const EDA_RECT&    bbox    = aRefItem->parent->GetBoundingBox();
        const int       mmin[2] = { bbox.GetX(), bbox.GetY() };
        const int       mmax[2] = { bbox.GetRight(), bbox.GetBottom() };

            ConnectivityVisitor v ( aRefItem, target );
               CN_RTREE_BASE::Search( mmin, mmax, v );

    }
};


void buildConnectedSets(BOARD_ITEM *item)
{
    CN_ITEM *cnItem = new CN_ITEM;
    cnItem->parent = (BOARD_CONNECTED_ITEM *) item;
    cnItem->layers  = item->GetLayerSet();
    cnItem->bbox =item->GetBoundingBox();

    printf("Type %d\n", item->Type () );

    switch(item->Type())
    {
        case PCB_TRACE_T:
        {
            TRACK *t = static_cast <TRACK*> (item);
            cnItem->anchors.push_back( t->GetStart() );
            cnItem->anchors.push_back( t->GetEnd() );
            connectedItems.insert(cnItem);
            break;
        }
        case PCB_VIA_T:
        {
            VIA *v = static_cast <VIA*> (item);
            cnItem->anchors.push_back ( v->GetPosition() );
            connectedItems.insert(cnItem);
            break;
        }

        case PCB_PAD_T:
        {
            D_PAD *pad = static_cast<D_PAD*>(item);
            cnItem->anchors.push_back( pad->ShapePos() );
            connectedItems.insert(cnItem);
            break;
        }

        /*case PCB_ZONE_AREA_T:
        {
            ZONE_CONTAINER *zone = static_cast<ZONE_CONTAINER *> ( item );
            const SHAPE_POLY_SET &shape = zone->GetFilledPolysList();
            delete cnItem;

            EDA_RECT r;

            for (int i =0 ;i < shape.OutlineCount(); i++)
            {
                CN_ITEM *cnItem = new CN_ITEM;
                cnItem->parent = item;
                cnItem->subpolyIndex = i;
                cnItem->layers  = item->GetLayerSet();


                for (int j = 0; j < shape.VertexCount (i); j++)
                {
                    VECTOR2I v ( shape.CVertex(j,i));

                    if(j==0)
                    {
                        r.SetOrigin(wxPoint ( v.x, v.y ) );
                        r.SetSize(wxSize( 0, 0 ) );
                    } else {
                        r.Merge ( wxPoint ( v.x, v.y ) );
                    }

                    cnItem->anchors.push_back ( wxPoint ( v.x, v.y ) );
                }

                cnItem->bbox = r;

                connectedItems.insert(cnItem);

            }

            break;
        }*/
        default:
            break;
    }
}

void buildConnectivity( BOARD *aBoard )
{

//    PROF_COUNTER cnt("build-conn", true);

    connectedItems.clear();

    for (TRACK *t = aBoard->m_Track; t; t=t->Next())
        buildConnectedSets(t);

    for (MODULE *mod = aBoard->m_Modules; mod; mod=mod->Next())
        for(D_PAD *pad = mod->Pads(); pad; pad=pad->Next())
            buildConnectedSets(pad);

    for (int i= 0; i<aBoard->GetAreaCount(); i++)
        buildConnectedSets( aBoard->GetArea(i));


    printf("cns size: %d\n", connectedItems.size());

    CN_TREE tree;

    BOOST_FOREACH ( CN_ITEM *item, connectedItems )
        tree.Insert ( item );

//        cnt.show();

//        PROF_COUNTER cnt2("prop-conn", true);

    int sets = 0;

    CN_VECTOR stack, conns;
    CN_SET current;

    int unconnSets = 0, conflictingSets = 0, nontrivialSets = 0;

    while (!connectedItems.empty() )
    {
        int primaryNet = -1;
        int conflictingNet = -1;

        current.clear();
        stack.clear();
        conns.clear();

	    stack.push_back ( *connectedItems.begin() );
//        current.insert (  *connectedItems.begin() );

        while (!stack.empty())
        {
            CN_ITEM *ref = stack.back ();

            conns.clear();
            stack.pop_back();

            if ( connectedItems.find ( ref ) == connectedItems.end() )
                continue;

            connectedItems.erase ( ref );
            current.insert ( ref );

            tree.Query ( ref, conns );

//            printf("Found : %d\n", conns.size());

            BOOST_FOREACH ( CN_ITEM *item, conns )
            {
//                printf("cis %d Ref %p %s Conn %p %s\n", connectedItems.size(), ref,  (const char *) ref->parent->GetClass().mb_str(), item, (const char *) item->parent->GetClass().mb_str() );

                stack.push_back ( item );
                current.insert ( item );
//                connectedItems.erase ( item  );


            }
        }

        BOOST_FOREACH ( CN_ITEM *item, current )
        {
            if (item->parent->Type() == PCB_PAD_T )
            {
                int net = item->parent->GetNetCode();

                if ( primaryNet < 0)
                    primaryNet = net;

                if ( net != primaryNet && primaryNet >= 0 )
                    conflictingNet = net;
            }
//            printf("rem %p\n", item);
            connectedItems.erase(item);
            //tree.Remove(item);
        }


//        printf("set size %d rema %d\n", current.size(), connectedItems.size() );

        if(primaryNet < 0)
            unconnSets++;

        if(conflictingNet >= 0)
            conflictingSets++;

        if(current.size() > 1)
            nontrivialSets++;

	sets++;

    //    printf("ConnSet: %d items\n", current.size() );
    }

//    cnt2.show();

    printf("%d connected sets found (%d nontrivial, %d unconn, %d conflicting)\n", sets, nontrivialSets, unconnSets, conflictingSets);

//        BOOST_FOREACH ( CN_ITEM *refItem, connectedItems )
//        {

//	printf("query for %p\n" ,refItem );
//	    ConnectivityVisitor v(  refItem );
//            tree.Query ( refItem, v );
//        }
    //}


}

/*
class COMMIT {
	
	COMMIT ( PCB_EDIT_FRAME *aFrame )
	{

	}

	void Add( EDA_ITEM *aItem );
    	void Remove ( EDA_ITEM *aItem );
	void Change ( EDA_ITEM *aItem );

	void Commit( const std::string& message )

private:
    KIGFX::VIEW *m_view;
    
    
};
*/

extern "C"
{
int main()
{
    BOARD* b = new BOARD();

    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        b = pi->Load( wxT( "../../demos/video/video.kicad_pcb" ), NULL, NULL );
        // b=pi -> Load( wxT("../../../tests/conns.kicad_pcb"), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.errorText.GetData() );

        printf( "%s\n", (const char*) msg.mb_str() );
    }
    printf( "brd %p\n", b );

    buildConnectivity( b );

}
}

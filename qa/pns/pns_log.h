#ifndef __PNS_LOG_H
#define __PNS_LOG_H

#include <cstdio>

#include <wx/tokenzr.h>

#include <geometry/shape.h>
#include <geometry/shape_circle.h>
#include <geometry/shape_file_io.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_rect.h>

#include <router/pns_debug_decorator.h>
#include <router/pns_item.h>
#include <router/pns_line.h>
#include <router/pns_line_placer.h>
#include <router/pns_logger.h>
#include <router/pns_node.h>
#include <router/pns_router.h>
#include <router/pns_solid.h>

#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <class_zone.h>

#include <router/pns_kicad_iface.h>

#include <class_board.h>

#include <kicad_plugin.h>
#include <pcb_parser.h>

class PNS_LOG_FILE
{
public:
    PNS_LOG_FILE(){};
    ~PNS_LOG_FILE()
    {
    }

      struct EVENT_ENTRY {
        VECTOR2I p;
        PNS::LOGGER::EVENT_TYPE type;
        const PNS::ITEM* item;
        KIID uuid;
    };

    bool Load( const std::string& logName, const std::string boardName );

    BOARD_CONNECTED_ITEM* ItemById( const EVENT_ENTRY& evt )
    {
        BOARD_CONNECTED_ITEM* parent = nullptr;

        for( auto item : m_board->AllConnectedItems() )
        {
         //   printf("scan %p\n", item);
            if( item->m_Uuid == evt.uuid )
            {
           //     printf("found %p\n", item);
                parent = item;
                break;
            };
        }

        return parent;
    }


    std::vector<EVENT_ENTRY>& Events()
    {
        return m_events;
    }
    std::shared_ptr<BOARD> GetBoard() const
    {
        return m_board;
    }

    void SetBoard( std::shared_ptr<BOARD> brd )
    {
        m_board = brd;
    }

private:
    std::vector<EVENT_ENTRY> m_events;
    std::shared_ptr<BOARD>   m_board;
};


class PNS_TEST_DEBUG_DECORATOR : public PNS::DEBUG_DECORATOR
{
public:
    struct SHAPE_ENT
    {
        SHAPE* m_shape;
        int    m_color;
        int    m_width;
        int                    m_iter;
        std::string m_name;
        std::string m_msg;
    };

    struct STAGE
    {
        std::string            m_name;
        int                    m_iter;
        std::vector<SHAPE_ENT> m_shapes;
    };

    PNS_TEST_DEBUG_DECORATOR()
    {
    }

    virtual ~PNS_TEST_DEBUG_DECORATOR()
    {
    }

    virtual void SetIteration( int iter ) override
    {
        m_iter = iter;
    }

    virtual void Message( const wxString msg ) override;
    virtual void AddPoint( VECTOR2I aP, int aColor, const std::string aName = "" ) override;
    virtual void AddLine( const SHAPE_LINE_CHAIN& aLine, int aType = 0, int aWidth = 0,
            const std::string aName = "" ) override;
    virtual void AddSegment( SEG aS, int aColor, const std::string aName = "" ) override;
    virtual void AddBox( BOX2I aB, int aColor, const std::string aName = "" ) override;
    virtual void AddDirections(
            VECTOR2D aP, int aMask, int aColor, const std::string aName = "" ) override;
    virtual void Clear() override;

    virtual void NewStage( const std::string& name, int iter ) override;

    int GetStageCount() const
    {
        return m_stages.size();
    }
    const STAGE& GetStage( int index ) const
    {
        return m_stages[index];
    }
    BOX2I GetStageExtents( int stage ) const;


private:
    STAGE& currentStage();
    int m_iter;
    std::vector<STAGE> m_stages;
};


class PNS_TEST_ENVIRONMENT
{

public:
    PNS_TEST_ENVIRONMENT();
    ~PNS_TEST_ENVIRONMENT();

    void SetMode( PNS::ROUTER_MODE mode );
    void ReplayLog( PNS_LOG_FILE* aLog, int aStartEventIndex = 0, int aFrom = 0, int aTo = -1 );

    const PNS_TEST_DEBUG_DECORATOR* GetDebugDecorator() const
    {
        return &m_debugDecorator;
    };

private:
    void createRouter();

    PNS::ROUTER_MODE                      m_mode;
    PNS_TEST_DEBUG_DECORATOR              m_debugDecorator;
    std::shared_ptr<BOARD>                m_board;
    std::unique_ptr<PNS_KICAD_IFACE_BASE> m_iface;
    std::unique_ptr<PNS::ROUTER>          m_router;
};


#endif

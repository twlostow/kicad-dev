#include <view/view_ng.h>
#include <view/view_item_ng.h>
#include <view/view_overlay.h>
#include <gal/graphics_abstraction_layer.h>
#include <painter.h>

namespace KIGFX {

VIEW_OVERLAY::VIEW_OVERLAY( VIEW_BASE *aView )
{
    m_view = aView;
    m_gal = m_view->GetGAL();

    m_currentGroup = -1;
}

VIEW_OVERLAY::~VIEW_OVERLAY()
{
    //fprintf(stderr,"VoDestroy!\n");

    m_view->RemoveOverlay ( this );
}

void VIEW_OVERLAY::Clear()
{
    if( m_currentGroup >= 0)
    {
        m_gal->DeleteGroup (m_currentGroup);
        m_currentGroup = -1;
        m_view->MarkTargetDirty ( TARGET_OVERLAY );
    }
}

void VIEW_OVERLAY::Begin()
{
//    fprintf(stderr,"VO-begin\n");
    m_currentGroup = m_gal->BeginGroup();
}

void VIEW_OVERLAY::End()
{
//    fprintf(stderr,"VO-end [cg %d]\n", m_currentGroup);
    if (m_currentGroup >= 0)
    {
        m_gal->EndGroup();
        m_view->MarkTargetDirty ( TARGET_OVERLAY );
    }
}

void VIEW_OVERLAY::updateGroups()
{
    //fprintf(stderr,"VO-update\n");
    if (m_currentGroup < 0)
    {
        m_gal->SetTarget ( TARGET_OVERLAY );
        m_currentGroup = m_gal->BeginGroup();
    }


}

const BOX2I VIEW_OVERLAY::ngViewBBox() const
{
    BOX2I maxBox;

    maxBox.SetMaximum();
    return maxBox;
}


void VIEW_OVERLAY::ngViewDraw( int aLayer, VIEW_BASE *aView ) const
{
    printf("OvlDraw! [grp %d]\n", m_currentGroup);

    if (m_currentGroup >= 0)
        aView->GetGAL()->DrawGroup (m_currentGroup);
}


void VIEW_OVERLAY::DrawSingleItem ( const EDA_ITEM *aItem )
{
    if(!aItem)
        return;

    Clear();
    updateGroups();
    m_view->GetPainter()->Draw ( aItem, 0 );
    End();
}

void VIEW_OVERLAY::ngViewGetLayers( int aLayers[], int& aCount ) const
{
    aLayers[0] = VIEW_BASE::DEFAULT_OVERLAY;
    aCount = 1;
}


}

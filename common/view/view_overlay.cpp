#include <view/view.h>
#include <view/view_item.h>
#include <view/view_overlay.h>
#include <gal/graphics_abstraction_layer.h>

namespace KIGFX {

VIEW_OVERLAY::VIEW_OVERLAY()
{
    m_currentGroup = -1;
}

VIEW_OVERLAY::~VIEW_OVERLAY()
{
    fprintf(stderr,"VoDestroy!\n");
}

void VIEW_OVERLAY::viewAssign( VIEW* aView )
{
    m_currentGroup = -1;

    VIEW_ITEM::viewAssign ( aView );
    m_gal = aView->GetGAL();
}

void VIEW_OVERLAY::Clear()
{
    if( m_currentGroup >= 0)
    {
        m_gal->DeleteGroup (m_currentGroup);
        m_currentGroup = -1;
    }
}

void VIEW_OVERLAY::Begin()
{
    fprintf(stderr,"VO-begin\n");
    m_currentGroup = m_gal->BeginGroup();
}

void VIEW_OVERLAY::End()
{
    fprintf(stderr,"VO-end\n");
    if (m_currentGroup >= 0)
        m_gal->EndGroup();
}

void VIEW_OVERLAY::updateGroups()
{
    fprintf(stderr,"VO-update\n");
    if (m_currentGroup < 0)
        m_currentGroup = m_gal->BeginGroup();
}

const BOX2I VIEW_OVERLAY::ViewBBox() const
{
    BOX2I maxBox;

    maxBox.SetMaximum();
    return maxBox;
}


void VIEW_OVERLAY::ViewDraw( int aLayer, GAL* aGal ) const
{
    printf("OvlDraw! [grp %d]\n", m_currentGroup);
    if (m_currentGroup >= 0)
        aGal->DrawGroup (m_currentGroup);



}


void VIEW_OVERLAY::ViewGetLayers( int aLayers[], int& aCount ) const
{
    // Everything is displayed on a single layer
    aLayers[0] = 0;
    aCount = 1;
}

}
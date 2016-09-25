#include <view/view.h>
#include <view/view_item.h>
#include <profile.h>

namespace KIGFX {

const BOX2I VIEW_ITEM::ViewBBox() const
{
    BOX2I rect;
    rect.SetMaximum();
    return rect;
}

void VIEW_ITEM::ViewGetLayers( int aLayers[], int& aCount ) const
{
    aLayers[0] = VIEW_BASE::DEFAULT_LAYER;
    aCount = 1;
}

}

#include <view/view_ng.h>
#include <view/view_item_ng.h>
#include <profile.h>

namespace KIGFX {

const BOX2I VIEW_ITEM_NG::ngViewBBox() const
{
    BOX2I rect;
    rect.SetMaximum();
    return rect;
}

void VIEW_ITEM_NG::ngViewGetLayers( int aLayers[], int& aCount ) const
{
    aLayers[0] = VIEW_BASE::DEFAULT_LAYER;
    aCount = 1;
}

}

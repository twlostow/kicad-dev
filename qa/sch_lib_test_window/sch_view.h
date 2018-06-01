#ifndef SCH_VIEW_H_
#define SCH_VIEW_H_

#include <memory>
#include <view/view.h>

#include <view/wx_view_controls.h>
#include <worksheet_viewitem.h>
#include <layers_id_colors_and_visibility.h>

class SCH_SHEET;
class LIB_PART;

namespace KIGFX {

class WORKSHEET_VIEWITEM;

class SCH_VIEW : public KIGFX::VIEW
{
public:
    SCH_VIEW( bool aIsDynamic );
    ~SCH_VIEW();

    void DisplaySheet( SCH_SHEET *aSheet );
    void DisplayComponent( LIB_PART *aPart );

    /// @copydoc VIEW::Add()
    virtual void Add( VIEW_ITEM* aItem, int aDrawPriority = -1 ) override;
    /// @copydoc VIEW::Remove()

    virtual void Remove( VIEW_ITEM* aItem ) override;

    /// @copydoc VIEW::Update()
    virtual void Update( VIEW_ITEM* aItem, int aUpdateFlags ) override;

    /// @copydoc VIEW::Update()
    virtual void Update( VIEW_ITEM* aItem ) override;

//    void SetWorksheet( KIGFX::WORKSHEET_VIEWITEM* aWorksheet );

private:
    std::unique_ptr<WORKSHEET_VIEWITEM> m_worksheet;

};

}; // namespace

#endif

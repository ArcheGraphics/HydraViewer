//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QToolButton>
#include <QTreeWidgetItem>
#include <pxr/usd/usd/prim.h>
#include "model/data_model.h"

namespace vox {
class PrimVisButton : public QToolButton {
public:
    PrimVisButton();

    bool toggleVisibility();

    void updateVisIcon();

    bool setVisibility(bool visibility);

private:
    QIcon vis_icon;
    QIcon invis_icon;
    bool vis;
};

class PrimItemWidget : public QTreeWidgetItem {
public:
    explicit PrimItemWidget(pxr::UsdPrim prim);

    QVariant data(int column, int role) const override;

private:
    friend class StageTreeWidget;
    pxr::UsdPrim _prim;
};

class StageTreeWidget : public QTreeWidget {
public:
    StageTreeWidget(DataModel &model, QWidget *parent);

    void refreshTree();

private:
    static PrimItemWidget *createItemFromPrim(const pxr::UsdPrim &prim);

    PrimItemWidget *populateItemTree(const pxr::UsdPrim &prim, QTreeWidgetItem *parent_item);

    static pxr::UsdPrimSiblingRange getFilteredPrimChildren(const pxr::UsdPrim &prim);

    void toggleHierarchyVisibility(QTreeWidgetItem *item, std::optional<bool> set_visibility_to = std::nullopt);

private:
    DataModel &_model;
};
}// namespace vox
//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <fmt/format.h>

#include <QHeaderView>
#include "stage_tree.h"

namespace vox {
PrimVisButton::PrimVisButton() {
    setStyleSheet("padding: 0px; margin: 0px; background-color: rgba(255, 255, 255, 0);");
    vis_icon = QIcon(QString(fmt::format("{}/{}", PROJECT_PATH, "editor/icons/eye_visible.svg").c_str()));
    invis_icon = QIcon(QString(fmt::format("{}/{}", PROJECT_PATH, "editor/icons/eye_invisible.svg").c_str()));
    vis = true;
    setIcon(vis_icon);
    setFixedSize(14, 14);
}

bool PrimVisButton::toggleVisibility() {
    vis = !vis;
    updateVisIcon();
    return vis;
}

void PrimVisButton::updateVisIcon() {
    if (vis) {
        setIcon(vis_icon);
    } else {
        setIcon(invis_icon);
    }
}

bool PrimVisButton::setVisibility(bool visibility) {
    vis = visibility;
    updateVisIcon();
    return vis;
}

PrimItemWidget::PrimItemWidget(pxr::UsdPrim prim) : _prim(std::move(prim)) {
}

QVariant PrimItemWidget::data(int column, int role) const {
    if (column == 0)
        if (role == Qt::DisplayRole)
            return {_prim.GetName().data()};
    return QTreeWidgetItem::data(column, role);
}

StageTreeWidget::StageTreeWidget(DataModel &model, QWidget *parent)
    : QTreeWidget(parent), _model{model} {
    auto qtreewidgetitem = new QTreeWidgetItem();
    qtreewidgetitem->setText(0, "StagePath");
    qtreewidgetitem->setTextAlignment(2, Qt::AlignLeading | Qt::AlignVCenter);
    setHeaderItem(qtreewidgetitem);
    setColumnCount(2);
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setStretchLastSection(false);
    header()->setVisible(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setLineWidth(0);
    setMidLineWidth(0);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);
    setColumnWidth(1, 10);

    connect(&_model, &DataModel::signalStageReplaced, this, &StageTreeWidget::refreshTree);
}

void StageTreeWidget::refreshTree() {
    clear();
    auto stage = _model.stage();
    if (!stage)
        return;

    auto stage_root = stage->GetPseudoRoot();
    auto invisible_root_item = invisibleRootItem();
    populateItemTree(stage_root, invisible_root_item);
    expandToDepth(0);
}

PrimItemWidget *StageTreeWidget::createItemFromPrim(const pxr::UsdPrim &prim) {
    auto item = new PrimItemWidget(prim);
    item->emitDataChanged();
    return item;
}

PrimItemWidget *StageTreeWidget::populateItemTree(const pxr::UsdPrim &prim, QTreeWidgetItem *parent_item) {
    auto created_item = createItemFromPrim(prim);
    parent_item->addChild(created_item);

    // FIXME: this will probably not work in all cases
    if (pxr::UsdGeomImageable(prim).GetVisibilityAttr()) {
        auto vis_button = new PrimVisButton();
        connect(vis_button, &QAbstractButton::clicked, this, [=](bool set_visibility_to) {
            toggleHierarchyVisibility(created_item, std::nullopt);
        });
        setItemWidget(created_item, 1, vis_button);
    }

    auto prim_children = getFilteredPrimChildren(prim);
    for (const auto &prim_child : prim_children) {
        populateItemTree(prim_child, created_item);
    }
    return created_item;
}

pxr::UsdPrimSiblingRange StageTreeWidget::getFilteredPrimChildren(const pxr::UsdPrim &prim) {
    return prim.GetFilteredChildren(pxr::UsdPrimIsActive);
}

void StageTreeWidget::toggleHierarchyVisibility(QTreeWidgetItem *item, std::optional<bool> set_visibility_to) {
    auto item_vis_button = static_cast<PrimVisButton *>(itemWidget(item, 1));
    if (item_vis_button) {
        bool visibility_to;
        if (set_visibility_to) {
            visibility_to = set_visibility_to.value();
            item_vis_button->setVisibility(visibility_to);
        } else {
            visibility_to = item_vis_button->toggleVisibility();
        }

        auto primItem = static_cast<PrimItemWidget *>(item);
        pxr::UsdGeomImageable(primItem->_prim).GetVisibilityAttr().Set(visibility_to ? pxr::UsdGeomTokens->inherited : pxr::UsdGeomTokens->invisible);

        for (int i = 0; i < item->childCount(); ++i) {
            auto child_item = item->child(i);
            toggleHierarchyVisibility(child_item, visibility_to);
        }
    }
}

}// namespace vox
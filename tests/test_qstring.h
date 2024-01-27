//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QTest>

class TestQString : public QObject {
    Q_OBJECT
private slots:
    void toUpper();
};
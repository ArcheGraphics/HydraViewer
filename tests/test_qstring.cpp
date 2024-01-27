//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "test_qstring.h"

void TestQString::toUpper() {
    QString str = "Hello";
    QVERIFY(str.toUpper() == "HELLO");
}
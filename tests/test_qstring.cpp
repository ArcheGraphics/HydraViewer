//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "test_qstring.h"

void TestQString::toUpper() {
    QFETCH(QString, string);
    QFETCH(QString, result);

    QCOMPARE(string.toUpper(), result);
}

void TestQString::toUpper_data() {
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("result");

    QTest::newRow("all-lower") << "hello"
                               << "HELLO";
    QTest::newRow("mixed") << "Hello"
                           << "HELLO";
    QTest::newRow("all-upper") << "HELLO"
                               << "HELLO";
}
//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <Qtest>

#include "test_qstring.h"
#include "test_gui.h"
#include "test_benchmark.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    int status = 0;
    status |= QTest::qExec(new TestQString, argc, argv);
    status |= QTest::qExec(new TestGui, argc, argv);
    status |= QTest::qExec(new TestBenchmark, argc, argv);

    return status;
}
//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <Qtest>

#include "test_qstring.h"

int main(int argc, char *argv[]) {
    int status = 0;
    status |= QTest::qExec(new TestQString, argc, argv);

    return status;
}
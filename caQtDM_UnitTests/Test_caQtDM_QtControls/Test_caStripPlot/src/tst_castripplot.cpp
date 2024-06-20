/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

/// This is a simple example how a Widget can be tested. This will usually be quite difficult, as it would require a lot of mocking.
/// However, it is possible and by reusing this code example together with the corresponding CMakeList it should be reproducible for other widgets.

#include "gtest/gtest.h"

#include <caStripPlot>
#include <QApplication>
#include <QString>

class tst_caStripPlot : public ::testing::Test {

protected:
    QApplication *app; // needed to create QWidgets.
    caStripPlot *m_caStripPlot;

    // The easiest way to serialize the tests so I can reuse the m_urlHandler pointer.
    std::mutex m_testSerializerMutex;

    void SetUp() override
    {
        m_testSerializerMutex.lock();
        int argc = 0;  // Normally, you'd pass argc and argv from main(), but here we set it to 0
        app = new QApplication(argc, nullptr);
        m_caStripPlot = new caStripPlot();
    }

    void TearDown() override
    {
        m_caStripPlot->deleteLater();
        app->deleteLater();
        m_testSerializerMutex.unlock();
    }
};

TEST_F(tst_caStripPlot, TestPlotTitle)
{
    EXPECT_NE(m_caStripPlot, nullptr);
}

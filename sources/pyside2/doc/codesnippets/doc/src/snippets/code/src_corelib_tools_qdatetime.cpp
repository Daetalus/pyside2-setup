/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt for Python project.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
d1 = QDate(1995, 5, 17)  # May 17, 1995
d2 = QDate(1995, 5, 20)  # May 20, 1995
d1.daysTo(d2)          # returns 3
d2.daysTo(d1)          # returns -3
//! [0]


//! [1]
date = QDate.fromString("1MM12car2003", "d'MM'MMcaryyyy")
# date is 1 December 2003
//! [1]


//! [2]
date = QDate.fromString("130", "Md") # invalid
//! [2]


//! [3]
QDate.fromString("1.30", "M.d")           # January 30 1900
QDate.fromString("20000110", "yyyyMMdd")  # January 10, 2000
QDate.fromString("20000110", "yyyyMd")    # January 10, 2000
//! [3]


//! [4]
QDate.isValid(2002, 5, 17)  # True
QDate.isValid(2002, 2, 30)  # False (Feb 30 does not exist)
QDate.isValid(2004, 2, 29)  # True (2004 is a leap year)
QDate.isValid(2000, 2, 29)  # True (2000 is a leap year)
QDate.isValid(2006, 2, 29)  # False (2006 is not a leap year)
QDate.isValid(2100, 2, 29)  # False (2100 is not a leap year)
QDate.isValid(1202, 6, 6)   # True (even though 1202 is pre-Gregorian)
//! [4]


//! [5]
n = QTime(14, 0, 0)              # n == 14:00:00
t = QTime()
t = n.addSecs(70)                # t == 14:01:10
t = n.addSecs(-70)               # t == 13:58:50
t = n.addSecs(10 * 60 * 60 + 5)  # t == 00:00:05
t = n.addSecs(-15 * 60 * 60)     # t == 23:00:00
//! [5]


//! [6]
time = QTime.fromString("1mm12car00", "m'mm'hcarss")
# time is 12:01.00
//! [6]


//! [7]
time = QTime.fromString("00:710", "hh:ms") # invalid
//! [7]


//! [8]
time = QTime.fromString("1.30", "m.s")
# time is 00:01:30.000
//! [8]


//! [9]
QTime.isValid(21, 10, 30) # returns True
QTime.isValid(22, 5,  62) # returns False
//! [9]


//! [10]
t = QTime()
t.start()
some_lengthy_task()
print ("Time elapsed: %d ms" % t.elapsed())
//! [10]


//! [11]
now = QDateTime.currentDateTime()
xmas(QDate(now.date().year(), 12, 25), QTime(0, 0))
print("There are %d seconds to Christmas" % now.secsTo(xmas))
//! [11]


//! [12]
time1 = QTime.fromString("131", "HHh")
# time1 is 13:00:00
time1 = QTime.fromString("1apA", "1amAM")
# time1 is 01:00:00

dateTime2 = QDateTime.fromString("M1d1y9800:01:02",
                                            "'M'M'd'd'y'yyhh:mm:ss")
# dateTime is 1 January 1998 00:01:02
//! [12]


//! [13]
dateTime = QDateTime.fromString("130", "Mm") # invalid
//! [13]


//! [14]
dateTime = QDateTime.fromString("1.30.1", "M.d.s")
# dateTime is January 30 in 1900 at 00:00:01.
//! [14]

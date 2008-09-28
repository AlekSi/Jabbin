/* vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
 * Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef SDP_H
#define SDP_H

#include <qstring.h>
#include <qmap.h>
#include <Q3ValueList>

// simple sdp,
// only one m=audio field assumed

class SDP
{
public:
    SDP();
    SDP(QString message);

    QString toString();
    void parse(QString);

    //SDP data
    QString sessionName; // s field
    QString owner; // o field
    QString host; // from c field
    int port;   // from the m=audio field
    Q3ValueList<int> formats; // from the m=audio field
    QMap<int,QString> rtmaps; // dict of values a=rtmap:<format>
    QMap<int,QString> fmtps; // dict of values a=fmtp:<format>
    //QStringList aFields; // a fields, except a=rtmap and a=fmtp
};

#endif // SDP_H


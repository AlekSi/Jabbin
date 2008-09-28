/*
 * sslcertdlg.h
 * Copyright (C) 2003  Justin Karneges
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

#ifndef SSLCERTDLG_H
#define SSLCERTDLG_H

#include <QtCrypto>

#include "ui_sslcert.h"

class SSLCertDlg : public QDialog
{
	Q_OBJECT
public:
	SSLCertDlg(QWidget *parent=0);

	void setCert(const QCA::Certificate &, int result, QCA::Validity);
	static void showCert(const QCA::Certificate &, int result, QCA::Validity);

protected:
	static void setLabelStatus(QLabel& l, bool ok);
	static QString makePropEntry(QCA::CertificateInfoType var, const QString &name, const QCA::CertificateInfo &list);
	QString makePropTable(const QString &heading, const QCA::CertificateInfo &props);

private:
	Ui::SSLCert ui_;
};

#endif

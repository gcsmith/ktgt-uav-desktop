// -----------------------------------------------------------------------------
// File:    ConnectionDialog.h
// Authors: Garrett Smith, Tyler Thierolf
// Created: 10-04-2010
//
// Definitions for connection dialog window.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_CONNECTIONDIALOG__H_
#define _HELIVIEW_CONNECTIONDIALOG__H_

#include <QWidget>
#include "ui_ConnectionDialog.h"
#include "DeviceController.h"

class ConnectionDialog: public QDialog, protected Ui::ConnectionDialog
{
    Q_OBJECT

public:
    ConnectionDialog(QWidget *parent);
    virtual ~ConnectionDialog();

signals:
    void requestConnection(const QString &source, const QString &device);

public slots:
    void onCancelClicked();
    void onConnectClicked();
    void onComboChanged(int index);
};

#endif // _HELIVIEW_CONNECTIONDIALOG__H_


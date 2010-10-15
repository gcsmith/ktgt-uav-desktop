// -----------------------------------------------------------------------------
// File:    ConnectionDialog.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_CONNECTIONDIALOG__H_
#define _HELIVIEW_CONNECTIONDIALOG__H_

#include <QWidget>
#include "ui_ConnectionDialog.h"

class ConnectionDialog : public QDialog, public Ui::ConnectionDialog
{
    Q_OBJECT

public:
    ConnectionDialog(QWidget *parent);

    virtual ~ConnectionDialog();

public slots:
    void s_cancelButton();
    void s_connectButton();
    void s_cbChange(int index);
};

#endif // _HELIVIEW_CONNECTIONDIALOG__H_


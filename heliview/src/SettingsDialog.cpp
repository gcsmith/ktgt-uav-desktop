#include "SettingsDialog.h"
#include <QColorDialog>
#include "DeviceController.h"

// -----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *pp, int r, int g, int b, int ht, int st)
: QDialog(pp), m_r(r), m_g(g), m_b(b), m_ht(ht), m_st(st)
{
    setupUi(this);

    // connect primary button events
    connect(btnCancel, SIGNAL(released()), this, SLOT(onCancelClicked()));
    connect(btnOK, SIGNAL(released()), this, SLOT(onOkClicked()));
    connect(btnApply, SIGNAL(released()), this, SLOT(onApplyClicked()));
    connect(btnNewColor, SIGNAL(released()), this, SLOT(onNewColorClicked()));

    sbR->setValue(m_r);
    sbG->setValue(m_g);
    sbB->setValue(m_b);
    sbHt->setValue(m_ht);
    sbSt->setValue(m_st);

    //TODO: Set LogFile Value
    //TODO: Set RGB Values
}

// -----------------------------------------------------------------------------
SettingsDialog::~SettingsDialog()
{
}

// -----------------------------------------------------------------------------
void SettingsDialog::onCancelClicked()
{
    reject();
}

// -----------------------------------------------------------------------------
void SettingsDialog::onOkClicked()
{
    onApplyClicked();
    accept();
}

// -----------------------------------------------------------------------------
void SettingsDialog::onApplyClicked()
{
    m_r = sbR->value();
    m_g = sbG->value();
    m_b = sbB->value();
    m_ht = sbHt->value();
    m_st = sbSt->value();

    emit updateTracking(m_r, m_g, m_b, m_ht, m_st);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onNewColorClicked()
{
    QColor initial(sbR->value(), sbG->value(), sbB->value());
    QColor color = QColorDialog::getColor(initial, this);
    
    if (color.isValid())
    {
        sbR->setValue(color.red());
        sbG->setValue(color.green());
        sbB->setValue(color.blue());
    }
}


#ifndef FILTERMENU_H
#define FILTERMENU_H

#include <QWidget>
#include "ui_filter-menu.h"

class FilterMenu : public QWidget,
                   public Ui::FilterMenu
{
    Q_OBJECT
public:
    FilterMenu(QWidget *parent = 0);
    QStringList filterList() const { return filter_list_; }
    void clearCheckBox();
signals:
    void filterChanged();
private slots:
    void onTextFile(bool checked);
    void onDocument(bool checked);
    void onImage(bool checked);
    void onVideo(bool checked);
    void onAudio(bool checked);
    void onMarkdown(bool checked);

private:
    void boxChanged(bool checked, const QString& text);
    Q_DISABLE_COPY(FilterMenu)
    QStringList filter_list_;
};

#endif // FILTERMENU_H
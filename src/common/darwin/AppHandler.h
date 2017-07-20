#ifndef APP_HANDLER_H
#define APP_HANDLER_H

class QWidget;
class QString;
class QStringList;
class QEvent;

#include <QObject>

namespace Darwin
{

void SetApplicationHandler(QWidget* handler);
void setDockBadge(int badgeInt);
void requestAttention();
unsigned long long getVolumeFreeSpace(QString path);
QString getOsVersion();
void updateDockMenu(QStringList userCWTitleList);

void setAsDefaultForExtension(QString extension);
void setAsDefaultForURI(QString URI);
bool isDefaultForExtension(QString extension);
bool isDefaultForURI(QString URI);
void setOverallProgress(int progress);
void showNotification(QString title, QString message);

class OpenEventFilter : public QObject
{
	Q_OBJECT

public:
	OpenEventFilter()
	{
	}

	~OpenEventFilter()
	{
	}

	bool eventFilter(QObject* obj, QEvent* event);
};

}  // Darwn

#endif // APP_HANDLER_H

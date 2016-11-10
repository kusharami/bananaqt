#pragma once

#include <QMainWindow>

namespace Banana
{
	struct MainWindowPrivate;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		explicit MainWindow(QWidget *parent = nullptr);
		virtual ~MainWindow();

		virtual void openFileOutside(const QString& filePath);
		virtual void customOutsideCommand(const QString &command, const QString &params);

	protected:
		void registerFileType(const QString& documentId,
							  const QString& fileTypeName,
							  const QString& fileExtension,
							  qint32         appIconIndex = 0);
		void registerCommand(const QString& command,
							 const QString& documentId,
							 const QString cmdLineArg = QString::null,
							 const QString ddeCommand = QString::null);
		void enableOpenOutside();
		virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;				

		friend struct MainWindowPrivate;
	private:
		MainWindowPrivate *p;
	};

}

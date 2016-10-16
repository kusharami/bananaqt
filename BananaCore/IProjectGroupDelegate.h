#pragma once

class QString;


namespace Core
{
	class ProjectDirectoryModel;

	enum class Answer
	{
		Unknown,
		No,
		NoToAll,
		Yes,
		YesToAll,
		Abort
	};

	struct IProjectGroupDelegate
	{
		virtual ~IProjectGroupDelegate() {}

		virtual Core::ProjectDirectoryModel *getProjectTreeModel() const = 0;

		virtual Answer shouldReplaceFile(const QString &filepath, Answer *remember_answer) = 0;
		virtual void errorMessage(const QString &message) = 0;
	};
}

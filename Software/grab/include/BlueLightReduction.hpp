#pragma once
#include <QList>
#include <QRgb>

namespace BlueLightReduction
{
	class Client
	{
		Q_DISABLE_COPY(Client)
	public:
		static bool isSupported() { Q_ASSERT_X(false, "BlueLightReduction::isSupported()", "not implemented"); return false; }
		virtual void apply(QList<QRgb>& colors, const double gamma = 1.2) = 0;
		virtual ~Client() = default;
	};

	Client* create();
};


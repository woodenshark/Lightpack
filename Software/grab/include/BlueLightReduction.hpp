#pragma once
#include <QList>
#include <QRgb>

namespace BlueLightReduction
{
	class Client
	{
	public:
		virtual void apply(QList<QRgb>& colors, const double gamma = 1.2) = 0;
	};

	Client* create();
};


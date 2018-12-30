#pragma once
#include <QList>
#include <QRgb>
#include <cassert>

namespace BlueLightReduction
{
	class Client
	{
	public:
		static bool isSupported() { assert(("BlueLightReduction::isSupported() is not implemented", false)); return false; }
		virtual void apply(QList<QRgb>& colors, const double gamma = 1.2) = 0;
		virtual ~Client() = default;
	};

	Client* create();
};


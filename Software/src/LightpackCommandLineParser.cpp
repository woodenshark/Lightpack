#include "LightpackCommandLineParser.hpp"

LightpackCommandLineParser::LightpackCommandLineParser()
    : m_noGUIOption("nogui", "Run application without a GUI.")
    , m_wizardOption("wizard", "Run settings wizard dialog.")
    , m_backlightOffOption("off", "Turn backlight off.")
    , m_backlightOnOption("on", "Turn backlight off.")
    , m_debugLevelOption("debug", "Set application debug level to [high|mid|low|zero].", "debug")
    , m_debugLevelHighOption("debug-high", "Set application debug level to high.")
    , m_debugLevelMidOption("debug-mid", "Set application debug level to mid.")
    , m_debugLevelLowOption("debug-low", "Set application debug level to low.")
    , m_debugLevelZeroOption("debug-zero", "Set application debug level to zero.")
    , m_versionOption(m_parser.addVersionOption())
    , m_helpOption(m_parser.addHelpOption())
{
    m_parser.addOption(m_noGUIOption);
    m_parser.addOption(m_wizardOption);
    m_parser.addOption(m_backlightOffOption);
    m_parser.addOption(m_backlightOnOption);
    m_parser.addOption(m_debugLevelOption);
    m_parser.addOption(m_debugLevelHighOption);
    m_parser.addOption(m_debugLevelMidOption);
    m_parser.addOption(m_debugLevelLowOption);
    m_parser.addOption(m_debugLevelZeroOption);
}

bool LightpackCommandLineParser::isSetNoGUI() const
{
    return m_parser.isSet(m_noGUIOption);
}

bool LightpackCommandLineParser::isSetWizard() const
{
    return m_parser.isSet(m_wizardOption);
}

bool LightpackCommandLineParser::isSetBacklightOff() const
{
    return m_parser.isSet(m_backlightOffOption);
}

bool LightpackCommandLineParser::isSetBacklightOn() const
{
    return m_parser.isSet(m_backlightOnOption);
}

bool LightpackCommandLineParser::isSetVersion() const
{
    return m_parser.isSet(m_versionOption);
}

bool LightpackCommandLineParser::isSetHelp() const
{
    return m_parser.isSet(m_helpOption);
}

bool LightpackCommandLineParser::isSetDebuglevel() const
{
    return m_parser.isSet(m_debugLevelOption) ||
        m_parser.isSet(m_debugLevelHighOption) ||
        m_parser.isSet(m_debugLevelMidOption) ||
        m_parser.isSet(m_debugLevelLowOption) ||
        m_parser.isSet(m_debugLevelZeroOption);
}

Debug::DebugLevels LightpackCommandLineParser::debugLevel() const
{
    Q_ASSERT(isSetDebuglevel());
    return m_debugLevel;
}

// static
Debug::DebugLevels LightpackCommandLineParser::parseDebugLevel(const QString& levelValue)
{
    if (levelValue == "high")
        return Debug::HighLevel;
    else if (levelValue == "mid")
        return Debug::MidLevel;
    else if (levelValue == "low")
        return Debug::LowLevel;
    else if (levelValue == "zero")
        return Debug::ZeroLevel;

    Q_ASSERT(false);
    return Debug::ZeroLevel;
}

bool LightpackCommandLineParser::parse(QString *errorMessage, const QStringList &arguments)
{
    if (!m_parser.parse(arguments))
    {
        if (errorMessage)
            *errorMessage = m_parser.errorText();
        return false;
    }

    if (m_parser.isSet(m_debugLevelOption))
    {
        const QString levelValue(m_parser.value(m_debugLevelOption));
        m_debugLevel = parseDebugLevel(levelValue);
    }
    else if (m_parser.isSet(m_debugLevelHighOption))
        m_debugLevel = Debug::HighLevel;
    else if (m_parser.isSet(m_debugLevelMidOption))
        m_debugLevel = Debug::MidLevel;
    else if (m_parser.isSet(m_debugLevelLowOption))
        m_debugLevel = Debug::LowLevel;
    else if (m_parser.isSet(m_debugLevelZeroOption))
        m_debugLevel = Debug::ZeroLevel;

    if (isSetBacklightOff() && isSetBacklightOn())
    {
        if (errorMessage)
            *errorMessage = "Bad options specified!";
        return false;
    }

    return true;
}

#include "LightpackCommandLineParser.hpp"

LightpackCommandLineParser::LightpackCommandLineParser()
    : m_noGUIOption("nogui", "no GUI (console mode)")
    , m_wizardOption("wizard", "run settings wizard first")
    , m_backlightOffOption("off", "send 'off leds' command to the device or running instance")
    , m_backlightOnOption("on", "send 'on leds' command to running instance, if any")
    , m_debugLevelOption("debug", "verbosity level of debug output (high, mid, low, zero)", "debug")
    , m_debugLevelHighOption("debug-high", "sets application debug level to high.")
    , m_debugLevelMidOption("debug-mid", "sets application debug level to mid.")
    , m_debugLevelLowOption("debug-low", "sets application debug level to low.")
    , m_debugLevelZeroOption("debug-zero", "sets application debug level to zero.")
    , m_versionOption(m_parser.addVersionOption())
    , m_helpOption(m_parser.addHelpOption())
    , m_optionSetProfile("set-profile", "switch to another profile in already running instance", "profile")
{
    m_parser.setApplicationDescription("Prismatik of Lightpack");
    m_parser.addOption(m_noGUIOption);
    m_parser.addOption(m_wizardOption);
    m_parser.addOption(m_backlightOffOption);
    m_parser.addOption(m_backlightOnOption);
    m_parser.addOption(m_debugLevelOption);
    m_parser.addOption(m_debugLevelHighOption);
    m_parser.addOption(m_debugLevelMidOption);
    m_parser.addOption(m_debugLevelLowOption);
    m_parser.addOption(m_debugLevelZeroOption);
    m_parser.addOption(m_optionSetProfile);
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

bool LightpackCommandLineParser::isSetProfile() const
{
    return m_parser.isSet(m_optionSetProfile);
}

Debug::DebugLevels LightpackCommandLineParser::debugLevel() const
{
    Q_ASSERT(isSetDebuglevel());
    return m_debugLevel;
}

QString LightpackCommandLineParser::profileName() const {
    Q_ASSERT(isSetProfile());
    return m_profileName;
}

QString LightpackCommandLineParser::helpText() const {
    return m_parser.helpText();
}

QString LightpackCommandLineParser::errorText() const {
    if (isSetBacklightOff() && isSetBacklightOn())
        return "Bad options specified!";
    return m_parser.errorText();
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

bool LightpackCommandLineParser::parse(const QStringList &arguments)
{
    if (!m_parser.parse(arguments))
        return false;

    if (m_parser.isSet(m_optionSetProfile))
        m_profileName = m_parser.value(m_optionSetProfile);

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
        return false;

    return true;
}

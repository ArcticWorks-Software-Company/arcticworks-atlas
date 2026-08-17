#include "qt/qt_common/arcticworks_style.hpp"

#include <QtWidgets/QApplication>
#include <QtGui/QPalette>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleFactory>

namespace arcticworks_style
{
namespace
{
Theme MakeDarkTheme()
{
  // Official @arcticworks/design dark tokens.
  Theme t;
  t.surface0 = QColor("#0B0B0D");            // palette.neutral.975
  t.surface1 = QColor("#111114");            // palette.neutral.950
  t.surface2 = QColor("#17171B");            // palette.neutral.925
  t.surface3 = QColor("#1D1D22");            // palette.neutral.900
  t.surfaceHover = QColor(255, 255, 255, 10);  // rgba(255,255,255,0.04)
  t.textPrimary = QColor("#F2F2F4");         // palette.neutral.50
  t.textSecondary = QColor("#A8A8B0");       // palette.neutral.400
  t.textTertiary = QColor("#7A7A84");        // palette.neutral.550
  t.textDisabled = QColor("#4E4E56");        // palette.neutral.750
  t.textOnAccent = QColor("#FFFFFF");        // palette.neutral.0
  t.borderSubtle = QColor("#1D1D22");        // palette.neutral.900
  t.borderDefault = QColor("#2C2C33");       // palette.neutral.850
  t.borderStrong = QColor("#3A3A42");        // palette.neutral.800
  t.borderAccent = QColor("#3D8BFF");        // interactive.default
  t.interactiveDefault = QColor("#3D8BFF");  // palette.blue.500
  t.interactiveHover = QColor("#5C9CFF");    // palette.blue.400
  t.interactivePrimary = QColor("#2B6FD9");  // palette.blue.600
  t.interactiveActive = QColor("#245BC2");   // palette.blue.700
  t.interactiveSubtle = QColor(61, 139, 255, 36);  // rgba(61,139,255,0.14)
  t.statusSuccess = QColor("#3FB950");       // palette.green.400
  t.statusWarning = QColor("#E3B341");       // palette.yellow.400
  t.statusDanger = QColor("#F04A4A");        // palette.red.400
  t.scrim = QColor(0, 0, 0, 140);            // rgba(0,0,0,0.55)
  return t;
}

Theme MakeLightTheme()
{
  // Light theme derived from the dark tokens per the @arcticworks/design
  // methodology: same palette, semantic roles remapped (see docs/ARCTICWORKS_THEME.md).
  Theme t;
  t.surface0 = QColor("#FFFFFF");            // palette.neutral.0
  t.surface1 = QColor("#F2F2F4");            // palette.neutral.50
  t.surface2 = QColor("#FFFFFF");            // palette.neutral.0
  t.surface3 = QColor("#E8E8EB");            // palette.neutral.100
  t.surfaceHover = QColor(0, 0, 0, 10);      // rgba(0,0,0,0.04)
  t.textPrimary = QColor("#17171B");         // palette.neutral.925
  t.textSecondary = QColor("#6F6F78");       // palette.neutral.600
  t.textTertiary = QColor("#7A7A84");        // palette.neutral.550
  t.textDisabled = QColor("#A8A8B0");        // palette.neutral.400
  t.textOnAccent = QColor("#FFFFFF");
  t.borderSubtle = QColor("#E8E8EB");        // palette.neutral.100
  t.borderDefault = QColor("#D9D9DD");       // palette.neutral.200
  t.borderStrong = QColor("#C4C4CA");        // palette.neutral.300
  t.borderAccent = QColor("#3D8BFF");
  t.interactiveDefault = QColor("#3D8BFF");  // palette.blue.500
  t.interactiveHover = QColor("#2B6FD9");    // palette.blue.600
  t.interactivePrimary = QColor("#2B6FD9");  // palette.blue.600
  t.interactiveActive = QColor("#245BC2");   // palette.blue.700
  t.interactiveSubtle = QColor(61, 139, 255, 36);
  t.statusSuccess = QColor("#3FB950");
  t.statusWarning = QColor("#E3B341");
  t.statusDanger = QColor("#F04A4A");
  t.scrim = QColor(0, 0, 0, 140);
  return t;
}

void ApplyPalette(QApplication & app, Theme const & t)
{
  QPalette p;

  p.setColor(QPalette::Window, t.surface1);
  p.setColor(QPalette::WindowText, t.textPrimary);
  p.setColor(QPalette::Base, t.surface2);
  p.setColor(QPalette::AlternateBase, t.surface1);
  p.setColor(QPalette::Text, t.textPrimary);
  p.setColor(QPalette::Button, t.surface2);
  p.setColor(QPalette::ButtonText, t.textPrimary);
  p.setColor(QPalette::BrightText, t.statusDanger);
  p.setColor(QPalette::Link, t.interactivePrimary);
  p.setColor(QPalette::LinkVisited, t.interactiveActive);
  p.setColor(QPalette::Highlight, t.interactiveDefault);
  p.setColor(QPalette::HighlightedText, t.textOnAccent);
  p.setColor(QPalette::ToolTipBase, t.surface3);
  p.setColor(QPalette::ToolTipText, t.textPrimary);
  p.setColor(QPalette::PlaceholderText, t.textTertiary);

  p.setColor(QPalette::Disabled, QPalette::WindowText, t.textDisabled);
  p.setColor(QPalette::Disabled, QPalette::Text, t.textDisabled);
  p.setColor(QPalette::Disabled, QPalette::ButtonText, t.textDisabled);
  p.setColor(QPalette::Disabled, QPalette::Highlight, t.textDisabled);

  app.setPalette(p);

  QString const qss = QStringLiteral(
      "QToolTip { background-color: %1; color: %2; border: 1px solid %3; padding: 4px 8px; }"
      "QMenu { background-color: %4; color: %2; border: 1px solid %5; }"
      "QMenu::item { padding: 4px 16px; }"
      "QMenu::item:selected { background-color: %6; }"
      "QMenu::separator { background-color: %5; height: 1px; }")
                          .arg(t.surface3.name(), t.textPrimary.name(), t.borderDefault.name(),
                               t.surface2.name(), t.borderSubtle.name(),
                               t.interactiveSubtle.name(QColor::HexArgb));
  app.setStyleSheet(qss);
}
}  // namespace

Theme const & LightTheme()
{
  static Theme const theme = MakeLightTheme();
  return theme;
}

Theme const & DarkTheme()
{
  static Theme const theme = MakeDarkTheme();
  return theme;
}

Theme const & Get(bool dark)
{
  return dark ? DarkTheme() : LightTheme();
}

void Apply(QApplication & app, bool dark)
{
  app.setStyle(QStyleFactory::create("Fusion"));
  ApplyPalette(app, Get(dark));
}
}  // namespace arcticworks_style

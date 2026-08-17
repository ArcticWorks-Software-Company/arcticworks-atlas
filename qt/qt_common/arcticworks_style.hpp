#pragma once

#include <QtGui/QColor>

class QApplication;

namespace arcticworks_style
{
// ArcticWorks design tokens from @arcticworks/design (dark) and the derived
// light theme. See docs/ARCTICWORKS_THEME.md for the full mapping.
struct Theme
{
  QColor surface0;
  QColor surface1;
  QColor surface2;
  QColor surface3;
  QColor surfaceHover;
  QColor textPrimary;
  QColor textSecondary;
  QColor textTertiary;
  QColor textDisabled;
  QColor textOnAccent;
  QColor borderSubtle;
  QColor borderDefault;
  QColor borderStrong;
  QColor borderAccent;
  QColor interactiveDefault;
  QColor interactiveHover;
  QColor interactivePrimary;
  QColor interactiveActive;
  QColor interactiveSubtle;
  QColor statusSuccess;
  QColor statusWarning;
  QColor statusDanger;
  QColor scrim;
};

Theme const & LightTheme();
Theme const & DarkTheme();
Theme const & Get(bool dark);

// Applies the ArcticWorks theme (Fusion style + palette + minimal QSS) to the application.
void Apply(QApplication & app, bool dark);
}  // namespace arcticworks_style

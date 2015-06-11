#include "staticvalueeditor.h"
#include "parameterwidgets.h"
#include <QtWidgets/QLabel>
#include "../watchers/watcherwidget.h"


StaticFloatEditor::StaticFloatEditor(FloatNode* node, WatcherWidget* panel)
    : PropertyEditor(node, panel) {
  WatcherWidget* widget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
  new FloatWatcher(static_cast<FloatNode*>(GetNode()), widget);
  mLayout->addWidget(widget);
}




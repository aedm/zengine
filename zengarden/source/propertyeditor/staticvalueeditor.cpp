#include "staticvalueeditor.h"
#include "parameterwidgets.h"
#include <QtWidgets/QLabel>


StaticFloatEditor::StaticFloatEditor(FloatNode* node, QWidget* PropertyPanel)
	: PropertyEditor(node, PropertyPanel)
{
	FloatSliderWidget* fsw = new FloatSliderWidget(PropertyPanel, node);
	Layout->addWidget(fsw);
}


#include "staticvalueeditor.h"
#include "parameterwidgets.h"
#include <QtWidgets/QLabel>


StaticFloatEditor::StaticFloatEditor(Node* Nd, QWidget* PropertyPanel)
	: PropertyEditor(Nd, PropertyPanel)
{
	FloatSliderWidget* fsw = new FloatSliderWidget(PropertyPanel);
	Layout->addWidget(fsw);
}


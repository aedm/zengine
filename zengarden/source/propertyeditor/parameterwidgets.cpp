//#include "../Kurbli/Component.h"
#include "parameterwidgets.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
//#include <QtGui/qspinbox.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qbuttongroup.h>
#include <QtGui/qpainter.h>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QTime>


Slider::Slider(QWidget* Parent)
	: QWidget(Parent)
	, Value(0.3)
	, Minimum(0)
	, Maximum(1)
{
}

void Slider::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	QSize s = size();
	p.setPen(palette().dark().color().darker());
	p.setBrush(palette().alternateBase().color());
	p.drawRect(0, 0, s.width() - 1, s.height() - 1);

	if (Maximum > Minimum)
	{
		p.setPen(Qt::NoPen);
		//p.setBrush(QColor(150, 50, 50));
		p.setBrush(palette().highlight().color());
		int widthPx = s.width() - 2;
		int full = float(widthPx) * Value / (Maximum - Minimum);
		if (full > widthPx) full = widthPx;
		else if (full < 0) full = 0;
		p.drawRect(1, 1, full, s.height() - 2);
	}

	if (!Text.isEmpty())
	{
		//QFont font = p.font();
		//font.setPointSize(font.pointSize() - 1);
		//p.setFont(font);
		p.setPen(palette().windowText().color());
		p.drawText(5, 0, width(), height(), Qt::AlignVCenter, Text);
	}
}

void Slider::mousePressEvent(QMouseEvent * event)
{
	HandleMouse(event);
}

void Slider::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		HandleMouse(event);
	}
}

void Slider::HandleMouse(QMouseEvent * event)
{
	if (Maximum > Minimum)
	{
		float newValue = float(event->x() + 1) * (Maximum - Minimum) / float(width());
		if (Value != newValue) {
			Value = newValue;
			EventValueChange(Value);
			this->repaint();
		}
	}
}

float Slider::GetValue()
{
	return Value;
}

void Slider::SetValue(float Value)
{
	this->Value = Value;
	repaint();
}


FloatSliderWidget::FloatSliderWidget(QWidget* Parent, FloatNode* node)
	: QWidget(Parent)
	//, Watcher(MakeDelegate(this, &FloatSliderWidget::OnOperatorValueChanges))
{
	//this->Op = Op;
	Op = node;

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setSpacing(4);
	layout->setContentsMargins(0, 0, 0, 0);

	Slide = new Slider(this);
	Slide->setMinimumHeight(20);
	Slide->setMinimumWidth(70);
	Slide->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	//float val = Op->GetValue();
	float val = node->Get();
	//if (Desc != NULL)
	//{
	//	Slide->Minimum = Desc->UIMinimum;
	//	Slide->Maximum = Desc->UIMaximum;
	//	Slide->Text = QString::fromStdString(*Desc->UIName);
	//}
	//else 
	{
		Slide->Minimum = 0;
		Slide->Maximum = 1;
		Slide->Text = QString("[unknown]");
	}
	layout->addWidget(Slide);
	Slide->SetValue(val);

	TextBox = new QLineEdit(this);
	SetTextBoxValue(val);
	TextBox->setFixedWidth(40);

	layout->addWidget(TextBox);

	Slide->EventValueChange += Delegate(this, &FloatSliderWidget::SliderValueChanged);
	//connect(Slider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged()));
	connect(TextBox, SIGNAL(editingFinished()), this, SLOT(SpinBoxValueChanged()));

	//Watcher.TheSlot.Connect(Op);

	SilentChange = false;
}

void FloatSliderWidget::SpinBoxValueChanged()
{
	if (!SilentChange)
	{
		SilentChange = true;
		float value = TextBox->text().toFloat();
		Slide->SetValue(value);
		Op->Set(value);
		SilentChange = false;
	}
}

void FloatSliderWidget::OnOperatorValueChanges()
{
	if (!SilentChange)
	{
		SilentChange = true;
		//float value = Op->GetValue();
		//Slider->setValue(int(value * Magnification));
		//SetTextBoxValue(value);
		SilentChange = false;
	}
}

void FloatSliderWidget::SetTextBoxValue(float value)
{
	TextBox->setText(QString::number(value, 'g', 3));
}

void FloatSliderWidget::SliderValueChanged(float Vlaue)
{
	float value = Slide->GetValue();
	SetTextBoxValue(value);
	Op->Set(value);
}

//IntegerSliderWidget::IntegerSliderWidget( QWidget* Parent, Parameter* Param )
//{
//	IntParameter = Param;
//
//	QHBoxLayout* layout = new QHBoxLayout(this);
//	layout->setSpacing(4);
//	layout->setContentsMargins(0, 0, 0, 0);
//
//	QLabel* text = new QLabel(this);
//	text->setText(QString::fromUtf16((ushort*)Param->Name.c_str()));
//	text->setMinimumHeight(20);
//	text->setMinimumWidth(70);
//	layout->addWidget(text);
//
//	int min = IntParameter->UIMin;
//	int max = IntParameter->UIMax;
//	int def = IntParameter->IntegerValue;
//
//	Slider = new QSlider(Qt::Horizontal, this);
//	Slider->setMinimum(int(min));
//	Slider->setMaximum(int(max));
//	Slider->setValue(int(def));
//	layout->addWidget(Slider);
//
//	SpinBox = new QSpinBox(this);
//	SpinBox->setValue(def);
//	layout->addWidget(SpinBox);
//
//	connect(Slider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged()));
//	connect(SpinBox, SIGNAL(valueChanged(int)), this, SLOT(SpinBoxValueChanged()));
//
//	SilentChange = false;
//}
//
//void IntegerSliderWidget::SliderValueChanged()
//{
//	if (!SilentChange)
//	{
//		SilentChange = true;
//		int value = Slider->value();
//		SpinBox->setValue(value);
//		IntParameter->SetIntValue(value);
//		SilentChange = false;
//	}
//}
//
//void IntegerSliderWidget::SpinBoxValueChanged()
//{
//	if (!SilentChange)
//	{
//		SilentChange = true;
//		int value = SpinBox->value();
//		Slider->setValue(value);
//		IntParameter->SetIntValue(value);
//		SilentChange = false;
//	}
//}
//
//EnumSelectorWidget::EnumSelectorWidget( QWidget* Parent, Parameter* Param )
//{
//	const int ColumnCount = 3;
//
//	EnumParameter = Param;
//
//	QHBoxLayout* layout = new QHBoxLayout(this);
//	layout->setSpacing(4);
//	layout->setContentsMargins(4, 0, 4, 0);
//
//	QGroupBox* groupBox = new QGroupBox(this);
//	groupBox->setTitle(QString::fromUtf16((ushort*)Param->Name.c_str()));
//	layout->addWidget(groupBox);
//
//	ButtonGroup = new QButtonGroup(groupBox);
//
//	QGridLayout* glayout = new QGridLayout(groupBox);
//	glayout->setSpacing(4);
//	glayout->setContentsMargins(4, 4, 4, 4);
//
//	for (UINT i=0; i<Param->EnumOptions.size(); i++)
//	{
//		QRadioButton* radio = new QRadioButton(groupBox);
//		radio->setText(QString::fromUtf16((ushort*)Param->EnumOptions[i].c_str()));
//		if (i == Param->IntegerValue) radio->setChecked(true);
//		glayout->addWidget(radio, i/ColumnCount, i%ColumnCount);
//		ButtonGroup->addButton(radio, i);
//		connect(radio, SIGNAL(toggled(bool)), this, SLOT(EnumChanged(bool)));
//	}
//}
//
//void EnumSelectorWidget::EnumChanged(bool On)
//{
//	if (!On) return;
//	EnumParameter->SetIntValue(ButtonGroup->checkedId());
//}


#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
//#include <QtWidgets/qbuttongroup.h>

class QSpinBox;
//class QDoubleSpinBox;
class QLineEdit;
class QSlider;

static const float	Magnification = 200.0f;

class Slider : public QWidget
{
	Q_OBJECT

public:
	Slider(QWidget* Parent);
	virtual ~Slider() {}

	QString				Text;
	float				Minimum;
	float				Maximum;
	Event<float>		EventValueChange;

	void				SetValue(float Value);
	float				GetValue();

private:
	void				paintEvent(QPaintEvent *e);
	virtual void		mousePressEvent(QMouseEvent * event) override;
	virtual void		mouseMoveEvent(QMouseEvent * event) override;

	void				HandleMouse(QMouseEvent * event);

	float				Value;
};

//template<OpTypeEnum T>
//class WatcherOperator : public Operator
//{
//public:
//	WatcherOperator(FastDelegate0<void>& DirtyCallback);
//	virtual void				OnSlotValueChanged(Slot* DirtySlot) override;
//	FastDelegate0<void>			DirtyCallback;
//	TypedSlot<T>				TheSlot;
//};

//template<OpTypeEnum T>
//void WatcherOperator<T>::OnSlotValueChanged(Slot* DirtySlot)
//{
//	DirtyCallback();
//}
//
//template<OpTypeEnum T>
//WatcherOperator<T>::WatcherOperator(FastDelegate0<void>& _DirtyCallback)
//	: Operator("watcher")
//	, TheSlot(this, SharedString(new string("client")))
//	, DirtyCallback(_DirtyCallback)
//{}

class FloatSliderWidget : public QWidget
{
	Q_OBJECT

public:
	FloatSliderWidget(QWidget* Parent, FloatNode* node);
	virtual ~FloatSliderWidget() {}

private:
	FloatNode*					Op;

	//QDoubleSpinBox*			SpinBox;
	QLineEdit*					TextBox;
	//QSlider*					Slider;
	//WatcherOperator<OP_FLOAT>	Watcher;
	Slider*						Slide;

	bool						SilentChange;			/// Do not attempt to modify Parameter in silent change mode.

	void						OnOperatorValueChanges();
	void						SetTextBoxValue(float value);

	void						SliderValueChanged(float Vlaue);

public slots:
	//void						SliderValueChanged();
	void						SpinBoxValueChanged();

};


//class IntegerSliderWidget : public QWidget
//{
//	Q_OBJECT
//
//public:
//	IntegerSliderWidget(QWidget* Parent, Parameter* Param);
//	virtual ~IntegerSliderWidget() {}
//
//private:
//	Int*			IntParameter;
//
//	QSpinBox*			SpinBox;
//	QSlider*			Slider;
//
//	bool				SilentChange;				/// Do not attempt to modify Parameter in silent change mode.
//
//public slots:
//	void				SliderValueChanged();
//	void				SpinBoxValueChanged();
//
//};


//class EnumSelectorWidget : public QWidget
//{
//	Q_OBJECT
//
//public:
//	EnumSelectorWidget(QWidget* Parent, Parameter* Param);
//	virtual ~EnumSelectorWidget() {}
//
//private:
//	Parameter*			EnumParameter;
//
//	QButtonGroup*		ButtonGroup;
//
//public slots:
//	void				EnumChanged(bool On);
//
//};
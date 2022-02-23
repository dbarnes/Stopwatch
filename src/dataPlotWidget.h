/*
 * @author fiachra & tom
 */

#ifndef DATA_PLOT_WIDGET_H
#define DATA_PLOT_WIDGET_H

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

class QPaintEvent;

class DataPlotWidget : public QWidget {
  Q_OBJECT;

 public:
  DataPlotWidget(QWidget* parent = 0);
  virtual ~DataPlotWidget();

  void updatePlot(float* value, bool* enabled);
  void resetPlot();
  void setDataLength(int length);

  int PLOT_WIDTH = 640;
  int PLOT_HEIGHT = 240;

  static const int DEFAULT_DATA_LENGTH = 100;
  static const int NUM_PLOTS = 10;
  static const int PEN_WIDTH = 1;
  static const int PEN_HIGHLIGHT_WIDTH = 2;

 private:
  QPainter painter;
  std::array<QColor, NUM_PLOTS> colours;

  void paintEvent(QPaintEvent* event);
  int getPlotY(float val, float dataMin, float dataMax);
  void drawDataPlot();
  void resizeEvent(QResizeEvent* event) override;

  int lastLimit;
  int dataLength;
  float** dataArray;
  int currentCount;
  int currentIndex;
};

#endif

#include "dataPlotWidget.h"
#include <QApplication>
#include <QPainter>
#include <QResizeEvent>
#include <cmath>

DataPlotWidget::DataPlotWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(PLOT_WIDTH, PLOT_HEIGHT);

  currentCount = 0;
  currentIndex = 0;
  dataLength = DEFAULT_DATA_LENGTH;

  dataArray = new float*[NUM_PLOTS];

  static constexpr float golden_ratio = 0.5 * (1 + std::sqrt(5));

  for (int i = 0; i < NUM_PLOTS; i++) {
    dataArray[i] = new float[dataLength];

    // colour distance by golden ratio
    colours[i] = QColor::fromHsvF(std::fmod(i * golden_ratio, 1), 1, 1);

    for (int j = 0; j < dataLength; j++) {
      dataArray[i][j] = 0.0f;
    }
  }
}

DataPlotWidget::~DataPlotWidget() {
  for (int i = 0; i < NUM_PLOTS; i++) {
    delete[] dataArray[i];
  }

  delete[] dataArray;
}

void DataPlotWidget::updatePlot(float* value, bool* enabled) {
  lastLimit = 0;

  while (enabled[++lastLimit] && lastLimit < DataPlotWidget::NUM_PLOTS)
    ;

  if (currentCount == currentIndex) {
    if (currentCount < dataLength) {
      for (int k = 0; k < lastLimit; k++) {
        dataArray[k][currentIndex] = value[k];
      }

      currentCount++;
      currentIndex++;
    } else {
      for (int i = 0; i < dataLength - 1; i++) {
        for (int k = 0; k < lastLimit; k++) {
          dataArray[k][i] = dataArray[k][i + 1];
        }
      }

      for (int k = 0; k < lastLimit; k++) {
        dataArray[k][dataLength - 1] = value[k];
      }
    }
  } else {
    currentIndex++;
  }

  update();
}

void DataPlotWidget::paintEvent(QPaintEvent* event) {
  if (event == 0)
    return;

  painter.begin(this);
  drawDataPlot();
  painter.end();
}

int DataPlotWidget::getPlotY(float val, float dataMin, float dataMax) {
  float dataRange;

  if (dataMax == dataMin) {
    dataMax += dataMax * 0.1;
    dataMin -= dataMin * 0.1;
  }

  dataRange = dataMax - dataMin;
  dataRange *= 1.1;

  int yVal = ((val - dataMin) * ((PLOT_HEIGHT - 13) / dataRange)) + (13);

  return yVal;
}

void DataPlotWidget::drawDataPlot() {
  painter.setBrush(QColor("black"));
  painter.drawRect(0, 0, PLOT_WIDTH, PLOT_HEIGHT);

  painter.setPen(QPen(QColor("green"), PEN_WIDTH));
  float dataMin = 0.0f;
  float dataMax = 0.0f;

  for (int k = 0; k < lastLimit; k++) {
    for (int i = 0; i < currentCount; i++) {
      if (i == 0 && k == 0) {
        dataMin = dataArray[k][i];
        dataMax = dataArray[k][i];
      } else {
        if (dataArray[k][i] > dataMax)
          dataMax = dataArray[k][i];

        if (dataArray[k][i] < dataMin)
          dataMin = dataArray[k][i];
      }
    }
  }

  painter.drawText(5, 24, "Max: " + QString::number(dataMax));
  painter.drawText(5, PLOT_HEIGHT - 2, "Min: " + QString::number(dataMin));

  QTransform trans(1, 0, 0, 0, -1, 0, 0, 0, 1);
  trans.translate(0, -PLOT_HEIGHT);
  painter.setTransform(trans);

  if (currentCount > 0) {
    for (int i = 1; i < currentCount; i++) {
      for (int k = 0; k < lastLimit; k++) {
        painter.setPen(QPen(colours[k], PEN_WIDTH));

        painter.drawLine(
            (int)(((float)PLOT_WIDTH / (float)dataLength) * (i - 1)),
            getPlotY(dataArray[k][i - 1], dataMin, dataMax),
            (int)(((float)PLOT_WIDTH / (float)dataLength) * (i)),
            getPlotY(dataArray[k][i], dataMin, dataMax));
      }
    }
  }
}

void DataPlotWidget::resetPlot() {
  currentCount = 0;
  currentIndex = 0;

  for (int i = 0; i < NUM_PLOTS; i++) {
    for (int j = 0; j < dataLength; j++) {
      dataArray[i][j] = 0.0f;
    }
  }
}

void DataPlotWidget::setDataLength(int length) {
  dataLength = length;

  for (int i = 0; i < NUM_PLOTS; i++) {
    delete[] dataArray[i];
  }

  delete[] dataArray;

  dataArray = new float*[NUM_PLOTS];

  for (int i = 0; i < NUM_PLOTS; i++) {
    dataArray[i] = new float[dataLength];
  }

  resetPlot();
}

void DataPlotWidget::resizeEvent(QResizeEvent* event) {
  PLOT_WIDTH = event->size().width();
  PLOT_HEIGHT = event->size().height();
}
